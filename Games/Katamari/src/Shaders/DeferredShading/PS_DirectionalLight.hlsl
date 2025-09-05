#include "ShaderStructs.hlsli"

struct LightPassInput
{
	float4 PositionH : SV_POSITION;
	float2 texCoord : TEXCOORD0;
};

ConstantBuffer<PerPassData> PassCB : register( b0 );

Texture2D GBuffer_AlbedoOpacity		: register(t0);
Texture2D GBuffer_NormalPower	: register(t1);
Texture2D GBuffer_ReflectanceSpec	: register(t2);
Texture2D GBuffer_EmissiveAmbient	: register(t3);
Texture2D SceneDepthTexture			: register(t4);

ConstantBuffer<CascadeData> CascadeDataCB : register( b1 );
ConstantBuffer<DirectionalLight> DirectionalLightCB : register( b2 );

Texture2D ShadowMap1 : register( t5 );
Texture2D ShadowMap2 : register( t6 );
Texture2D ShadowMap3 : register( t7 );
Texture2D ShadowMap4 : register( t8 );

SamplerState PointWrapSamp     : register( s0 );
SamplerState PointClampSamp    : register( s1 );
SamplerState LinWrapSamp       : register( s2 );
SamplerState LinClampSamp      : register( s3 );
SamplerState AniWrapSamp       : register( s4 );
SamplerState AniClampSamp      : register( s5 );

float3 ReconstructWorldPos(float2 texCoord, float depth)
{
	float x = texCoord.x * 2.0f - 1.0f;
	float y = (1.0f - texCoord.y) * 2.0f - 1.0f;
	float z = depth;

	float4 positionH = float4(x, y, z, 1.0f);

	positionH = mul(positionH, PassCB.InvProj);
	positionH /= positionH.w;
	positionH = mul(positionH, PassCB.InvView);

	return positionH.xyz;
}

float3 ReconstructViewSpacePosition(float2 screenUV, float rawDepth)
{
	float4 clipPos = float4(screenUV.x * 2.0f - 1.0f, screenUV.y * 2.0f - 1.0f, rawDepth, 1.0f);

	float4 viewPos = mul(PassCB.InvProj, clipPos);
	return viewPos.xyz / viewPos.w; // Perform perspective divide
}

float3 UnpackNormal(float3 n_packed)
{
	return n_packed * 2.0f - 1.0f;
}

struct GBufferPixelData
{
	float3 Albedo;
	float  Opacity;
	float3 NormalWS;      // World space normal (unpacked to -1..1)
	float  SpecularPower; // Unpacked
	float3 Reflectance;
	float3 Emissive;
	float  Ambient;  // Alpha component of ambient material property
	float3 PositionVS;    // Reconstructed view space position
	float3 PositionWS;    // Reconstructed world space position
	float  DistanceToCamera; // World-space distance from pixel to camera
};

GBufferPixelData FetchGBufferData(float2 screenUV)
{
	GBufferPixelData data;

	// RT0: Albedo (RGB) + Opacity (A)
	float4 albedoOpacity = GBuffer_AlbedoOpacity.Sample(LinClampSamp, screenUV);
	data.Albedo = albedoOpacity.rgb;
	data.Opacity = albedoOpacity.a;

	// RT1: Normal (RGB, remapped 0-1) + SpecularPower (A, remapped 0-1)
	float4 normal = GBuffer_NormalPower.Sample(LinClampSamp, screenUV);
	data.NormalWS = UnpackNormal(normal.rgb); // Unpack normal to [-1, 1]

	// RT2: Reflectance (RGB)
	float4 ReflectanceSpec = GBuffer_ReflectanceSpec.Sample(LinClampSamp, screenUV);
	data.Reflectance = ReflectanceSpec.rgb;
	data.SpecularPower = ReflectanceSpec.a; // Remap specular power back (assuming max 256)

	// RT3: Emissive (RGB) + AmbientAlpha (A)
	float4 emissiveAmbient = GBuffer_EmissiveAmbient.Sample(LinClampSamp, screenUV);
	data.Emissive = emissiveAmbient.rgb;
	data.Ambient = emissiveAmbient.a;

	float rawDepth = SceneDepthTexture.Sample(LinClampSamp, screenUV).r;

	data.PositionWS = ReconstructWorldPos(screenUV, rawDepth);//worldPosH.xyz / worldPosH.w;
	data.PositionVS =  mul(float4 (data.PositionWS, 1.0f), PassCB.View).xyz;

	data.DistanceToCamera = distance(PassCB.EyePos, data.PositionWS);

	return data;
}

float3 DoLightingCalculation(
    float3  lightStrength,
    float3  L,             // Normalized light direction (from surface to light)
    float3  N,             // Normalized surface normal
    float3  V,             // Normalized view direction (from surface to camera)
    float   specularPower, // From G-Buffer
    float3  fresnelR0,     // From G-Buffer (Reflectance)
    float3  diffuseAlbedo) // From G-Buffer (Albedo)
{
    float NdotL = saturate(dot(N, L));
    float3 diffuseColor = diffuseAlbedo * NdotL;

    float3 H = normalize(L + V); 
    float NdotH = saturate(dot(N, H));
	float m = specularPower * 256.0f;
    float roughnessFactor = (m + 8.0f) * pow(NdotH, m) / 8.0f; // Direct use of specularPower

    float cosIncidentAngle = saturate(dot(H, V)); // Using H.V for Fresnel in Blinn-Phong
    float3 reflectPercent = fresnelR0 + (1.0f - fresnelR0) * pow(1.0f - cosIncidentAngle, 5.0f);
    float3 specAlbedo = reflectPercent * roughnessFactor;

    specAlbedo = specAlbedo / (specAlbedo + 1.0f);

    return (diffuseColor + specAlbedo) * lightStrength;
}

float DoShadowCascade(float4 ShadowPos, Texture2D ShadowMapTex)
{
    float3 projCoords = ShadowPos.xyz / ShadowPos.w;
    projCoords.xy = 0.5 * projCoords.xy + 0.5;
    projCoords.y = 1.0f - projCoords.y; // Flip Y for DirectX textures

    // Add a small epsilon to avoid "shadow acne" due to floating point precision issues
    const float bias = 0.0003;

    // PCF (Percentage-Closer Filtering) for smoother shadows
    uint width, height, numMips;
    ShadowMapTex.GetDimensions(0, width, height, numMips);
    float texelSize = 1.0f / (float)width; // Assuming square shadow map

    float shadow = 0.0;
    // 3x3 PCF sampling
	[unroll]
    for (int x = -1; x <= 1; ++x)
    {
    	[unroll]
        for (int y = -1; y <= 1; ++y)
        {
            float depth = ShadowMapTex.Sample(AniClampSamp, projCoords.xy + float2(x, y) * texelSize).r;
            if (depth + bias < projCoords.z)
            {
                shadow += 0.0; // In shadow
            }
            else
            {
                shadow += 1.0; // Not in shadow
            }
        }
    }

    return shadow / 9.0f; // Average the samples
}

float DoShadow(float3 PositionWS, float distanceToCamera)
{
    // Determine which cascade to use based on distance from camera
    int chosenCascade = 3; // Default to the closest cascade

    if (distanceToCamera < CascadeDataCB.distances.x)
        chosenCascade = 0;
    else if (distanceToCamera < CascadeDataCB.distances.y)
        chosenCascade = 1;
    else if (distanceToCamera < CascadeDataCB.distances.z)
        chosenCascade = 2;
    else if (distanceToCamera < CascadeDataCB.distances.w)
        chosenCascade = 3;
    else
        return 1.0; // Beyond the farthest cascade, assume no shadow

    float4 pos4 = float4(PositionWS, 1.0);
    // Transform world position to light's view-projection space for the chosen cascade
    float4 lightViewProjPos = mul(pos4, CascadeDataCB.viewProjMats[chosenCascade]);

    // Sample the correct shadow map texture
    switch(chosenCascade)
    {
    case 0: return DoShadowCascade(lightViewProjPos, ShadowMap1);
    case 1: return DoShadowCascade(lightViewProjPos, ShadowMap2);
    case 2: return DoShadowCascade(lightViewProjPos, ShadowMap3);
    case 3: return DoShadowCascade(lightViewProjPos, ShadowMap4);
    default: return 1.0; // Should not happen
    }
}


float4 PS_DirectionalLight(float4 position : SV_POSITION, float2 screenUV : TEXCOORD0) : SV_Target0
{
	//return float4(1.0, 0.0, 0.0, 1.0);
	GBufferPixelData gBuffer = FetchGBufferData(screenUV);

	//return float4(gBuffer.PositionWS, 1.0);
	if (gBuffer.Opacity < 0.01f)
		discard;

	float3 LightDirection = normalize(-DirectionalLightCB.DirectionWS.xyz);
	
	float3 ViewDirection_WS = normalize(PassCB.EyePos - gBuffer.PositionWS);

	float3 Normal_WS = normalize(gBuffer.NormalWS.xyz);

	float3 lightStrength = DirectionalLightCB.Color.rgb;
	
	float shadowFactor = DoShadow(gBuffer.PositionWS, gBuffer.DistanceToCamera);
	lightStrength *= saturate(0.1 + shadowFactor);

	float3 finalLightColor = DoLightingCalculation(
		lightStrength,
		LightDirection,
		Normal_WS,
		ViewDirection_WS,
		gBuffer.SpecularPower,
		gBuffer.Reflectance,
		gBuffer.Albedo
	);

	finalLightColor += DirectionalLightCB.Color.rgb * DirectionalLightCB.Ambient * gBuffer.Albedo;

	return float4(finalLightColor, 1.0);
}
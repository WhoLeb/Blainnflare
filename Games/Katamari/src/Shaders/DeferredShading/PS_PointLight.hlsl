#include "ShaderStructs.hlsli"

ConstantBuffer<PerPassData> PassCB : register( b0 );
ConstantBuffer<PointLight> PointLightCB : register( b1, space1 );

Texture2D GBuffer_AlbedoOpacity		: register(t0);
Texture2D GBuffer_Normal			: register(t1);
Texture2D GBuffer_ReflectanceSpec	: register(t2);
Texture2D GBuffer_EmissiveAmbient	: register(t3);
Texture2D SceneDepthTexture			: register(t4);

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
	// Convert screen UV to clip space (-1 to 1 range)
	float4 clipPos = float4(screenUV * 2.0f - 1.0f, rawDepth, 1.0f);

	// Transform from clip space to view space
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
	float  AmbientAlpha;  // Alpha component of ambient material property
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
	float4 normalSpec = GBuffer_Normal.Sample(LinClampSamp, screenUV);
	data.NormalWS = UnpackNormal(normalSpec.rgb); // Unpack normal to [-1, 1]

	// RT2: Reflectance (RGB)
	float4 ReflectanceSpec = GBuffer_ReflectanceSpec.Sample(LinClampSamp, screenUV);
	data.Reflectance = ReflectanceSpec.xyz;
	data.SpecularPower = ReflectanceSpec.a;

	// RT3: Emissive (RGB) + AmbientAlpha (A)
	float4 emissiveAmbient = GBuffer_EmissiveAmbient.Sample(LinClampSamp, screenUV);
	data.Emissive = emissiveAmbient.rgb;
	data.AmbientAlpha = emissiveAmbient.a;

	// Retrieve raw depth from the bound depth buffer (not an SV_Target output)
	// This assumes the depth buffer is available as a shader resource view (SRV) for sampling.
	// If not, you might need to output linear depth to a separate RT in your GBufferPS.
	float rawDepth = SceneDepthTexture.Sample(LinClampSamp, screenUV).r;

	// Reconstruct view-space position
	data.PositionWS = ReconstructWorldPos(screenUV, rawDepth);
	data.PositionVS = ReconstructViewSpacePosition(screenUV, rawDepth);

	// If PositionVS.z is very close to NearZ or FarZ, or if it's the clear value,
	// it likely means no geometry was rendered there.
	// This can also be done with a 'discard' check later.
	// For now, return a sentinel if needed, or rely on discard.

	// Reconstruct world-space position from view-space position
	float4 worldPosH = mul(PassCB.InvView, float4(data.PositionVS, 1.0f));

	// Calculate distance to camera for shadow cascade selection
	data.DistanceToCamera = distance(PassCB.EyePos, data.PositionWS);

	return data;
}

float DoAttenuation( float c, float l, float q, float d )
{
	return 1.0f / ( c + l * d + q * d * d );
}

// Adapted from your original BlinnPhong function.
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
	float roughnessFactor = (m + 8.0f) * pow(NdotH, m) / 8.0f; // Fix: multiply, not divide
	float cosIncidentAngle = saturate(dot(H, V));
	float3 reflectPercent = fresnelR0 + (1.0f - fresnelR0) * pow(1.0f - cosIncidentAngle, 5.0f);
	float3 specAlbedo = reflectPercent * roughnessFactor;
	specAlbedo = specAlbedo / (specAlbedo + 1.0f);
	return (diffuseColor + specAlbedo) * lightStrength;
}

float4 PS_PointLight(float4 position : SV_Position, float2 screenUV : TEXCOORD0) : SV_Target
{
    // Fetch all necessary data for this pixel from the G-Buffer
    GBufferPixelData gbuffer = FetchGBufferData(screenUV);

    float3 lightVec_WS = PointLightCB.PositionWS.xyz - gbuffer.PositionWS;
    float  distance = length(lightVec_WS);

    if (distance > PointLightCB.Radius)
    {
        discard;
    }

    float3 L = normalize(lightVec_WS); // Normalized light direction (from fragment to light)

    float3 V_WS = normalize(PassCB.EyePos - gbuffer.PositionWS);

    float3 N_WS = normalize(gbuffer.NormalWS); 

    float attenuation = max(1.0f - distance / PointLightCB.Radius, 0);
    attenuation *= attenuation;

    //attenuation *= saturate(1.0 - pow(distance / PointLightCB.Radius, 2.0));

    float3 lightStrength = PointLightCB.Color.rgb * attenuation;

    float3 finalLightColor = DoLightingCalculation(
                                lightStrength,
                                L,
                                N_WS,
                                V_WS,
                                gbuffer.SpecularPower,
                                gbuffer.Reflectance,
                                gbuffer.Albedo);

    return float4(finalLightColor, 1.0);
}
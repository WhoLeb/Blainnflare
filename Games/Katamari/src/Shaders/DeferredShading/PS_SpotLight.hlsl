#include "ShaderStructs.hlsli"

ConstantBuffer<PerPassData> PassCB : register( b0 );
ConstantBuffer<SpotLight> SpotLightCB : register( b1, space1 );

Texture2D GBuffer_AlbedoOpacity		: register(t0);
Texture2D GBuffer_NormalSpecPower	: register(t1);
Texture2D GBuffer_Reflectance		: register(t2);
Texture2D GBuffer_EmissiveAmbient	: register(t3);
Texture2D SceneDepthTexture			: register(t4);

SamplerState PointWrapSamp     : register( s0 );
SamplerState PointClampSamp    : register( s1 );
SamplerState LinWrapSamp       : register( s2 );
SamplerState LinClampSamp      : register( s3 );
SamplerState AniWrapSamp       : register( s4 );
SamplerState AniClampSamp      : register( s5 );

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
	float4 albedoOpacity = GBuffer_AlbedoOpacity.Sample(AniClampSamp, screenUV);
	data.Albedo = albedoOpacity.rgb;
	data.Opacity = albedoOpacity.a;

	// RT1: Normal (RGB, remapped 0-1) + SpecularPower (A, remapped 0-1)
	float4 normalSpec = GBuffer_NormalSpecPower.Sample(AniClampSamp, screenUV);
	data.NormalWS = UnpackNormal(normalSpec.rgb); // Unpack normal to [-1, 1]
	data.SpecularPower = normalSpec.a * 256.0f; // Remap specular power back (assuming max 256)

	// RT2: Reflectance (RGB)
	data.Reflectance = GBuffer_Reflectance.Sample(AniClampSamp, screenUV).rgb;

	// RT3: Emissive (RGB) + AmbientAlpha (A)
	float4 emissiveAmbient = GBuffer_EmissiveAmbient.Sample(AniClampSamp, screenUV);
	data.Emissive = emissiveAmbient.rgb;
	data.AmbientAlpha = emissiveAmbient.a;

	// Retrieve raw depth from the bound depth buffer (not an SV_Target output)
	// This assumes the depth buffer is available as a shader resource view (SRV) for sampling.
	// If not, you might need to output linear depth to a separate RT in your GBufferPS.
	float rawDepth = SceneDepthTexture.Sample(AniClampSamp, screenUV).r;

	// Reconstruct view-space position
	data.PositionVS = ReconstructViewSpacePosition(screenUV, rawDepth);

	// If PositionVS.z is very close to NearZ or FarZ, or if it's the clear value,
	// it likely means no geometry was rendered there.
	// This can also be done with a 'discard' check later.
	// For now, return a sentinel if needed, or rely on discard.

	// Reconstruct world-space position from view-space position
	float4 worldPosH = mul(PassCB.InvView, float4(data.PositionVS, 1.0f));
	data.PositionWS = worldPosH.xyz / worldPosH.w;

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
	// Diffuse component (Lambertian)
	float NdotL = saturate(dot(N, L));
	float3 diffuseColor = diffuseAlbedo * NdotL;

	// Specular component (Blinn-Phong)
	float3 H = normalize(L + V); // Half vector
	float NdotH = saturate(dot(N, H));
	float roughnessFactor = pow(NdotH, specularPower);

	// Schlick Fresnel
	float cosIncidentAngle = saturate(dot(H, V));
	float3 reflectPercent = fresnelR0 + (1.0f - fresnelR0) * pow(1.0f - cosIncidentAngle, 5.0f);
	float3 specAlbedo = reflectPercent * roughnessFactor;

	// Scaling for LDR rendering as per your original shader
	specAlbedo = specAlbedo / (specAlbedo + 1.0f);

	return (diffuseColor + specAlbedo) * lightStrength;
}

float4 PS_PointLight(float4 position : SV_Position, float2 screenUV : TEXCOORD0) : SV_Target
{
	
	GBufferPixelData gbuffer = FetchGBufferData(screenUV);

	// Discard pixels that are fully transparent or where no geometry exists.
	// This discard is critical for performance and correctness in the light volume pass.
	if (gbuffer.Opacity < 0.01f) // Matches your GBufferPS alpha clip. Adjust threshold as needed.
	{
		discard;
	}

	// Light direction (from fragment to light) in world space
	float3 lightVec_WS = SpotLightCB.PositionWS.xyz - gbuffer.PositionWS;
	float  distance = length(lightVec_WS);

	// Early exit if outside light radius (this should largely be handled by light volume culling,
	// but a shader-side check can catch edge cases or optimize if volume is slightly larger).
	if (distance > SpotLightCB.Radius)
	{
		discard;
	}

	float3 L = normalize(lightVec_WS); // From fragment to light
	float3 V_WS = normalize(PassCB.EyePos - gbuffer.PositionWS);
	float3 N_WS = normalize(gbuffer.NormalWS);

	float cosSpotAngle = cos(SpotLightCB.SpotAngle);

	float spotDot = dot(SpotLightCB.DirectionWS.xyz, -L); // -L: from light to fragment
	float spotFactor = (spotDot > cosSpotAngle) ? 1.0 : 0.0;

	if (spotFactor == 0.0)
		discard;

	float attenuation = DoAttenuation(	SpotLightCB.ConstantAttenuation,
										SpotLightCB.LinearAttenuation,
										SpotLightCB.QuadraticAttenuation,
										distance);
	attenuation *= saturate(1.0f - pow(distance / SpotLightCB.Radius, 2.0f));

	float3 lightStrength = SpotLightCB.Color.rgb * attenuation * spotFactor;

	float3 finalLightColor = DoLightingCalculation(
		lightStrength,
		L,
		N_WS,
		V_WS,
		gbuffer.SpecularPower,
		gbuffer.Reflectance,
		gbuffer.Albedo
		);
	finalLightColor += SpotLightCB.Color.rgb * SpotLightCB.Ambient;

	return float4(finalLightColor, 1.0f);
}

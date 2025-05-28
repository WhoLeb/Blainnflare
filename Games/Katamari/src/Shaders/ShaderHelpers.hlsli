#include "ShaderStructs.hlsli"

float3 LinearToSRGB(float3 x)
{
	// This is exactly the sRGB curve
	//return x < 0.0031308 ? 12.92 * x : 1.055 * pow(abs(x), 1.0 / 2.4) - 0.055;

	// This is cheaper but nearly equivalent
	return x < 0.0031308 ? 12.92 * x : 1.13005 * sqrt(abs(x - 0.00228)) - 0.13448 * x + 0.005719;
}

float DoDiffuse( float3 N, float3 L )
{
    return max( 0, dot( N, L ) );
}

float DoSpecular( float3 V, float3 N, float3 L, float specularPower )
{
    float3 R = normalize( reflect( -L, N ) );
    float RdotV = max( 0, dot( R, V ) );

    return pow( RdotV, specularPower );
}

float DoAttenuation( float c, float l, float q, float d )
{
    return 1.0f / ( c + l * d + q * d * d );
}

float3 SchlickFresnel(float3 R0, float3 normal, float3 lightVec)
{
    float cosIncidentAngle = saturate(dot(normal, lightVec));

    float f0 = 1.0f - cosIncidentAngle;
    float3 reflectPercent = R0 + (1.0f - R0)*(f0*f0*f0*f0*f0);

    return reflectPercent;
}

float3 BlinnPhong(
    float3 lightStrength,
    float3 L,
    float3 N,
    float3 P,
    float shininess,
    float3 fresnelR0,
    float4 diffuseAlbedo)
{
    const float m = shininess * 256.0f;
    float3 halfVec = normalize(P + L);

    float roughnessFactor = (m + 8.0f)*pow(max(dot(halfVec, N), 0.0f), m) / 8.0f;
    float3 fresnelFactor = SchlickFresnel(fresnelR0, halfVec, L);

    float3 specAlbedo = fresnelFactor*roughnessFactor;

    // Our spec formula goes outside [0,1] range, but we are 
    // doing LDR rendering.  So scale it down a bit.
    specAlbedo = specAlbedo / (specAlbedo + 1.0f);

    return (diffuseAlbedo.rgb + specAlbedo) * lightStrength;
}

float DoSpotCone( float3 spotDir, float3 L, float spotAngle )
{
    float minCos = cos( spotAngle );
    float maxCos = ( minCos + 1.0f ) / 2.0f;
    float cosAngle = dot( spotDir, -L );
    return smoothstep( minCos, maxCos, cosAngle );
}

LightResult DoPointLight( PointLight light, float3 V, float3 P, float3 N, Material mat )
{
    LightResult result = (LightResult)0;
    float3 L = ( light.PositionWS.xyz - P );
    float d = length( L );
    L = L / d;

    float attenuation = DoAttenuation( light.ConstantAttenuation,
                                       light.LinearAttenuation,
                                       light.QuadraticAttenuation,
                                       d );

    float NdotL = DoDiffuse(N, L);
    float3 lightStrength = light.Color.rgb * NdotL; //attenuation;
    lightStrength *= attenuation;

    float3 c = BlinnPhong(lightStrength, L, N, V, 
                            mat.SpecularPower,
                            mat.Reflectance.rgb,
                            mat.Diffuse);

    result.Diffuse = float4(c, 1); //DoDiffuse( N, L ) * attenuation * light.Color;
    //result.Specular = DoSpecular( V, N, L, specularPower ) * attenuation * light.Color;
    //result.Ambient = light.Color * light.Ambient;

    return result;
}

LightResult DoSpotLight( SpotLight light, float3 V, float3 P, float3 N, Material mat )
{
    LightResult result;
    float3 L = ( light.PositionWS.xyz - P );
    float d = length( L );
    L = L / d;

    float NdotL = DoDiffuse(N, L);
    float3 lightStrength = light.Color.rgb * NdotL;

    float attenuation = DoAttenuation( light.ConstantAttenuation,
                                       light.LinearAttenuation,
                                       light.QuadraticAttenuation,
                                       d );

    lightStrength *= attenuation;

    float spotIntensity = DoSpotCone( light.DirectionVS.xyz, L, light.SpotAngle );
    lightStrength *= spotIntensity;

    float3 c = BlinnPhong(lightStrength, L, N, V, 
                            mat.SpecularPower,
                            mat.Reflectance.rgb,
                            mat.Diffuse);

    result.Diffuse = float4(c, 1);//DoDiffuse( N, L ) * attenuation * spotIntensity * light.Color;
    //result.Specular = DoSpecular( V, N, L, specularPower ) * attenuation * spotIntensity * light.Color;
    //result.Ambient = light.Color * light.Ambient;

    return result;
}

LightResult DoDirectionalLight( DirectionalLight light, float3 V, float3 P, float3 N, Material mat )
{
    LightResult result;

    float3 L = normalize( -light.DirectionWS.xyz );

    float3 lightStrength = light.Color.rgb * DoDiffuse( N, L );

    float3 c = BlinnPhong(lightStrength, L, N, V, 
                            mat.SpecularPower,
                            mat.Reflectance.rgb,
                            mat.Diffuse);

    result.Diffuse = float4(c, 1);//light.Color * DoDiffuse( N, L );
    //result.Specular = light.Color * DoSpecular( V, N, L, specularPower );
    //result.Ambient = light.Color * light.Ambient;

    return result;
}

LightResult DoLighting( float3 P, float3 N, Material mat )
{
    uint i;

    // Lighting is performed in view space.
    //float3 V = normalize( -P );
    float3 V = normalize(PassCB.EyePos - P);

    LightResult totalResult = (LightResult)0;

    // Iterate directinal lights
    for (i = 0; i < LightPropertiesCB.NumDirectionalLights; ++i)
    {
        LightResult result = DoDirectionalLight( DirectionalLights[i], V, P, N, mat );

        totalResult.Diffuse += result.Diffuse;
        totalResult.Specular += result.Specular;
        totalResult.Ambient += result.Ambient;
    }

    // Iterate point lights.
    for ( i = 0; i < LightPropertiesCB.NumPointLights; ++i )
    {
        LightResult result = DoPointLight( PointLights[i], V, P, N, mat );

        totalResult.Diffuse += result.Diffuse;
        totalResult.Specular += result.Specular;
        totalResult.Ambient += result.Ambient;
    }

    // Iterate spot lights.
    for ( i = 0; i < LightPropertiesCB.NumSpotLights; ++i )
    {
        LightResult result = DoSpotLight( SpotLights[i], V, P, N, mat );

        totalResult.Diffuse += result.Diffuse;
        totalResult.Specular += result.Specular;
        totalResult.Ambient += result.Ambient;
    }

    totalResult.Diffuse = saturate( totalResult.Diffuse );
    totalResult.Specular = saturate( totalResult.Specular );
    totalResult.Ambient = saturate( totalResult.Ambient );

    return totalResult;
}

float DoShadowCascade(float4 ShadowPos, Texture2D ShadowMapTex)
{
    float3 projCoords = ShadowPos.xyz / ShadowPos.w;
    projCoords.xy = 0.5 * projCoords.xy + 0.5;
    projCoords.y = 1.0f - projCoords.y;
    
    uint width, height, numMips;
    ShadowMapTex.GetDimensions(0, width, height, numMips);

    float dx = 1.0f / (float)width;

    const float2 offsets[9] =
    {
        float2(-dx, -dx), float2(0.0f, -dx), float2(dx, -dx),
        float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
        float2(-dx, +dx), float2(0.0f, +dx), float2(dx, +dx)
    };
    
    float shadow = 0.0;

    [unroll]
    for (int i = 0; i < 9; ++i)
    {
        float depth = ShadowMapTex.Sample(PointClampSamp, projCoords.xy + offsets[i]).r;
        
        if (depth + 0.0003 < projCoords.z)
        {
            shadow += 0.0;
        }
        else
        {
            shadow += 1.0;
        }
    }
    
    return shadow / 9.0f;
}

float DoShadow(float3 Position, float Distance)
{
    int chosenCascade=3;

    if(Distance < CascadeDataCB.distances.x)
        chosenCascade=0;
    else if(Distance < CascadeDataCB.distances.y)
        chosenCascade=1;
    else if(Distance < CascadeDataCB.distances.z)
        chosenCascade=2;
    else if(Distance < CascadeDataCB.distances.w)
        chosenCascade=3;

    float4 pos4 = float4(Position, 1.0);
    float4 lightViewPos = mul(pos4, CascadeDataCB.viewProjMats[chosenCascade]);

    switch(chosenCascade)
    {
    case 0:
        return DoShadowCascade(lightViewPos, ShadowMap1);
    case 1:
        return DoShadowCascade(lightViewPos, ShadowMap2);
    case 2:
        return DoShadowCascade(lightViewPos, ShadowMap3);
    case 3:
        return DoShadowCascade(lightViewPos, ShadowMap4);
    }

}
float4 DoDebugShadow(float3 Position, float Distance)
{
    int chosenCascade=0;

    if(Distance < CascadeDataCB.distances.x)
        chosenCascade=0;
    else if(Distance < CascadeDataCB.distances.y)
        chosenCascade=1;
    else if(Distance < CascadeDataCB.distances.z)
        chosenCascade=2;
    else if(Distance < CascadeDataCB.distances.w)
        chosenCascade=3;

    switch(chosenCascade)
    {
    case 0:
        return float4(1, 0, 0, 1);
    case 1:
        return float4(0, 1, 0, 1);
    case 2:
        return float4(0, 0, 1, 1);
    case 3:
        return float4(1, 1, 1, 1);
    }
}


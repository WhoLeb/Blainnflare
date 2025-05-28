struct PixelShaderInput
{
	float4 PositionH  : SV_Position;
	float3 PositionW  : POSITION0;
	float3 NormalW    : NORMAL;
	float3 TangentW   : TANGENT;
	float3 BitangentW : BITANGENT;
	float2 TexCoord   : TEXCOORD;
};

struct Material
{
	float4 Diffuse;
	//------------------------------------ ( 16 bytes )
	float4 Specular;
	//------------------------------------ ( 16 bytes )
	float4 Emissive;
	//------------------------------------ ( 16 bytes )
	float4 Ambient;
	//------------------------------------ ( 16 bytes )
	float4 Reflectance;
	//------------------------------------ ( 16 bytes )
	float Opacity; // If Opacity < 1, then the material is transparent.
	float SpecularPower;
	float IndexOfRefraction; // For transparent materials, IOR > 0.
	float BumpIntensity; // When using bump textures (height maps) we need
	// to scale the height values so the normals are visible.
	//------------------------------------ ( 16 bytes )
	bool HasAmbientTexture;
	bool HasEmissiveTexture;
	bool HasDiffuseTexture;
	bool HasSpecularTexture;
	//------------------------------------ ( 16 bytes )
	bool HasSpecularPowerTexture;
	bool HasNormalTexture;
	bool HasBumpTexture;
	bool HasOpacityTexture;
	//------------------------------------ ( 16 bytes )
	// Total:                              ( 16 * 8 = 128 bytes )
};

struct PerPassData
{
    float4x4 View;
    float4x4 InvView;
    float4x4 Proj;
    float4x4 InvProj;
    float4x4 ViewProj;
    float4x4 InvViewProj;
    float3 EyePos;
    float padding;
    float2 RenderTargetSize;
    float2 InvRenderTargetSize;
    float NearZ;
    float FarZ;
    float TotalTime;
    float DeltaTime;
};

ConstantBuffer<PerPassData> PassCB : register( b0 );

ConstantBuffer<Material> MaterialCB : register( b1 );


// Textures
Texture2D AmbientTexture       : register( t1 );
Texture2D EmissiveTexture      : register( t2 );
Texture2D DiffuseTexture       : register( t3 );
Texture2D SpecularTexture      : register( t4 );
Texture2D SpecularPowerTexture : register( t5 );
Texture2D NormalTexture        : register( t6 );
Texture2D BumpTexture          : register( t7 );
Texture2D OpacityTexture       : register( t8 );

SamplerState PointWrapSamp     : register( s0 );
SamplerState PointClampSamp    : register( s1 );
SamplerState LinWrapSamp       : register( s2 );
SamplerState LinClampSamp      : register( s3 );
SamplerState AniWrapSamp       : register( s4 );
SamplerState AniClampSamp      : register( s5 );

float3 LinearToSRGB(float3 x)
{
    // This is exactly the sRGB curve
    //return x < 0.0031308 ? 12.92 * x : 1.055 * pow(abs(x), 1.0 / 2.4) - 0.055;

    // This is cheaper but nearly equivalent
    return x < 0.0031308 ? 12.92 * x : 1.13005 * sqrt(abs(x - 0.00228)) - 0.13448 * x + 0.005719;
}

float3 ExpandNormal(float3 n)
{
    return n * 2.0f - 1.0f;
}

float3 DoNormalMapping(float3x3 TBN, Texture2D tex, float2 uv)
{
    float3 N = tex.Sample(AniWrapSamp, uv).xyz;
    N = ExpandNormal(N);

    // Transform normal from tangent space to view space.
    N = mul(N, TBN);
    return normalize(N);
}

float3 DoBumpMapping(float3x3 TBN, Texture2D tex, float2 uv, float bumpScale)
{
    // Sample the heightmap at the current texture coordinate.
    float height_00 = tex.Sample(AniWrapSamp, uv).r * bumpScale;
    // Sample the heightmap in the U texture coordinate direction.
    float height_10 = tex.Sample(AniWrapSamp, uv, int2(1, 0)).r * bumpScale;
    // Sample the heightmap in the V texture coordinate direction.
    float height_01 = tex.Sample(AniWrapSamp, uv, int2(0, 1)).r * bumpScale;

    float3 p_00 = { 0, 0, height_00 };
    float3 p_10 = { 1, 0, height_10 };
    float3 p_01 = { 0, 1, height_01 };

    // normal = tangent x bitangent
    float3 tangent = normalize(p_10 - p_00);
    float3 bitangent = normalize(p_01 - p_00);

    float3 normal = cross(tangent, bitangent);

    // Transform normal from tangent space to view space.
    normal = mul(normal, TBN);

    return normal;
}


// If c is not black, then blend the color with the texture
// otherwise, replace the color with the texture.
float4 SampleTexture(Texture2D t, float2 uv, float4 c)
{
    if (any(c.rgb))
    {
        c *= t.Sample(AniWrapSamp, uv);
    }
    else
    {
        c = t.Sample(AniWrapSamp, uv);
    }

    return c;
}

struct GBufferOutput
{
    float4 RT0_AlbedoOpacity : SV_Target0;
    float4 RT1_Normal : SV_Target1;
    float4 RT2_ReflectanceSpec : SV_Target2;
    float4 RT3_EmissiveAmbient : SV_Target3;
};

GBufferOutput PS_GBuffer(PixelShaderInput IN)
{
    GBufferOutput result;
    Material material = MaterialCB;
    
    float alpha = material.Diffuse.a;
    if (material.HasOpacityTexture)
    {
        alpha = OpacityTexture.Sample(AniWrapSamp, IN.TexCoord.xy).r;
    }

    float4 ambient = material.Ambient;
    float4 emissive = material.Emissive;
    float4 diffuse = material.Diffuse;
    float specularPower = material.SpecularPower;
    float2 uv = IN.TexCoord.xy;
    
    if (material.HasAmbientTexture)
    {
        ambient = SampleTexture(AmbientTexture, uv, ambient);
    }
    if (material.HasEmissiveTexture)
    {
        emissive = SampleTexture(EmissiveTexture, uv, emissive);
    }
    if (material.HasDiffuseTexture)
    {
        diffuse = SampleTexture(DiffuseTexture, uv, diffuse);
    }
    if (material.HasSpecularPowerTexture)
    {
        specularPower *= SpecularPowerTexture.Sample(AniWrapSamp, uv).r;
    }

    float3 N;
    float3 tangent = normalize(IN.TangentW);
    float3 bitangent = normalize(IN.BitangentW);
    float3 normal = normalize(IN.NormalW);

    // Normal mapping
    if (material.HasNormalTexture)
    {
        float3x3 TBN = float3x3(tangent,
                                 bitangent,
                                 normal);

        N = DoNormalMapping(TBN, NormalTexture, uv);
    }
    else if (material.HasBumpTexture)
    {
        float3x3 TBN = float3x3(tangent,
                                 -bitangent,
                                 normal);

        N = DoBumpMapping(TBN, BumpTexture, uv, material.BumpIntensity);
    }
    else
    {
        N = normalize(IN.NormalW);
    }

    result.RT0_AlbedoOpacity = float4(diffuse.rgb, alpha);
    result.RT1_Normal = float4(N * 0.5f + 0.5f, 0.f);
    result.RT2_ReflectanceSpec  = float4(material.Reflectance.rgb, saturate(specularPower/256.0f));
    result.RT3_EmissiveAmbient = float4(emissive.rgb, ambient.a);

    return result;
}

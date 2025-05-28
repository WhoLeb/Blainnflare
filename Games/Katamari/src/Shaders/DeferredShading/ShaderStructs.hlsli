#define CASCADE_COUNT 4

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
struct PointLight
{
    float4 PositionWS; // Light position in world space.
    //----------------------------------- (16 byte boundary)
    float4 PositionVS; // Light position in view space.
    //----------------------------------- (16 byte boundary)
    float4 Color;
    //----------------------------------- (16 byte boundary)
    float  Ambient;
    float  ConstantAttenuation;
    float  LinearAttenuation;
    float  QuadraticAttenuation;
    //----------------------------------- (16 byte boundary)
	float Radius;
	float3 Padding0;
    //----------------------------------- (16 byte boundary)
    // Total:                              16 * 5 = 80 bytes
};

struct SpotLight
{
    float4 PositionWS; // Light position in world space.
    //----------------------------------- (16 byte boundary)
    float4 PositionVS; // Light position in view space.
    //----------------------------------- (16 byte boundary)
    float4 DirectionWS; // Light direction in world space.
    //----------------------------------- (16 byte boundary)
    float4 DirectionVS; // Light direction in view space.
    //----------------------------------- (16 byte boundary)
    float4 Color;
    //----------------------------------- (16 byte boundary)
    float  Ambient;
    float  SpotAngle;
    float  ConstantAttenuation;
    float  LinearAttenuation;
    //----------------------------------- (16 byte boundary)
    float  QuadraticAttenuation;
	float  Radius;
    float2 Padding;
    //----------------------------------- (16 byte boundary)
    // Total:                              16 * 7 = 112 bytes
};

struct DirectionalLight
{
    float4 DirectionWS;  // Light direction in world space.
    //----------------------------------- (16 byte boundary)
    float4 DirectionVS;  // Light direction in view space.
    //----------------------------------- (16 byte boundary)
    float4 Color;
    //----------------------------------- (16 byte boundary)
    float Ambient;
    float3 Padding;
    //----------------------------------- (16 byte boundary)
    // Total:                              16 * 4 = 64 bytes
};

struct LightProperties
{
    uint NumPointLights;
    uint NumSpotLights;
    uint NumDirectionalLights;
};

struct LightResult
{
    float4 Diffuse;
    float4 Specular;
    float4 Ambient;
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

struct CascadeData
{
	float4x4 viewProjMats[CASCADE_COUNT];
	float4 distances;
};

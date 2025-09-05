struct VertexPosition
{
	float3 PositionOS : POSITION;
};

struct VertexShaderOutput
{
	float4 PositionH : SV_POSITION;
	float2 ScreenUV : TEXCOORD0;
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

ConstantBuffer<PerPassData> PerPassCB : register(b0);

struct PerInstanceData
{
	float4x4 World;
};

ConstantBuffer<PerInstanceData> PerInstanceCB : register(b1, space0);

VertexShaderOutput VS_LightVolume(VertexPosition IN : POSITION)
{
	VertexShaderOutput OUT;

	float4 worldPos = mul(float4(IN.PositionOS, 1.0f), PerInstanceCB.World);
	OUT.PositionH = mul(worldPos, PerPassCB.ViewProj);
	OUT.ScreenUV = OUT.PositionH.xy / OUT.PositionH.w; // Get NDC
	OUT.ScreenUV = OUT.ScreenUV * float2(0.5f, -0.5f) + 0.5f; // Map to [0,1] and flip Y
	
	return OUT;
}


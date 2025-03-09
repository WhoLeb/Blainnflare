cbuffer cbPerObject : register(b0)
{
    float4x4 gWorld;
};

cbuffer cbPass : register(b1)
{
    float4x4 gView;
    float4x4 gInvView;
    float4x4 gProj;
    float4x4 gInvProj;
    float4x4 gViewProj;
    float4x4 gInvViewProj;
    float3 gEyePos;
    float padding;
    float2 gRenderTargetSize;
    float2 gInvRenderTargetSize;
    float gNearZ;
    float gFarZ;
    float gTotalTime;
    float gDeltaTime;
};

struct VSin
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float4 color : COLOR;
    float2 uv : UV;
};

struct VSout
{
    float4 posH : SV_POSITION;
    float3 posW : POSITION;
    float3 normal : NORMAL;
    float4 color : COLOR;
    float2 uv : UV;
};

VSout VSmain(VSin vin)
{
    VSout vout;
	
    float4 posW = mul(float4(vin.pos, 1.f), gWorld);
    vout.posH = mul(posW, gViewProj);

    vout.posW = vin.pos;
    vout.color = vin.color;
    vout.normal = vin.normal;
    vout.uv = vin.uv;

    return vout;
}

float4 PSmain(VSout pin) : SV_Target
{
    return pin.color;
}




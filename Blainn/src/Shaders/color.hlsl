cbuffer cbPerObject : register(b0)
{
	float4x4 gWorldViewProj;
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
	
	vout.posH = mul(float4(vin.pos, 1.f), gWorldViewProj);

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




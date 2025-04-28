struct ObjectData
{
	float4x4 WorldMatrix;
};

ConstantBuffer<ObjectData> ObjectCB : register(b0);

struct PassData
{
	float4x4 View;
	float4x4 Proj;
	float4x4 ViewProj;
};

ConstantBuffer<PassData> PassCB : register(b1);

struct VertexPositionNormalTangentBitangentTexture
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float3 Bitangent : BITANGENT;
    float3 TexCoord : TEXCOORD;
};

float4 main(VertexPositionNormalTangentBitangentTexture IN) : SV_Position
{
	float4 posW = mul(float4(IN.Position, 1), ObjectCB.WorldMatrix);
	float4 posV = mul(posW, PassCB.ViewProj);
	return posV;
}

void mainPS()

{
}
struct ObjectData
{
	float4x4 WorldMatrix;
};

StructuredBuffer<ObjectData> ObjectSB : register(t0);

struct PassData
{
	float4x4 ViewProj;
};

ConstantBuffer<PassData> PassCB : register(b0);

struct VertexPositionNormalTangentBitangentTexture
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float3 Bitangent : BITANGENT;
    float3 TexCoord : TEXCOORD;
	uint InstanceID : SV_INSTANCEID;
};

float4 main(VertexPositionNormalTangentBitangentTexture IN) : SV_Position
{
	float4 posW = mul(float4(IN.Position, 1), ObjectSB[IN.InstanceID].WorldMatrix);
	float4 posV = mul(posW, PassCB.ViewProj);
	return posV;
}

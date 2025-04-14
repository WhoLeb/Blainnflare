// clang-format off

#include "Matrix.hlsli"

struct PerObjectData
{
    float4x4 WorldMatrix;
};

ConstantBuffer<PerObjectData> ObjectCB : register(b0);

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

ConstantBuffer<PerPassData> PassCB : register(b1);

struct VertexPositionNormalTangentBitangentTexture
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float3 Bitangent : BITANGENT;
    float3 TexCoord : TEXCOORD;
};

struct VertexShaderOutput
{
    float4 PositionVS : POSITION;
    float3 NormalVS : NORMAL;
    float3 TangentVS : TANGENT;
    float3 BitangentVS : BITANGENT;
    float2 TexCoord : TEXCOORD;
    float4 Position : SV_Position;
};

VertexShaderOutput main(VertexPositionNormalTangentBitangentTexture IN)
{
    VertexShaderOutput OUT;

    float4x4 ModelViewMatrix = mul(ObjectCB.WorldMatrix, PassCB.View);
    float4x4 ModelViewProjMatrix = mul(ObjectCB.WorldMatrix, PassCB.ViewProj);

    float3x3 MV3x3 = (float3x3)ModelViewMatrix;
    float3x3 invMV3x3 = Inverse3x3(MV3x3);
    float3x3 InverseTransposeMV3x3 = invMV3x3;

    OUT.PositionVS = mul(float4(IN.Position, 1.0f), ModelViewMatrix);
    OUT.Position = mul(float4(IN.Position, 1.0f), ModelViewProjMatrix);

    OUT.NormalVS = mul(IN.Normal, InverseTransposeMV3x3);
    OUT.TangentVS = mul(IN.Tangent, InverseTransposeMV3x3);
    OUT.BitangentVS = mul(IN.Bitangent, InverseTransposeMV3x3);

    OUT.TexCoord = IN.TexCoord.xy;

    return OUT;
}
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
    float3 Position  : POSITION;
    float3 Normal    : NORMAL;
    float3 Tangent   : TANGENT;
    float3 Bitangent : BITANGENT;
    float3 TexCoord  : TEXCOORD;
};

struct VertexShaderOutput
{
    float4 PositionH  : SV_Position;
    float3 PositionW  : POSITION;
    float3 NormalW    : NORMAL;
    float3 TangentW   : TANGENT;
    float3 BitangentW : BITANGENT;
    float2 TexCoord   : TEXCOORD;
};

VertexShaderOutput main(VertexPositionNormalTangentBitangentTexture IN)
{
    VertexShaderOutput OUT;

    float4 posW = mul(float4(IN.Position, 1.0f), ObjectCB.WorldMatrix);

    OUT.PositionW = posW.xyz;

    //float4x4 MV = ModelViewMatrix;
    //float4x4 invMV = inverse(MV);
    //float4x4 InverseTransposeMV = (invMV);

    OUT.PositionH = mul(posW, PassCB.ViewProj);

    float3x3 world3x3 = (float3x3)ObjectCB.WorldMatrix;
    float3x3 invTransWorld3x3 = transpose(Inverse3x3(world3x3));

    OUT.NormalW = mul(invTransWorld3x3, IN.Normal);
    OUT.TangentW = mul(invTransWorld3x3, IN.Tangent);
    OUT.BitangentW = mul(invTransWorld3x3, IN.Bitangent);

    //OUT.NormalVS = mul(InverseTransposeMV, IN.Normal);
    //OUT.TangentVS = mul(InverseTransposeMV, IN.Tangent);
    //OUT.BitangentVS = mul(InverseTransposeMV, IN.Bitangent);
    
    //OUT.NormalVS = mul(MV3x3, IN.Normal);
    //OUT.TangentVS = mul(MV3x3, IN.Tangent);
    //OUT.BitangentVS = mul(MV3x3, IN.Bitangent);

    OUT.TexCoord = IN.TexCoord.xy;

    return OUT;
}
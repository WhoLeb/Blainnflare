
#ifndef NUM_DIR_LIGHTS
#define  NUM_DIR_LIGHTS 3
#endif
#ifndef NUM_POINT_LIGHTS
    #define NUM_POINT_LIGHTS 1
#endif

#ifndef NUM_SPOT_LIGHTS
    #define NUM_SPOT_LIGHTS 0
#endif

#include "LightingUtil.hlsl"

Texture2D gDiffuseMap : register(t0);
SamplerState gsamLinear : register(s0);

cbuffer cbPerObject : register(b0)
{
    float4x4 gWorld;
};

cbuffer cbMaterial : register(b1)
{
    float4 gDiffuseAlbedo;
    float3 gFrensel;
    float  gRoughness;
    float4x4 gMatTransform;
}

cbuffer cbPass : register(b2)
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

    float4 gAmbientLight;
    Light gLights[MaxLights];

};

struct VSin
{
    uint vertexID : SV_VertexID;
    float3 posL : POSITION;
    float3 normalL : NORMAL;
    float4 color : COLOR;
    float2 uv : UV;
};

struct VSout
{
    float4 posH   : SV_POSITION;
    float3 posW   : POSITION;
    float3 normal : NORMAL;
    float4 color  : COLOR;
    float2 uv     : TEXCOORD;
};

VSout VSmain(VSin vin)
{
    VSout vout = (VSout)0.0f;

    float4 posW = mul(float4(vin.posL, 1.f), gWorld);
    vout.posW = posW.xyz;

    vout.normal = mul(vin.normalL, (float3x3)gWorld);

    vout.posH = mul(posW, gViewProj);

    //float4 texc = mul(float4(vin.uv, 0.0f, 1.0f), gTexTransform);
    vout.uv = mul(float4(vin.uv, 0.0f, 1.0f), gMatTransform).xy;

    vout.color = vin.color;

    return vout;
}

float4 PSmain(VSout pin) : SV_Target
{
    float4 diffuseAlbedo = gDiffuseMap.Sample(gsamLinear, pin.uv) * gDiffuseAlbedo;

    pin.normal = normalize(pin.normal);

    float3 toEyeW = normalize(gEyePos - pin.posW);
    
    float4 ambient = gAmbientLight * diffuseAlbedo;

    const float shininess = 1.0f - gRoughness;
    Material mat = { diffuseAlbedo, gFrensel, shininess };
    float3 shadowFactor = 1.0f;
    float4 directLight = ComputeLighting(gLights, mat, pin.posW,
        pin.normal, toEyeW, shadowFactor);

    float4 litColor = ambient + directLight;

    litColor.a = diffuseAlbedo.a;

    return litColor;
}




struct VertexShaderOutput
{
	float4 PositionH : SV_Position;
	float2 TexCoord : TEXCOORD0;
};

VertexShaderOutput VS_TexturedQuad(float3 pos : POSITION, uint VertexID : SV_VertexID)
{
	VertexShaderOutput OUT;

	float2 positions[4] = {
		float2(0.0f, 0.0f),
		float2(1.0f, 0.0f),
		float2(1.0f, 1.0f),
		float2(0.0f, 1.0f)
    };
	OUT.PositionH = float4(pos.xy, 0.f, 1.f);
	OUT.TexCoord = positions[VertexID];
	return OUT;
}

Texture2D Texture : register(t0);

SamplerState PointWrapSamp : register(s0);

float4 PS_TexturedQuad(VertexShaderOutput IN) : SV_Target0
{
	return Texture.Sample(PointWrapSamp, IN.TexCoord);
}
struct VertexShaderOutput
{
	float4 PositionH : SV_Position;
	float2 TexCoord : TEXCOORD0;
};

VertexShaderOutput VS_FullScreenQuad(float3 pos : POSITION, uint VertexID : SV_VertexID)
{
	VertexShaderOutput OUT;

	float2 positions[4] = {
		float2(-1.0f, -1.0f),
		float2(-1.0f,  1.0f),
		float2( 1.0f, -1.0f),
		float2( 1.0f,  1.0f),
	};
	OUT.PositionH = float4(positions[VertexID], 0.f, 1.f);
	//OUT.TexCoord = (positions[VertexID] * 0.5f) + 0.5f;
	OUT.TexCoord = float2(positions[VertexID].x * 0.5f + 0.5f, -positions[VertexID].y * 0.5f + 0.5f);	
	return OUT;
}
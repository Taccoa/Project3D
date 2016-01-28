struct VS_IN
{
	float3 Pos : POSITION;
	/*float2 Tex : TEXCOORD;*/
};

struct VS_OUT
{
	float4 Pos : SV_POSITION;
	/*float2 Tex : TEXCOORD;*/
};

//-----------------------------------------------------------------------------------------
// VertexShader: VSScene
//-----------------------------------------------------------------------------------------

cbuffer VS_CONSTANT_BUFFER : register(b0)
{
	matrix worldViewProj;
	matrix world;
};

VS_OUT VS_main(VS_IN input)
{
	VS_OUT output = (VS_OUT)0;

	//Sends the Position and the Texure to the Geometry Shader

	output.Pos = mul(float4(input.Pos, 1), worldViewProj);
	/*output.Tex = input.Tex;*/

	return output;
}
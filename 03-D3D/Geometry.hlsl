struct GS_IN
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD;
};

struct GS_OUT
{
	float4 Pos : SV_POSITION;
	float4 PosWorld : POSITION;
	float2 Tex : TEXCOORD;
	float3 Nor : NORMAL;
};
cbuffer VS_CONSTANT_BUFFER : register(b0)
{
	matrix worldViewProj;
	matrix world;
};

[maxvertexcount(6)]
void GS_main(
	triangle GS_IN input[3], 
	inout TriangleStream< GS_OUT > outputStream
)
{
	GS_OUT output = (GS_OUT)0;

	//Calculates the Normal

	float3 edge0 = input[1].Pos - input[0].Pos;
	float3 edge1 = input[2].Pos - input[0].Pos;
	float3 normal = cross(edge0, edge1);
	normalize(normal);

	output.Nor = mul(normal, world); //Sends the Normal in world view to the Pixel Shader

	for (uint i = 0; i < 3; i++)
	{
		//Creates the first Quads Geometry
		output.Pos = mul(input[i].Pos, worldViewProj);
		output.PosWorld = mul(input[i].Pos, world);
		output.Tex = input[i].Tex;
		outputStream.Append(output);
	}
	//outputStream.RestartStrip();
	/*for (uint i = 0; i < 3; i++)
	{
		//Creates the second Quads Geometry
		output.Pos = mul(input[i].Pos + float4(normal, 0), worldViewProj);
		output.PosWorld = mul(input[i].Pos + float4(normal, 0), world);
		output.Tex = input[i].Tex;
		outputStream.Append(output);
	}*/
}
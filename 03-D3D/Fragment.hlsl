Texture2D txDiffuse : register(t0);
SamplerState sampAni;
struct GS_OUT
{
	float4 Pos : SV_POSITION;
	float4 PosWorld : POSITION;
	float2 Tex : TEXCOORD;
	float3 Nor : NORMAL;
};

float4 PS_main(GS_OUT input) : SV_Target
{
	float4 lightPosition = float4(0.0, 0.0, -10.0, 0.0);
	float3 lightIntensity = float3(1.0, 1.0, 1.0);

	float4 s = normalize(lightPosition - input.PosWorld); //Distance between the Object and the Light

	float3 Kd = txDiffuse.Sample(sampAni, input.Tex).xyz; //Gets the texture and puts it with the UV Coordinates on the Quad

	input.Nor = normalize(input.Nor);

	float3 diffuseLight = Kd * max(dot(s, input.Nor), 0.0f); //Calculates the Diffuse Light by taking "the Alpha" Angle times Kd

	float4 rt = float4((lightIntensity * (diffuseLight)) , 1.0f); //Calculates the Light Intensity time the Diffuse Light

	return (1, 1, 1, 1);
};
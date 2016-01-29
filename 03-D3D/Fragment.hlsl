Texture2D txDiffuse : register(t0);
SamplerState sampAni;
struct PS_IN
{
	float4 Pos : SV_POSITION;
	float3 Nor : NORMAL;
	float2 Tex : TEXCOORD;
	float4 wPos : POSITION;
	float3 Col : COLOR;
};

float4 PS_main(PS_IN input) : SV_Target
{
	//float4 lightPosition = float4(0.0, 0.0, -10.0, 0.0);
	//float3 lightSourceIntensity = float3(1.0, 1.0, 1.0);

	//input.Nor = normalize(input.Nor);

	//float4 s = normalize(lightPosition - input.wPos); //The direction from the surface to the light source.

	//float3 Kd = txDiffuse.Sample(sampAni, input.Col).xyz;

	//float3 diffuseLight = Kd * max(dot(s, input.Nor), 0); //To calculate the diffuse.

	//Finally the light source intensity is multiplied with (A + D + S), which now only have the diffuse.
	//return float4(lightSourceIntensity * (diffuseLight), 1); 

	return float4(0.5, 0.5, 1, 0);

};
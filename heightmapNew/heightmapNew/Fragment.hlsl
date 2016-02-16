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
	float3 cameraPosition = float3(0.0, 0.0, 0.0);

	float4 s = normalize(lightPosition - input.PosWorld); //Distance between the Object and the Light

	float3 Kd = txDiffuse.Sample(sampAni, input.Tex).xyz; //Gets the texture and puts it with the UV Coordinates on the Quad
	float3 v = normalize(cameraPosition - input.PosWorld); //distance between the objekt and the camera

	float3 color = txDiffuse.Sample(sampAni, input.Tex).xyz; //Gets the texture and puts it with the UV Coordinates on the Quad

	input.Nor = normalize(input.Nor);

	float3 r = reflect(-s, input.Nor);
	float3 ka = float3(0.2, 0.2, 0.2);
	float3 ks = float3(1.0, 1.0, 1.0);

	float shinyPower = 2000.0f;

	float3 diffuseLight = color * max(dot(s, input.Nor), 0.0f); //Calculates the Diffuse Light by taking "the Alpha" Angle times Kd
	float3 ambientLight = color * ka;
	float3 specularLight = ks * pow(max(dot(r, v), 0.0f), shinyPower);

	float4 rt = float4((lightIntensity * (diffuseLight)+(ambientLight)+(specularLight)), 1.0f); //Calculates the Light Intensity time the Diffuse Light

	return rt;
};
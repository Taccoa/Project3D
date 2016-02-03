Texture2D txDiffuse : register(t0);
SamplerState sampAni;
struct GS_OUT
{
	float4 Pos : SV_POSITION;
	float3 Nor : NORMAL;
	float2 Tex : TEXCOORD;
	float4 WPos : POSITION;
};

cbuffer MaterialBuffer
{
	float3 ambient;
	float transparency;
	
	float3 diffuse;
	float shininess;

	float3 specular;
	float reflection;

	//float3 emissive;

};

float4 PS_main(GS_OUT input) : SV_Target
{
	float4 lightPosition = float4(0.0, 0.0, -20.0, 0.0);
	float3 lightIntensity = float3(0.7, 0.7, 0.7);
	float3 cameraPosition = float3(0.0, 0.0, -4.0);

	float4 s = normalize(lightPosition - input.WPos); //Distance between the Object and the Light
	float3 v = normalize(cameraPosition - input.WPos); //distance between the objekt and the camera

	float3 color = txDiffuse.Sample(sampAni, input.Tex).xyz; //Gets the texture and puts it with the UV Coordinates on the Quad

	input.Nor = normalize(input.Nor);

	float3 r = reflect(-s, input.Nor);

	float3 diffuseLight = diffuse * max(dot(s, input.Nor), 0.0f); //Calculates the Diffuse Light by taking "the Alpha" Angle times Kd
	float3 ambientLight = ambient;
	float3 specularLight = specular * pow(max(dot(r, v), 0.0f), shininess);

	float4 rt = float4(lightIntensity * (diffuseLight + ambientLight + specularLight), 0.0f); //Calculates the Light Intensity time the Diffuse Light

	return rt;
};
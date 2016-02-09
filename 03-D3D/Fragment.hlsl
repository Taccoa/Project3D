Texture2D shaderTexture : register(t0);

SamplerState sampAni
{
	Filter = MIN_MAG_MIP_LINEAR;
	AdressU = Wrap;
	AdressV = Wrap;
};

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

	bool textureBool;
	float3 padding;

	float3 camPos;
	float padding2;

	//float3 emissive;
};

float4 PS_main(GS_OUT input) : SV_Target
{
	float4 lightPosition = float4(0.0, 0.0, -10.0, 0.0);
	float3 lightIntensity = float3(0.9, 0.9, 0.9);

	float4 s = normalize(lightPosition - input.WPos); //Distance between the Object and the Light
	float3 v = normalize(camPos - input.WPos); //Distance between the Object and the Camera

	float3 color;

	input.Nor = normalize(input.Nor);

	float3 r = reflect(-s, input.Nor);

	float3 diffuseLight;
	float3 ambientLight;
	float3 specularLight;
	float4 rt;

	//diffuseLight = color * diffuse * max(dot(s, input.Nor), 0.0f);
	//ambientLight = color * ambient + float3(0.7, 0.7, 0.7);
	//specularLight = color * specular * pow(max(dot(r, v), 0.0f), shininess);

	if (textureBool == true)
	{
		color = shaderTexture.Sample(sampAni, input.Tex).xyz; //Gets the texture and puts it with the UV Coordinates on the Quad

		diffuseLight = color * diffuse * max(dot(s, input.Nor), 0.0f); //Calculates the Diffuse Light by taking "the Alpha" Angle times Kd
		ambientLight = color * ambient + float3(0.2, 0.2, 0.2);
		specularLight = color * specular * pow(max(dot(r, v), 0.0f), shininess);
	}
	else
	{
		diffuseLight = diffuse * max(dot(s, input.Nor), 0.0f); //Calculates the Diffuse Light by taking "the Alpha" Angle times Kd
		ambientLight = ambient + float3(0.2, 0.2, 0.2);
		specularLight = specular * pow(max(dot(r, v), 0.0f), shininess);
	}

	if (specular.x == 0 && specular.y == 0 && specular.z == 0) //Required to check if there is a specular attribute. 
	{
		rt = float4(lightIntensity * (diffuseLight + ambientLight), 0.0f); //Remove specular calculation from the light equation. 

	}

	else //Add specular calculation if there is a attribute from the material. 
	{
		rt = float4(lightIntensity * (diffuseLight + ambientLight + specularLight), 0.0f);

	}

	return rt;

};
struct DirectionalLight
{
	float3 direction;
	float padding;
	float4 colour;
};

struct Pointlight
{
	float4 position;
	float4 colour;
	float range;
	float3 padding2;
};

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
cbuffer ConstantBuffer : register(b0)
{
	float4x4 World;
	float4x4 View;
	float4x4 Projection;
	float4 CameraPosition;
	float4 Colour;
	float Time;
	float3 padding3;
}

//A lighting buffer would be nice, could do with setting ambient light in here too

cbuffer LightingBuffer : register (b1)
{
	float numDirLights;
	float3 padding4;
	DirectionalLight dirLights[2];

	float numPointLights; //5 max
	float3 padding5;
	Pointlight pointLights[5];
}

Texture2D txDiffuse : register(t0);
SamplerState txDiffSampler : register(s0);

Texture2D txBump : register(t1);
SamplerState txBumpSampler : register(s1);


//--------------------------------------------------------------------------------------
// Shader Inputs
//--------------------------------------------------------------------------------------
struct VS_INPUT
{
	float3 Pos : POSITION;
	float3 Normal : NORMAL;
	//float3 Tangent : TANGENT;
	//float3 Binormal : BINORMAL;
	float2 TexCoord : TEXCOORD0;
	//float3 InstancePos : INSTANCEPOS;
};

struct PS_INPUT
{
	float4 Pos : SV_POSITION;
	float3 Normal: NORMAL;
	float4 PosWorld : TEXCOORD0;
	float2 TexCoord : TEXCOORD1;
};


//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS(VS_INPUT input)
{
	PS_INPUT output = (PS_INPUT)0;
	output.Pos = mul(float4(input.Pos, 1.0f), World);
	output.Pos = mul(output.Pos, View);
	output.Pos = mul(output.Pos, Projection);
	output.Normal = mul(World, float4(input.Normal, 1.0f)).xyz;
	output.Normal = normalize(output.Normal);
	output.PosWorld = mul(float4(input.Pos, 1.0f), World);
	output.TexCoord = input.TexCoord;
	output.TexCoord.y = 1 - output.TexCoord.y;

	return output;
}

float4 CalcLightColour(float4 matDiffuse, float4 matSpec, float3 viewDirection, float3 lightDir, float4 lightColour, PS_INPUT input)
{
	float diffuse = max(0.0, dot(lightDir, input.Normal));
	float3 R = normalize(reflect(-lightDir, input.Normal));
	float spec = pow(max(0.0, dot(viewDirection, R)), 50);

	return (lightColour * matDiffuse * diffuse) + (lightColour * matSpec * spec);
}

float3x3 cotangent_frame(float3 N, float3 p, float2 uv)
{
	// get edge vectors of the pixel triangle
	float3 dp1 = ddx(p);
	float3 dp2 = ddy(p);
	float2 duv1 = ddx(uv);
	float2 duv2 = ddy(uv);

	// solve the linear system
	float3 dp2perp = cross(dp2, N);
	float3 dp1perp = cross(N, dp1);
	float3 T = dp2perp * duv1.x + dp1perp * duv2.x;
	float3 B = dp2perp * duv1.y + dp1perp * duv2.y;

	// construct a scale-invariant frame 
	float invmax = rsqrt(max(dot(T, T), dot(B, B)));
	return transpose(float3x3(T * invmax, B * invmax, N));
}

float3 perturb_normal(float3 N, float3 V, float2 texcoord)
{
	// assume N, the interpolated vertex normal and 
	// V, the view vector (vertex to eye)
	float3 map = txBump.Sample(txBumpSampler, texcoord).xyz;
	////float4 texColour = txDiffuse.Sample(txSampler, input.TexCoord);

	//map.y = -map.y;
	float3x3 TBN = cotangent_frame(N, -V, texcoord);
	return normalize(mul(TBN, map));
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(PS_INPUT input) : SV_Target
{
	float4 matDiffuse = txDiffuse.Sample(txDiffSampler, input.TexCoord);// *Tint;
	float4 matSpec = float4(1.0, 1.0, 1.0, 1.0);
	float4 ambient = float4(0.01, 0.01, 0.01, 1.0);

	float3 viewDirection = normalize(CameraPosition - input.PosWorld);
	float4 outputCol = ambient;// *matDiffuse;

	input.Normal = perturb_normal(normalize(input.Normal), viewDirection, input.TexCoord);

	//Calc directional lights
	for (int i = 0; i < numDirLights; ++i)
	{
		float4 lightColour = CalcLightColour(matDiffuse, matSpec, viewDirection, dirLights[i].direction, dirLights[i].colour, input);
		outputCol = saturate(lightColour + outputCol);
	}

	//Test directional light - Remove when buffers in & uncomment above
	//DirectionalLight testDir;
	//testDir.direction = normalize(float3(0.5, 1, -0.5));
	//testDir.colour = float4(0.15, 0.15, 0.15, 1);
	//outputCol += CalcLightColour(matDiffuse, matSpec, viewDirection, testDir.direction, testDir.colour, input);


	//Calc spotlights

	for (int i = 0; i < numPointLights; ++i)
	{
		float3 lightDir = normalize(pointLights[i].position - input.PosWorld);

		float intensity = 1 - min(distance(pointLights[i].position.xyz, input.PosWorld.xyz) / pointLights[i].range, 1);
		float4 lightColour = CalcLightColour(matDiffuse, matSpec, viewDirection, lightDir, pointLights[i].colour, input) * intensity;
		outputCol = saturate(lightColour + outputCol);
	}


	//Test spotlight - Remove when buffers in & uncomment above
	/*Pointlight testPoint;
	testPoint.position = float4(0, 0, -96, 1);
	testPoint.colour = float4(1, 0, 0, 1);
	testPoint.range = 10;
	float3 lightDir = normalize(testPoint.position - input.PosWorld);

	float intensity =  1 - min(distance(testPoint.position.xyz, input.PosWorld.xyz) / testPoint.range, 1);
	float4 lightColour = CalcLightColour(matDiffuse, matSpec, viewDirection, lightDir, testPoint.colour, input) * intensity;
	outputCol += saturate(lightColour + outputCol);
*/


//TODO: Spotlights


return outputCol;

}

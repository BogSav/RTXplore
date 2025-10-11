#include "RasterizationHelper.hlsl"

struct VertexIn
{
	float3 Position : Position;
	float2 TextureC : TextureC;
};

struct VertexOut
{
	float3 Position : Position;
	float2 TextureC : TextureC;
};

struct HullOut
{
	float3 Position : Position;
	float2 TextureC : TextureC;
};

struct PatchHullOut
{
	float EdgeTess[3] : SV_TessFactor;
	float InsideTess[1] : SV_InsideTessFactor;
};

struct DomainOut
{
	float4 Position : SV_POSITION;
	float3 WorldPosition : WorldPosition;
	float3x3 TBNMatrix : TBNMatrix;
	float3 Normal : Normal;
	float2 TextureC : TextureC;
    float Depth : Depth;
};

//////////////////////////////////////////////////////////////////////////
// Vertex shader 
VertexOut VSMain(VertexIn input)
{
	VertexOut vertexOut;

	vertexOut.Position = input.Position;
	vertexOut.TextureC = mul(float4(input.TextureC, 0, 1), objectCB.textureTransform).xy;

	return vertexOut;
}

//////////////////////////////////////////////////////////////////////////
// Hull shader 
PatchHullOut ConstantHS(InputPatch<VertexOut, 3> patch, uint patchId : SV_PrimitiveID)
{
	PatchHullOut output;

	float3 center = (patch[0].Position + patch[1].Position + patch[2].Position).xyz / 3.f;

	const float distanceFromCamera = distance(center, passCB.eyePosition);

	const float d0 = passCB.nearZ;
	const float d1 = 30.f;

	float tess = MAX_TESSELATION_FACTOR * saturate((d1 - distanceFromCamera) / (d1 - d0));
	tess = clamp(tess, 1.f, MAX_TESSELATION_FACTOR);

	output.EdgeTess[0] = tess;
	output.EdgeTess[1] = tess;
	output.EdgeTess[2] = tess;

	output.InsideTess[0] = tess;

	return output;
}

[domain("tri")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]			   
[patchconstantfunc("ConstantHS")]
[maxtessfactor(MAX_TESSELATION_FACTOR)] 
HullOut HSMain(
	InputPatch<VertexOut, 3> inputPatch, 
	uint i : SV_OutputControlPointID, 
	uint PatchID : SV_PrimitiveID)
{
	HullOut hullOut;

	hullOut.Position = inputPatch[i].Position;
	hullOut.TextureC = inputPatch[i].TextureC;

	return hullOut;
}

//////////////////////////////////////////////////////////////////////
// Domain shader
[domain("tri")] 
DomainOut DSMain(
	PatchHullOut patchTess, 
	float3 barycentricCoord : SV_DomainLocation, 
	const OutputPatch<HullOut, 3> tri)
{
	DomainOut domainOut;

	//=====================================================================
	// Calculare pozitie in functie de valuri
	const float3 worldPosition = barycentricCoord.x * tri[0].Position + 
							barycentricCoord.y * tri[1].Position + 
							barycentricCoord.z * tri[2].Position;

	//====================================================================
	// Calculam noile coordonate de textura
	domainOut.TextureC = barycentricCoord.x * tri[0].TextureC +
                         barycentricCoord.y * tri[1].TextureC +
                         barycentricCoord.z * tri[2].TextureC;

	//=====================================================================
	// Calculam normala, tangenta, bitangenta si TBN matrix-ul
	float3 tangent, bitangent, transformedWorldPos;

	if (passCB.renderMode & RenderMode::BigWaves)
	{
		transformedWorldPos = CalculateWavePosition(worldPosition);
		CalculateWaveTangentAndBitangent(tangent, bitangent, transformedWorldPos);
	}
	else
	{
		transformedWorldPos = worldPosition;
    	bitangent = float3(1, 0, 0);
    	tangent = float3(0, 0, 1);
	}
	
	domainOut.Normal = normalize(cross(tangent, bitangent));
	domainOut.TBNMatrix = float3x3(tangent, bitangent, domainOut.Normal);

	//====================================================================
	// Calculam displacement-ul pe baza texturii
 	float3 displacedWorldPosition;
	if (passCB.renderMode & RenderMode::SmallWaves)
	{
		const float4 displacement = waterDisp.SampleLevel(gsamLinearWrap,  domainOut.TextureC, 0);
		displacedWorldPosition = transformedWorldPos + domainOut.Normal * displacement.r * 4;
	}
	else
	{
		displacedWorldPosition = transformedWorldPos;
	}

	domainOut.Position = mul(float4(displacedWorldPosition, 1.0f), passCB.viewProjMatrix);
	domainOut.WorldPosition = displacedWorldPosition;	

    //=====================================================================
    // Calculare Depth
    domainOut.Depth = mul(float4(domainOut.WorldPosition, 1.f), passCB.viewMatrix).z;

	return domainOut;
}

float3 CorrectSampleDirection(in float3 worldPostion, in float3 D)
{
	const float3 F = passCB.eyePosition - waterCB.cubeMapCenter;
	const float3 G = dot(F, F) - pow(waterCB.cubeMapSphereRadius, 2);
	return passCB.eyePosition + (dot(D, F) + sqrt(pow(dot(D,F), 2.f) - G)) * D;
}

/////////////////////////////////////////////////////////////////////
// Pixel shader 
float4 PSMain(DomainOut input) : SV_TARGET
{
	//===================================================================================
    // Calculam normala - o preluam din normal map si dupa o transformam in sptiul lume cu TBN
	float3 worldNormal;
	if (passCB.renderMode & RenderMode::SmallWaves)
	{
		float3 normalFromMap = waterNormal.Sample(gsamPointWrap, input.TextureC).xyz;
		const float3 localNormal = normalFromMap * 2.f - 1.f;
		worldNormal = normalize(mul(localNormal, input.TBNMatrix));
	}
	else
	{
		worldNormal = normalize(mul(float3(0,1,0), input.TBNMatrix));
	}

	//========================================================================
	// Calculam vectorii de reflexie si refractie  si dupa aplicam corectiile geometrice
	float3 dir = normalize(input.WorldPosition - passCB.eyePosition);

	const float3 reflectionRay = CorrectSampleDirection(input.WorldPosition, -reflect(dir, worldNormal));
	const float3 refractionRay 
		= CorrectSampleDirection(input.WorldPosition, refract(dir, worldNormal, 1 / materialCB.IndexOfRefraction));

	//========================================================================
	// Calculam reflectia si refractia
	float3 reflectionColor = environmentalTexture.Sample(gsamLinearWrap, reflectionRay).xyz;
	float3 refractionColor = environmentalTexture.Sample(gsamLinearWrap, refractionRay).xyz;

    return CalculateWaterFinalColor(input.WorldPosition, worldNormal, waterCB.waterColor.xyz, reflectionColor, refractionColor);
}


#include "RayTracingHelper.hlsl"

//////////////////////////////////////////////////////////////////////////
// RayGen shader
/////////////////////////////////////////////////////////////////////////
[shader("raygeneration")] 
void rayGen()
{
	uint3 launchIndex = DispatchRaysIndex();
    float3 rayOrigin, rayDirection;

	GenerateCameraRay(launchIndex.xy, rayOrigin, rayDirection);
    
    Ray radianceRay;
    radianceRay.worldDirection = rayDirection;
    radianceRay.worldOrigin = rayOrigin;

	gOutput[launchIndex.xy] = TraceRadianceRay(radianceRay, 0);
}


//////////////////////////////////////////////////////////////////////////
// Miss shader
/////////////////////////////////////////////////////////////////////////
[shader("miss")] 
void DefaultMiss(inout RayPayload payload)
{
    payload.color = skyboxTexture.SampleLevel(gsamLinearWrap, WorldRayDirection(), 0);
}

[shader("miss")]
void ShadowMiss(inout ShadowRayPayload payload)
{
    payload.hit = false;
}

//////////////////////////////////////////////////////////////////////////
// Closest hit shaders
/////////////////////////////////////////////////////////////////////////
[shader("closesthit")] 
void CHS_TriangleGeometry(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attribs)
{   
    const uint indexSizeInBytes = 4;
    const uint indicesPerTriangle = 3;
    const uint triangleIndexStride = indicesPerTriangle * indexSizeInBytes;
    const uint baseIndex = PrimitiveIndex() * triangleIndexStride;

    const uint3 indices = Load3x32BitIndices(baseIndex, g_indices);

    float3 barycentrics =
            float3(1.0 - attribs.barycentrics.x - attribs.barycentrics.y, attribs.barycentrics.x, attribs.barycentrics.y);

    Vertex input;
    InterpolateVertexAttributes(barycentrics, g_vertices[indices[0]], g_vertices[indices[1]], g_vertices[indices[2]], input);
    input.Position = HitWorldPosition();
    const float3 bitangent = normalize(cross(input.Tangent, input.Normal));
    const float3x3 TBNMatrix = float3x3(input.Tangent, bitangent, input.Normal); 

    input.TextureC = mul(float4(input.TextureC, 0.0f, 1.0f), objectCB.textureTransform).xy;
 
    int textureIndex = 0;
    float lerpFactor = 0.0f;

    float heights[5] = { -10, 0, 15, 20, 300 };
    
    heights[2] += 15 * NoiseFunction(input.Position.x, input.Position.z);
    heights[3] += 40 * NoiseFunction(input.Position.x, input.Position.z);

    for (int i = 0; i < 4; ++i)
    {
        if (input.Position.y >= heights[i] && input.Position.y < heights[i + 1])
        {
            float range = heights[i + 1] - heights[i];
            lerpFactor = (input.Position.y - heights[i]) / range;
            
            textureIndex = i;

            break;
        }
    }

    const float4 texDiffuseAlbedo1 
        = terrainDiffuse.SampleLevel(gsamAnisotropicWrap, float3(input.TextureC, textureIndex), 0);
    const float4 texDiffuseAlbedo2 
        = terrainDiffuse.SampleLevel(gsamAnisotropicWrap, float3(input.TextureC, textureIndex + 1), 0);
    
    const float4 texNormal1 
        = terrainNormal.SampleLevel(gsamPointWrap, float3(input.TextureC, textureIndex), 0);
    const float4 texNormal2 
        = terrainNormal.SampleLevel(gsamPointWrap, float3(input.TextureC, textureIndex + 1), 0);
    
    const float3 localNormal = lerp(texNormal1, texNormal2, lerpFactor).xyz * 2.f - 1.f;
    const float3 worldNormal = normalize(mul(localNormal, TBNMatrix));

    float3 Kd = lerp(texDiffuseAlbedo1, texDiffuseAlbedo2, lerpFactor).xyz;
    
    payload.color = CalculateTerrainFinalColor(input.Position, worldNormal, Kd, payload.rescursionDepth);
}

[shader("closesthit")] 
void CHS_AABBGeometry(inout RayPayload payload, in ProceduralPrimitiveAttributes attribs)
{
    Vertex input;
    input.Position = HitWorldPosition();
    input.TextureC = (input.Position.xz + float2(200, 200)) / float2(400, 400);
    float3 bitangent;

	if (passCB.renderMode & RenderMode::BigWaves)
	{
        CalculateWaveTangentAndBitangent(input.Tangent, bitangent, input.Position);
	}
    else
    {
    	bitangent = float3(1, 0, 0);
    	input.Tangent = float3(0, 0, 1);
    }

    float3 normal = normalize(cross(input.Tangent, bitangent));
    input.Normal = normal;
    const float3x3 TBNMatrix = float3x3(input.Tangent, bitangent, input.Normal); 

    input.TextureC = mul(float4(input.TextureC, 0.0f, 1.0f), objectCB.textureTransform).xy;

    const float4 texNormal = waterNormal.SampleLevel(gsamAnisotropicWrap, input.TextureC, 0);

    float3 localNormal = texNormal.xyz * 2.f - 1.f;
    //localNormal = float3(0,-1,0);
    float3 worldNormal = normalize(mul(localNormal, TBNMatrix));
    //worldNormal = float3(0,1,0);

    float3 Kd = waterCB.waterColor.xyz;

    float4 reflexionColor = float4(0, 0, 0, 0);
    if (materialCB.Reflectivity > 0.01f)
    {
        Ray reflexionRay;
        float3 reflexionDirection = reflect(WorldRayDirection(), worldNormal);

        reflexionRay.worldDirection = reflexionDirection;
        reflexionRay.worldOrigin = input.Position;

        reflexionColor = TraceRadianceRay(reflexionRay, payload.rescursionDepth);
    }

    float4 refractionColor = float4(0, 0, 0, 0);
    if (materialCB.Transparency > 0.01)
    {
        Ray refractionRay;
        float3 refractionDirection = refract(WorldRayDirection(), worldNormal, 1 / materialCB.IndexOfRefraction);

        refractionRay.worldDirection = refractionDirection;
        refractionRay.worldOrigin = input.Position;

        refractionColor = TraceRadianceRay(refractionRay, payload.rescursionDepth);
    }

    payload.color = CalculateWaterFinalColor(input.Position, worldNormal, Kd, reflexionColor.xyz, refractionColor.xyz, payload.rescursionDepth);
    //payload.color = float4(attribs.normal, 1);
}

//////////////////////////////////////////////////////////////////////////
// Intersection shaders
/////////////////////////////////////////////////////////////////////////
[shader("intersection")]
void IntersectionShader()
{
    ProceduralPrimitiveAttributes attribs;
    //attribs.normal = float3(1,0,0);

    float3 rayOrigin = WorldRayOrigin();
    float3 rayDirection = WorldRayDirection();
    float tInitial = -rayOrigin.y / rayDirection.y;

	if (!(passCB.renderMode & RenderMode::BigWaves))
	{
        ReportHit(tInitial, 0, attribs);
        return;
	}

    float3 initialIntersect = rayOrigin + tInitial * rayDirection;

    attribs.normal = float3((initialIntersect.xz + 200) / 400, 0);

    float3 displacedPosition = CalculateWavePosition(initialIntersect);

    float tCorrected = tInitial + (displacedPosition.y - initialIntersect.y) / rayDirection.y;
    if (abs(rayOrigin.y + tCorrected * rayDirection.y - displacedPosition.y) < 0.01)
    {
        ReportHit(tCorrected, 0, attribs);
    }
}

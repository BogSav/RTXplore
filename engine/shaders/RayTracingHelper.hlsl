#pragma once

#include "LightingUtils.hlsl"

/////////////////////////////////////////////////////////////////////////////
// Structuri si define-uri
////////////////////////////////////////////////////////////////////////////
struct Vertex
{
    float3 Position;
    float4 Color;
    float3 Normal;
    float3 Tangent;
    float2 TextureC;
};

struct RayPayload
{
    float4 color;
    uint rescursionDepth;
};

struct ShadowRayPayload
{
    bool hit;
};

struct ProceduralPrimitiveAttributes
{
    XMFLOAT3 normal;
};

struct Ray
{
    float3 worldOrigin;
    float3 worldDirection;
};

//////////////////////////////////////////////////////////////////
// Shader resorurces
RaytracingAccelerationStructure gRtScene : register(t0, space0);

StructuredBuffer<Vertex> g_vertices : register(t1, space0);
ByteAddressBuffer g_indices : register(t2, space0);

Texture2DArray terrainDiffuse : register(t0, space1);
Texture2DArray terrainNormal : register(t1, space1);
Texture2D waterNormal : register(t2, space1);

RWTexture2D<float4> gOutput : register(u0);

SamplerState gsamPointWrap : register(s0);
SamplerState gsamPointClamp : register(s1);
SamplerState gsamLinearWrap : register(s2);
SamplerState gsamLinearClamp : register(s3);
SamplerState gsamAnisotropicWrap : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);

TextureCube skyboxTexture : register(t3, space1);

ConstantBuffer<WaterConstantBuffer> waterCB : register(b3, space0);

/////////////////////////////////////////////////////////////////////////////
// Functii utilitare
////////////////////////////////////////////////////////////////////////////
// Functie de calcul a poizitie valurilor
float3 CalculateWavePosition(float3 pos)
{
    float x = pos.x;
    float y = pos.y;
    float z = pos.z;

    for (int i = 0; i < WAVE_PROPERTIES_COUNT; i++)
    {
        const float w = 2.f / waterCB.waveParameters[i].wavelength;

        x += (waterCB.waveParameters[i].steepness * waterCB.waveParameters[i].amplitude * waterCB.waveParameters[i].direction.x *
			cos(w * dot(waterCB.waveParameters[i].direction, pos.xz) + waterCB.waveParameters[i].speed * passCB.totalTime));

        y += (waterCB.waveParameters[i].amplitude *
			sin(w * dot(waterCB.waveParameters[i].direction, pos.xz) + waterCB.waveParameters[i].speed * passCB.totalTime));
        
        z += (waterCB.waveParameters[i].steepness * waterCB.waveParameters[i].amplitude * waterCB.waveParameters[i].direction.y *
			cos(w * dot(waterCB.waveParameters[i].direction, pos.xz) + waterCB.waveParameters[i].speed * passCB.totalTime));
    }

    return float3(x, y, z);
}

// Functie de calcul a bitangentei si tangentei valurilor
void CalculateWaveTangentAndBitangent(out float3 tangent, out float3 bitangent, in float3 pos)
{
    bitangent = float3(1, 0, 0);
    tangent = float3(0, 0, 1);

    for (int i = 0; i < WAVE_PROPERTIES_COUNT; i++)
    {
        const float w = 2.f / waterCB.waveParameters[i].wavelength;
        const float WA = w * waterCB.waveParameters[i].amplitude;

        const float S = sin(w * dot(waterCB.waveParameters[i].direction, pos.xz) + waterCB.waveParameters[i].speed * passCB.totalTime);
        const float C = cos(w * dot(waterCB.waveParameters[i].direction, pos.xz) + waterCB.waveParameters[i].speed * passCB.totalTime);

		// Calcul bitangenta
        bitangent.x 
			-= (waterCB.waveParameters[i].steepness * pow(waterCB.waveParameters[i].direction.x, 2) * WA * S);
        bitangent.y 
			+= (waterCB.waveParameters[i].direction.x * WA * C);
        bitangent.z 
			-= (waterCB.waveParameters[i].steepness * waterCB.waveParameters[i].direction.x * waterCB.waveParameters[i].direction.y * WA * S);

		// Calcul tangenta
        tangent.y 
			+= (waterCB.waveParameters[i].direction.y * WA * C);
        tangent.z 
			-= (waterCB.waveParameters[i].steepness * pow(waterCB.waveParameters[i].direction.y, 2) * WA * S);

    }

    tangent.x = bitangent.z;
}

inline void GenerateCameraRay(uint2 index, out float3 origin, out float3 direction)
{
    float2 xy = index + 0.5f;
    float2 screenPos = xy / DispatchRaysDimensions().xy * 2.0 - 1.0;

    screenPos.y = -screenPos.y;

    float4 world = mul(float4(screenPos, 0, 1), passCB.invViewProjMatrix);

    world.xyz /= world.w;
    origin = passCB.eyePosition.xyz;
    direction = normalize(world.xyz - origin);
}

uint3 Load3x32BitIndices(uint offsetBytes, ByteAddressBuffer Indices)
{
    uint3 indices;

    // ByteAdressBuffer loads must be aligned at a 4 byte boundary.
    // Since each index is 32 bits, we need to read 12 bytes (3 indices x 4 bytes each),
    // but we will load 16 bytes (4 indices) to handle alignment safely.
    const uint dwordAlignedOffset = offsetBytes & ~3;
    const uint4 four32BitIndices = Indices.Load4(dwordAlignedOffset);

    // Calculate the position of the first index within our loaded range
    uint indexOffset = (offsetBytes - dwordAlignedOffset) >> 2;

    // Retrieve the three 32bit indices
    indices.x = four32BitIndices[indexOffset % 4];
    indices.y = four32BitIndices[(indexOffset + 1) % 4];
    indices.z = four32BitIndices[(indexOffset + 2) % 4];

    return indices;
}

void InterpolateVertexAttributes(
    float3 barycentrics,
    in Vertex v1, 
    in Vertex v2,
    in Vertex v3,
    out Vertex interpolatedVertex)
{
    //interpolatedVertex.Position = v1.Position * barycentrics.x + v2.Position * barycentrics.y + v3.Position * barycentrics.z;
    interpolatedVertex.Normal = v1.Normal * barycentrics.x + v2.Normal * barycentrics.y + v3.Normal * barycentrics.z;
    interpolatedVertex.TextureC = v1.TextureC * barycentrics.x + v2.TextureC * barycentrics.y + v3.TextureC * barycentrics.z;
    interpolatedVertex.Tangent = v1.Tangent * barycentrics.x + v2.Tangent * barycentrics.y + v3.Tangent * barycentrics.z;
}

float4 TraceRadianceRay(in Ray ray, in uint currentRayRecursionDepth)
{
    if (currentRayRecursionDepth >= MAX_RAY_RECURSION_DEPTH)
    {
        return float4(0, 0, 0, 0);
    }

    RayDesc rayDesc;
    rayDesc.Origin = ray.worldOrigin;
    rayDesc.Direction = ray.worldDirection;
    rayDesc.TMin = 0.4;
    rayDesc.TMax = 10000;

    RayPayload rayPayload = { float4(0, 0, 0, 0), currentRayRecursionDepth + 1 };

    TraceRay(gRtScene, RAY_FLAG_CULL_BACK_FACING_TRIANGLES, 0xFF, 0, 0, 0, rayDesc, rayPayload);

    return rayPayload.color;
}

bool TraceShadowRayAndReportIfHit(in Ray ray, in uint currentRayRecursionDepth)
{
    if (currentRayRecursionDepth >= MAX_RAY_RECURSION_DEPTH)
    {
        return false;
    }

    // Set the ray's extents.
    RayDesc rayDesc;
    rayDesc.Origin = ray.worldOrigin;
    rayDesc.Direction = ray.worldDirection;
    rayDesc.TMin = 0.1;
    rayDesc.TMax = 80;

    ShadowRayPayload shadowPayload = { true };
    TraceRay(gRtScene,
        RAY_FLAG_CULL_BACK_FACING_TRIANGLES
        | RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH
        | RAY_FLAG_FORCE_OPAQUE             // ~skip any hit shaders
        | RAY_FLAG_SKIP_CLOSEST_HIT_SHADER, // ~skip closest hit shaders,
        1, 0, 0, 1,
        rayDesc, shadowPayload);

    return shadowPayload.hit;
}

float3 HitWorldPosition()
{
    return WorldRayOrigin() + RayTCurrent() * WorldRayDirection();
}

float4 CalculateTerrainFinalColor(float3 P, float3 N, float3 Kd, UINT currentRecursion)
{
    const float3 v = -WorldRayDirection();

    bool inShadow = false;
    {
        Ray shadowRay;
        shadowRay.worldDirection = passCB.lightSources[0].Direction;
        shadowRay.worldOrigin = P;

        inShadow = TraceShadowRayAndReportIfHit(shadowRay, currentRecursion);
    }

    const float3 colorFromLightSources = ComputeLightingFromSources(P, N, v, Kd) * (inShadow ? inShadowRadiance : 1.f);

    return float4(colorFromLightSources, 1.f);
}

float4 CalculateWaterFinalColor(float3 P, float3 N, float3 Kd, float3 reflexionColor, float3 refractionColor, UINT currentRecursion)
{
    const float3 v = -WorldRayDirection();
    
    // Calcul lumina de la sursele de lumina si umbre
    bool inShadow = false;
    {
        Ray shadowRay;
        shadowRay.worldDirection = passCB.lightSources[0].Direction;
        shadowRay.worldOrigin = P;

        inShadow = TraceShadowRayAndReportIfHit(shadowRay, currentRecursion);
    }

    const float3 colorFromLightSources = ComputeLightingFromSources(P, N, v, Kd) * (inShadow ? inShadowRadiance : 1.f);

    //return float4(refractionColor, 1);
    //return float4(lerp(colorFromLightSources, reflexionColor, 0.5), 1);

    // Calcule pentru reflectii si refractii
    const float3 fresnelReflectivity =  ShlickFresnel(materialCB.Rf0, N, v);
    
    return float4(colorFromLightSources * (1 - materialCB.Transparency)
        + lerp(refractionColor * materialCB.Transparency, reflexionColor * materialCB.Reflectivity, fresnelReflectivity), 1);
}
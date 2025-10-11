#pragma once

#include "LightingUtils.hlsl"

//////////////////////////////////////////////////////////////////
// Shader resorurces
ConstantBuffer<WaterConstantBuffer> waterCB : register(b0, space1);

SamplerState gsamPointWrap : register(s0);
SamplerState gsamPointClamp : register(s1);
SamplerState gsamLinearWrap : register(s2);
SamplerState gsamLinearClamp : register(s3);
SamplerState gsamAnisotropicWrap : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);

SamplerComparisonState gsamShadow : register(s6);

Texture2DArray terrainDiffuse : register(t0, space0);
Texture2DArray terrainNormal : register(t1, space0);
Texture2D waterNormal : register(t2, space0);
TextureCube skyBox : register(t3, space0);

Texture2DArray terrainDisp : register(t0, space1);
Texture2D waterDisp : register(t1, space1);

TextureCube environmentalTexture : register(t0, space2);
Texture2D shadowTexture : register(t1, space2);

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

// Functie de calcul a factorului de umbrire aka shadow factor
float CalcShadowFactor(float4 shadowPosH)
{
    // Complete projection by doing division by w.
    shadowPosH.xyz /= shadowPosH.w;
    shadowPosH.xy = shadowPosH.xy * 0.5f + 0.5f;
    shadowPosH.y = 1.f - shadowPosH.y;
    
    // Depth in NDC space.
    float depth = shadowPosH.z;
    uint width, height, numMips;
    shadowTexture.GetDimensions(0, width, height, numMips);
    
    // Texel size.
    float dx = 1.0f / (float) width;
    float percentLit = 0.0f;

    const float2 offsets[9] =
    {
        float2(-dx, -dx), float2(0.0f, -dx), float2(dx, -dx),
        float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
        float2(-dx, +dx), float2(0.0f, +dx), float2(dx, +dx)
    };

    [unroll]
    for (int i = 0; i < 9; ++i)
    {
        percentLit += shadowTexture.SampleCmpLevelZero(gsamShadow, shadowPosH.xy + offsets[i], depth).r;
    }
    
    return percentLit / 9.0f;
}

float4 CalculateTerrainFinalColor(float3 P, float3 N, float3 Kd, float depth)
{
    const float3 v = normalize(passCB.eyePosition - P);
    
    const float shadowFactor = CalcShadowFactor(mul(float4(P, 1), passCB.lightSources[0].lightTransformMatrix));

    const float3 colorFromLightSources = ComputeLightingFromSources(P, N, v, Kd) * shadowFactor;

    return float4(colorFromLightSources, 1);

    const float fogFactor = ComputeFogFactor(passCB.fogStart, passCB.farZ, depth);
}

float4 CalculateWaterFinalColor(float3 P, float3 N, float3 Kd, float3 reflexionColor, float3 refractionColor)
{
    const float3 v = normalize(passCB.eyePosition - P);
    
    const float shadowFactor = CalcShadowFactor(mul(float4(P, 1), passCB.lightSources[0].lightTransformMatrix));

    const float3 colorFromLightSources = ComputeLightingFromSources(P, N, v, Kd) * shadowFactor;

    //return float4(refractionColor, 1);

    // Calcule pentru reflectii si refractii
    const float3 fresnelReflectivity =  ShlickFresnel(materialCB.Rf0, N, v);
    
    return float4(colorFromLightSources * (1 - materialCB.Transparency)
        + lerp(refractionColor * materialCB.Transparency, reflexionColor * materialCB.Reflectivity, fresnelReflectivity), 1.);
}
#pragma once

#ifndef HLSL
#define HLSL
#endif

#include "../gfx/include/engine/gfx/HlslUtils.h"

//////////////////////////////////////////////////////////////////
// Shader resorurces common to RT and RAST
ConstantBuffer<PassConstantBuffer> passCB : register(b0, space0);
ConstantBuffer<ObjectConstantBuffer> objectCB : register(b1, space0);
ConstantBuffer<MaterialProperties> materialCB : register(b2, space0);



//////////////////////////////////////////////////////////////////
// Helper functions for lighting calculation

// Distance based attenuation function used both for point and spot light
float CalculateAttenuation(in LightProperties light, in float distance)
{
    const float attenuation = 1.0f
		/ (light.Kc + distance * light.Kl + light.Kq * pow(distance, 2));

    return saturate(attenuation);
}

// Distance based spot attenuation function used for spot lights
float CalculateSpotAttenuation(in LightProperties light, in float distance, in float3 L)
{
    float spotEffect = dot(-L, light.Direction);
    float spotLimit = cos(light.SpotAngle * 0.5f);
   
    if (spotEffect < spotLimit)
        return 0.0f;
    
    float attenuation = (spotEffect - spotLimit) / (1.0f - spotLimit);
    
    return pow(attenuation, 2);
}

// Shlick approximation of the Fesnel equations
float3 ShlickFresnel(in float3 Rf0, in float cosi)
{
    return Rf0 + (1.0f - Rf0) * pow((1.0f - cosi), 5);
}
float3 ShlickFresnel(in float3 Rf0, in float3 N, in float3 L)
{
    return ShlickFresnel(Rf0, dot(N, L));
}
void fresnel(in float3 I, in float3 N, in float ior, out float kr)
{
    float cosi = clamp(-1, 1, dot(I, N));
    float etai = 1, etat = ior;
    if (cosi > 0) 
    { 
        float aux  = etai;
        etai = etat;
        etat = aux;    
    }
    // Compute sini using Snell's law
    float sint = etai / etat * sqrt(max(0.f, 1 - cosi * cosi));
    // Total internal reflection
    if (sint >= 1) {
        kr = 1;
    }
    else {
        float cost = sqrt(max(0.f, 1 - sint * sint));
        cosi = abs(cosi);
        float Rs = ((etat * cosi) - (etai * cost)) / ((etat * cosi) + (etai * cost));
        float Rp = ((etai * cosi) - (etat * cost)) / ((etai * cosi) + (etat * cost));
        kr = (Rs * Rs + Rp * Rp) / 2;
    }
    // As a consequence of the conservation of energy, the transmittance is given by:
    // kt = 1 - kr;
}

// Roughness factor function used for the micro facet model 
float3 RoughnessDistributionFunction(in float m, in float3 N, in float3 H)
{
    return pow(dot(N, H), m);
}
float3 NormalizedRoughnessDistributionFunction(in float m, in float3 N, in float3 H)
{
    return (m + 8) / 8 * RoughnessDistributionFunction(m, N, H);
}

// BlinnPhong lighting model calculation fucntion
float3 CalculateBlinnPhongLighting(
    in float3 Bl, in float3 N, in float3 v, in float3 L, in float3 P, in float3 Kd)
{
    float3 receivedLight = max(dot(L, N), 0.0f) * Bl;

    float3 ca = passCB.ambientLight.xyz * materialCB.Ka;
    float3 cd = receivedLight * Kd;
    float3 cs = float3(0, 0, 0);

    float3 H = normalize(L + v);
    if (dot(L, H) > 0)
    {
        const float3 fresnelSpecular = ShlickFresnel(materialCB.Rf0, dot(L, H));
        const float3 roughnessFactor = RoughnessDistributionFunction(materialCB.Shiness * 256.0f, N, H);

        cs = receivedLight * fresnelSpecular * materialCB.Ks;
    }

    return ca + cd + cs;
}

// Point light calculation
float3 ComputePointLight(
    in LightProperties light, in float3 P, in float3 N, in float3 v, in float3 Kd)
{
    const float3 L = light.Position - P;
    const float distanceFromCamera = length(L);
    
    const float attenuation = CalculateAttenuation(light, distanceFromCamera);
    
    return CalculateBlinnPhongLighting(light.Strength, N, v, normalize(L), P, Kd) * attenuation;
}

// Spot light calculation
float3 ComputeSpotLight(
    in LightProperties light, in float3 P, in float3 N, in float3 v, in float3 Kd)
{
    float3 L = light.Position - P;
    const float distance = length(v);
    L = normalize(L);
    
    const float attenuation = CalculateAttenuation(light, distance);
    const float spotAttenuation = CalculateSpotAttenuation(light, distance, L);
    
    return CalculateBlinnPhongLighting(light.Strength, N, v, L, P, Kd) * attenuation * spotAttenuation;
}

// Directional light calculation
float3 ComputeDirectionalLight(
    in LightProperties light, in float3 P, in float3 N, in float3 v, in float3 Kd)
{   
    return CalculateBlinnPhongLighting(light.Strength, N, v, -light.Direction, P, Kd);
}

// Functie de calcul a luminii provenita de la surse
float3 ComputeLightingFromSources(in float3 P, in float3 N, in float3 v, in float3 Kd)
{
    float3 finalLigth = float3(0, 0, 0);

    [unroll]
    for (int i = 0; i < DIR_LIGHTS_COUNT; i++)
    {
        finalLigth += ComputeDirectionalLight(passCB.lightSources[i], P, N, v, Kd);
    }
    
    [unroll]
    for (int i = DIR_LIGHTS_COUNT; i < DIR_LIGHTS_COUNT + POINT_LIGHTS_COUNT; i++)
    {
        finalLigth += ComputePointLight(passCB.lightSources[i], P, N, v, Kd);
    }
    
    [unroll]
    for (int i = DIR_LIGHTS_COUNT + POINT_LIGHTS_COUNT; i < MAX_NUMBER_OF_LIGHTS; i++)
    {
        finalLigth += ComputeSpotLight(passCB.lightSources[i], P, N, v, Kd);
    }

    return finalLigth;
}

float ComputeFogFactor(float startFogDistance, float endFogDistance, float distance)
{   
    return smoothstep(startFogDistance, endFogDistance, distance);
}

float NoiseFunction(float x, float z)
{
    return sin(x * 0.1) * cos(z * 0.1) * 0.5 + 0.5;
}
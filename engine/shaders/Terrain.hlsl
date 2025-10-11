#include "RasterizationHelper.hlsl"

struct VertexIn
{
    float3 Position : Position;
    float3 Normal : Normal;
    float3 Tangent : Tangent;
    float2 TextureC : TextureC;
};

struct VertexOut
{
    float4 Position : SV_POSITION;
    float3 Normal : Normal;
    float3 Tangent : Tangent;
    float2 TextureC : TextureC;
};

struct HullOut
{
    float4 Position : SV_POSITION;
    float3 Normal : Normal;
    float3 Tangent : Tangent;
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
    float2 TextureC : TextureC;
    float Depth : Depth;
};

//////////////////////////////////////////////////////////////////////////
// Vertex shader normal
VertexOut VSMain(VertexIn input)
{
    VertexOut vertexOut;
    
    vertexOut.Position = float4(input.Position, 1.f);
    vertexOut.Normal = input.Normal;
    vertexOut.Tangent = input.Tangent;
    vertexOut.TextureC = mul(float4(input.TextureC, 0.0f, 1.0f), objectCB.textureTransform).xy;
    
    return vertexOut;
}

//////////////////////////////////////////////////////////////////////////
// Vertex shader cube map
DomainOut VSForCubeMapRendering(VertexIn input)
{
    DomainOut pixelInput;

    const float3 biTangent = normalize(cross(input.Tangent, input.Normal));
    pixelInput.TBNMatrix = float3x3(input.Tangent, biTangent, input.Normal);

    pixelInput.WorldPosition = input.Position;
    pixelInput.Position = mul(float4(input.Position, 1.f), passCB.viewProjMatrix);
    pixelInput.Depth = mul(float4(input.Position, 1.f), passCB.viewMatrix).z;
    pixelInput.TextureC = mul(float4(input.TextureC, 0.0f, 1.0f), objectCB.textureTransform).xy;

    return pixelInput;
}

//////////////////////////////////////////////////////////////////////////
// Hull shader 
PatchHullOut ConstantHS(
    InputPatch<VertexOut, 3> patch,
    uint patchId : SV_PrimitiveID)
{
    PatchHullOut output;
    
    float3 center = (patch[0].Position + patch[1].Position + patch[2].Position).xyz / 3.f;
    
    const float distanceFromCamera = distance(center, passCB.eyePosition);
    
    const float d0 = passCB.nearZ;
    const float d1 = 20.f;
    
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
HullOut HSMain(InputPatch<VertexOut, 3> inputPatch,
    uint i : SV_OutputControlPointID,
    uint PatchID : SV_PrimitiveID)
{
    HullOut hullOut;
    
    hullOut.Position = inputPatch[i].Position;
    hullOut.Normal = inputPatch[i].Normal;
    hullOut.TextureC = inputPatch[i].TextureC;
    hullOut.Tangent = inputPatch[i].Tangent;
    
    return hullOut;
}

//////////////////////////////////////////////////////////////////////////
// Domain shader

[domain("tri")]
DomainOut DSMain(
    PatchHullOut patchTess,
    float3 barycentricCoord : SV_DomainLocation,
    const OutputPatch<HullOut, 3> tri)
{
    DomainOut domainOut;

    //=====================================================================
    // Calculare coordonate de textura
    domainOut.TextureC = barycentricCoord.x * tri[0].TextureC +
                         barycentricCoord.y * tri[1].TextureC +
                         barycentricCoord.z * tri[2].TextureC;

    //=====================================================================
    // Calculare matrice TBN
    const float3 normal = normalize(barycentricCoord.x * tri[0].Normal +
                              barycentricCoord.y * tri[1].Normal +
                              barycentricCoord.z * tri[2].Normal);
    const float3 tangent = normalize(barycentricCoord.x * tri[0].Tangent +
                               barycentricCoord.y * tri[1].Tangent +
                               barycentricCoord.z * tri[2].Tangent);
    const float3 biTangent = normalize(cross(tangent, normal));

    domainOut.TBNMatrix = float3x3(tangent, biTangent, normal);

    //=====================================================================
    // Calculare pozitie lume si pozitie in spatiu proiectie
    const float4 worldPosition = barycentricCoord.x * tri[0].Position +
                           barycentricCoord.y * tri[1].Position +
                           barycentricCoord.z * tri[2].Position;
    
    float heights[5] = { -10, 0, 15, 20, 300 };
    heights[2] += 15 * NoiseFunction(worldPosition.x, worldPosition.z);
    heights[3] += 40 * NoiseFunction(worldPosition.x, worldPosition.z);
    int textureIndex = 0;
    for (int i = 0; i < 4; ++i)
    {
        if (worldPosition.y >= heights[i] && worldPosition.y < heights[i + 1])
        {
            textureIndex = i;
            break;
        }
    }
    float4 displacement = terrainDisp.SampleLevel(gsamPointWrap, float3(domainOut.TextureC, textureIndex), 0);
    domainOut.WorldPosition = worldPosition.xyz + normal * displacement.r - normal;

    
    //=====================================================================
    // Calculare pozitie finala projection space
    domainOut.Position = mul(float4(domainOut.WorldPosition, 1), passCB.viewProjMatrix);
    
    
    //=====================================================================
    // Calculare Depth
    domainOut.Depth = mul(float4(domainOut.WorldPosition, 1.f), passCB.viewMatrix).z;

    return domainOut;
}

//////////////////////////////////////////////////////////////////////////
// Pixel shader

float4 ComputeHeightColor(in float3 worldPosition)
{
    // Extragerea altitudinii (y-ul poziției în lume)
    float height = worldPosition.y;

    // Normalizarea altitudinii între 0 și 1 pentru intervalul dat (-30 la 30)
    float normalizedHeight = (height + 30) / 60;

    // Culori în funcție de altitudine
    float3 color;

    if (height < -10) {
        // Albastru pentru subacvatic
        color = lerp(float3(0.0, 0.0, 0.5), float3(0.0, 0.5, 1.0), normalizedHeight);
    } else if (height < 15) {
        // Verde pentru nivelul de bază
        color = lerp(float3(0.0, 0.5, 0.0), float3(0.5, 1.0, 0.5), normalizedHeight);
    } else if (height < 45) {
        // Maro pentru altitudine înaltă
        color = lerp(float3(0.4, 0.2, 0.1), float3(0.6, 0.4, 0.2), normalizedHeight);
    } else {
        // Alb pentru vârfuri
        color = float3(1.0, 0, 0);
    }

    // Returnarea culorii finale
    return float4(color, 1.0);
}


float4 PSMain(DomainOut input) : SV_TARGET
{
    //return diffuseTextures.Sample(gsamAnisotropicWrap, float3(input.TextureC, 1));
    //return float4(input.TBNMatrix[2], 1);
    //return ComputeHeightColor(input.WorldPosition);

    //===================================================================================
    // Calculam culoarea difuza a materialului in fucntie de inaltime
    int textureIndex = 0;
    float lerpFactor = 0.0f;

    float heights[5] = { -10, 0, 15, 20, 300 };
    heights[2] += 15 * NoiseFunction(input.WorldPosition.x, input.WorldPosition.z);
    heights[3] += 40 * NoiseFunction(input.WorldPosition.x, input.WorldPosition.z);

    for (int i = 0; i < 4; ++i)
    {
        if (input.WorldPosition.y >= heights[i] && input.WorldPosition.y < heights[i + 1])
        {
            float range = heights[i + 1] - heights[i];
            lerpFactor = (input.WorldPosition.y - heights[i]) / range;
            
            textureIndex = i;

            break;
        }
    }

    const float4 texDisp1 = terrainDiffuse.Sample(gsamAnisotropicWrap, float3(input.TextureC, textureIndex));
    const float4 texDisp2 = terrainDiffuse.Sample(gsamAnisotropicWrap, float3(input.TextureC, textureIndex + 1));
    float3 Kd = lerp(texDisp1, texDisp2, lerpFactor).xyz;
    
    
    //===================================================================================
    // Calculam normala - o preluam din normal map si dupa o transformam in sptiul lume cu TBN
    float4 texNormal1 = terrainNormal.Sample(gsamPointWrap, float3(input.TextureC, textureIndex));
    float4 texNormal2 = terrainNormal.Sample(gsamPointWrap, float3(input.TextureC, textureIndex + 1));
    const float3 localNormal = lerp(texNormal1, texNormal2, lerpFactor).xyz * 2.f - 1.f;
    const float3 worldNormal = normalize(mul(localNormal, input.TBNMatrix));
    
    //===================================================================================
    // Calculam iluminare, umbrire, ceata
    return CalculateTerrainFinalColor(input.WorldPosition, worldNormal, Kd, input.Depth);
}

#include "RasterizationHelper.hlsl"

struct VertexIn
{
    float3 Position : Position;
    float4 Color : Color;
    float3 Normal : Normal;
    float3 Tangent : Tangent;
    float2 TextureC : TextureC;
};

struct VertexOut
{
    float4 Position : SV_POSITION;
    float4 Color : Color;
};

Texture2D displacemmentTexture : register(t0, space1);
Texture2D diffuseTexture : register(t2, space0);
Texture2D normalTexture : register(t3, space0);

////////////////////////////////////////////////////////////////////////////////
// Vertex shader
VertexOut VSMain(VertexIn input)
{
    float4 worldPos = mul(float4(input.Position, 1.0f), objectCB.worldMatrix);

    float3 worldNormal = mul(float4(input.Normal, 0.0f), transpose(objectCB.invWorldMatrix)).xyz;
    worldNormal = normalize(worldNormal);

    float4 color = float4(1.f, 1.f, 1.f, 1.f);
	
    switch (passCB.renderMode)
    {
        case 0:
            color = input.Color;
            break;
        case 1:
            color = float4(worldNormal, 1.0f);
            break;
        case 2:
            color = float4(1,0,0,0);
            break;
    }

    VertexOut output;
    output.Position = mul(worldPos, passCB.viewProjMatrix);
    output.Color = color;

    return output;
}

////////////////////////////////////////////////////////////////////////////////
// Pixel shader
float4 PSMain(VertexOut input) : SV_TARGET
{
    return input.Color;
}

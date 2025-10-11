#include "RasterizationHelper.hlsl"

struct VertexIn
{
    float3 Position : Position;
};

struct VertexOut
{
    float4 Position : SV_POSITION;
};

//////////////////////////////////////////////
// Vertex shader
VertexOut VSMain(VertexIn vin)
{
    VertexOut vout;

    const float4 worldPosition = mul(float4(vin.Position, 1.0f), objectCB.worldMatrix);
    vout.Position = mul(worldPosition, passCB.viewProjMatrix);
	
    return vout;
}

////////////////////////////////////////////
// Pixel shader
float4 PSMain(VertexOut pin) : SV_Target
{
    return float4(1, 0, 0, 1);
}

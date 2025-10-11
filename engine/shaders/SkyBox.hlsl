#include "RasterizationHelper.hlsl"

struct VertexIn
{
    float3 Position : Position;
    float2 TextureC : TextureC;
};

struct VertexOut
{
    float4 Position : SV_POSITION;
    float3 textureC : TextureC;
};

/////////////////////////////////////////////////////////////////////////
// Vertex Shader
VertexOut VSMain(VertexIn input)
{
    VertexOut output;

    float4x4 viewMatrixRotOnly = passCB.viewMatrix;
    viewMatrixRotOnly[3][0] = 0.0;
    viewMatrixRotOnly[3][1] = 0.0;
    viewMatrixRotOnly[3][2] = 0.0;

    const float4x4 viewProjRotOnly = mul(viewMatrixRotOnly, passCB.projMatrix);
    output.Position = mul(float4(input.Position, 1.0), viewProjRotOnly);    

    const float3 rotatedPosition = mul(float4(input.Position, 1), objectCB.textureTransform).xyz;
    output.textureC = rotatedPosition;

    return output;
}


//////////////////////////////////////////////////////////////////////////
// Pixel Shader
float4 PSMain(VertexOut input) : SV_TARGET
{
    return skyBox.Sample(gsamLinearWrap, input.textureC);
}
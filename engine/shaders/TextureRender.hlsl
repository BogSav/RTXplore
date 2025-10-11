SamplerState gsamPointWrap : register(s0);
SamplerState gsamPointClamp : register(s1);
SamplerState gsamLinearWrap : register(s2);
SamplerState gsamLinearClamp : register(s3);
SamplerState gsamAnisotropicWrap : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);

Texture2D renderTexture : register(t0, space0);

struct VertexIn
{
    float3 Position : Position;
    float2 TextureC : TextureC;
};

struct VertexOut
{
    float4 Position : SV_POSITION;
    float2 TexC : TEXCOORD;
};

//////////////////////////////////////////////
// Vertex shader
VertexOut VSMain(VertexIn vin)
{
    VertexOut vout;

    vout.Position = float4(vin.Position, 1.0f);
    vout.TexC = vin.TextureC;
	
    return vout;
}


////////////////////////////////////////////
// Pixel shader
float4 PSMain(VertexOut pin) : SV_Target
{
    //return float4(1,0,0,0);
    return float4(renderTexture.Sample(gsamLinearWrap, pin.TexC).rrr, 1.0f);
}

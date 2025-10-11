Texture2D gInputA;
Texture2D gInputB;

RWTexture2D<float4> gOutput;

[numthreads(16, 16, 1)]
void CSMain(int3 dispatchThreadID : SV_DispatchThreadID) 
{
    gOutput[dispatchThreadID.xy] =
        gInputA[dispatchThreadID.xy] +
        gInputB[dispatchThreadID.xy];
}
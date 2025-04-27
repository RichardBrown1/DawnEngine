struct Info //struct DawnEngine::InfoBufferLayout
{
    float2 texcoord;
    uint materialId;
    uint PAD0;
};

struct InputInfo
{
    uint2 dimensions;
    uint materialId;
    uint PAD0;
};

RWTexture2D accumulatorTexture : register(t0, space0);
StructuredBuffer<Info> infoBuffer : register(b1, space0);
ConstantBuffer<InputInfo> inputInfoBuffer : register(b2, space0);
Texture2D inputTexture : register(t3, space0);
SamplerState inputSamplerState : register(s4, space0);


void CS_main(uint2 dispatchThreadID: SV_DispatchThreadID)
{
    accumulatorTexture.GetDimensions(width, height); //TODO: Convert this to shader constant
    float4 result = accumulatorTexture.Load(dispatchThreadID.x, dispatchThreadID.y);
    Info info = infoBuffer.Load(width * dispatchThreadID.y + dispatchThreadID.x);
    if(info.materialId == )
    accumulatorTexture[dispatchThreadID.x, dispatchThreadID.y] += inputTexture.Sample(inputSamplerState, )
}

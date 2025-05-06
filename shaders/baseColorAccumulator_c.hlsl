#include "_definitions.hlsli"

struct TextureMasterInfo //struct DawnEngine::InfoBufferLayout
{
    float2 texcoord;
    uint materialId;
    uint PAD0;
};

struct TextureInputInfo
{
    uint2 dimensions;
    uint materialId;
    uint PAD0;
};

RWTexture2D<float4> accumulatorTexture : register(u0, space0);
StructuredBuffer<TextureMasterInfo> textureMasterBuffer : register(t1, space0);
ConstantBuffer<TextureInputInfo> textureInputInfoBuffer : register(b2, space0);
Texture2D inputTexture : register(t3, space0);
SamplerState inputSamplerState : register(s4, space0);


[numthreads(1,1,1)]
void cs_main(uint2 dispatchThreadID: SV_DispatchThreadID)
{
    uint textureWidth, textureHeight; //accumulatorTexture and inputTexture are assumed to be of the same size
    accumulatorTexture.GetDimensions(textureWidth, textureHeight);
    TextureMasterInfo info = textureMasterBuffer.Load(textureWidth * dispatchThreadID.y + dispatchThreadID.x);
    float4 inputTextureColor = inputTexture.Sample(inputSamplerState, info.texcoord);
    if (info.materialId == textureInputInfoBuffer.materialId)
    {
        accumulatorTexture[dispatchThreadID.xy] = inputTextureColor;
    }
}

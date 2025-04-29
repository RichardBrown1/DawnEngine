
struct TextureInputInfo
{
    uint samplerTexturePairId;
    uint PAD0;
    uint PAD1;
    uint PAD2;
};

RWTexture2D accumulatorTexture : register(t0, space0);
StructuredBuffer<TextureMasterInfo> textureMasterBuffer : register(t1, space0);
ConstantBuffer<TextureInputInfo> textureInputInfoBuffer : register(b2, space0);
Texture2D inputTexture : register(t3, space0);
SamplerState inputSamplerState : register(s4, space0);


void cs_main(uint2 dispatchThreadID: SV_DispatchThreadID)
{
    uint textureWidth, textureHeight; //accumulatorTexture and inputTexture are assumed to be of the same size
    accumulatorTexture.GetDimensions(textureWidth, textureHeight);
    TextureMasterInfo info = textureMasterBuffer.Load(textureWidth * dispatchThreadID.y + dispatchThreadID.x);
    float4 inputTextureColor = inputTexture.Sample(inputSamplerState, info.texcoord);
    if (info.samplerTexturePairId == textureInputInfoBuffer.samplerTexturePairId)
    {
        accumulatorTexture[dispatchThreadID.x, dispatchThreadID.y] = inputTextureColor;
    }
}

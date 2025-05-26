#include "canvas.hlsli"
Texture2D ultimateTexture : register(t0, space0);
SamplerState ultimateSampler : register(s1, space0);

float4 fs_main(CanvasOutput input) : SV_TARGET0 {
    return ultimateTexture.Sample(ultimateSampler, input.texCoord);
}
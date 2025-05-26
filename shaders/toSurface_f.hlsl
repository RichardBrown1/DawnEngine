Texture2D ultimateTexture : register(t0, space0);
SamplerState ultimateSampler : register(t1, space0);

float4 fs_main(float4 position : SV_Position) : SV_TARGET0 {
    return ultimateTexture.Sample(ultimateSampler, position.xy);
}
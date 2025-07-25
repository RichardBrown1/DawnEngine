#include "shadowMap.hlsli"

ConstantBuffer<Light> light : register(b0, space1);

float4 fs_main(VSOutput input) : SV_Target
{
    const float distance = length(light.position - input.Position);
    const float attenuation = max(min(1.0 - pow(distance / light.range, 4), 1), 0) / (distance * distance);
    const float color = 1 - attenuation;
    return float4(color, color, color, 1.0);
}


#include "definitions.hlsli"
StructuredBuffer<float4x4> transforms : register(t1, space0);
StructuredBuffer<Light> lights : register(t4, space0);

struct VSInput
{
    float3 Position : POSTION0;
    float3 Normal : NORMAL0;
};

struct VSOutput
{
    float4 ClipPosition : SV_Position;
    float3 Position : POSITION0;
};

VSOutput VS_main(VSInput input, uint VertexIndex : SV_VertexID, uint InstanceIndex : SV_InstanceID)
{
    VSOutput output = (VSOutput) 0;    
    
    output.Position = (float3) mul(transforms[InstanceIndex], float4(input.Position, 1.0));
    output.ClipPosition = mul(lights[0].lightSpaceMatrix, float4(output.Position, 1.0));
    
    return output;
}

float4 FS_main(VSOutput input) : SV_Target
{
    const Light light = lights[0];
    const float distance = length(light.position - input.Position);
    const float attenuation = max(min(1.0 - pow(distance / light.range, 4), 1), 0) / (distance * distance);
    const float color = 1 - attenuation;
    return float4(color, color, color, 1.0);
}

#include "shadowMap.hlsli"
StructuredBuffer<float4x4> transforms : register(t0, space0);
ConstantBuffer<Light> light : register(b1, space0);

struct VSInput
{
    float3 Position : POSTION0;
    float3 Normal : NORMAL0;
	float3 texcoord : TEXCOORD0;
};

VSOutput vs_main(VSInput input, uint VertexIndex : SV_VertexID, uint InstanceIndex : SV_InstanceID)
{
    VSOutput output = (VSOutput) 0;    
    
    output.Position = (float3) mul(transforms[InstanceIndex], float4(input.Position, 1.0));
    output.ClipPosition = mul(light.lightSpaceMatrix, float4(output.Position, 1.0));
    
    return output;
}

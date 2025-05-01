#include "_definitions.hlsli"
#include "_helpers.hlsli"
#include "initialRender.hlsli"

ConstantBuffer<ProjectionView> camera : register(b0, space0);
StructuredBuffer<float4x4> transforms : register(t1, space0);

struct VSInput
{
    float3 position : POSTION0;
    float3 normal : NORMAL0;
    float2 texcoord : TEXCOORD0;
};

VSOutput vs_main(VSInput input, uint vertexIndex : SV_VertexID, uint instanceIndex : SV_InstanceID)
{
    VSOutput output = (VSOutput) 0;    
    
    const float3 position = (float3) mul(transforms[instanceIndex], float4(input.position, 1.0));
    output.cameraPosition = mul(
                            camera.projectionView,
                            float4(position, 1.0)
                          );
    output.texcoord = input.texcoord;
    output.normal = normalize((float3) mul(invertTranspose(transforms[instanceIndex]), float4(input.normal, 0.0)));
    
    output.instanceIndex = instanceIndex;

    return output;
}

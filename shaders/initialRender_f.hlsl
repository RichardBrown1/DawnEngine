#include "_definitions.hlsli"
#include "initialRender.hlsli"

StructuredBuffer<InstanceProperties> instanceProperties : register(t2, space0);
StructuredBuffer<Material> materials : register(t3, space0);

struct FSOutput //FYI there is a max of 8 targets in webgpu
{
    float4 worldPosition : SV_Target0; //only xyz is used, w is currently always 1
    float4 baseColor : SV_Target1;
    float4 normal : SV_Target2; //float3(normal) + 1.0f
    float2 texCoord : SV_Target3;
    uint baseColorTextureId : SV_Target4; 
    uint normalTextureId : SV_Target5;
};

FSOutput fs_main(VSOutput input)
{
    FSOutput output;

    output.worldPosition = input.worldPosition;
    output.normal = float4(input.normal, 1.0f);
    output.texCoord = input.texcoord;

    const InstanceProperties ip = instanceProperties[input.instanceIndex];
    const Material material = materials[ip.materialIndex];

    output.baseColor = material.pbrMetallicRoughness.baseColor;
    output.baseColorTextureId = material.pbrMetallicRoughness.baseColorTextureInfo.index;
    output.normalTextureId = material.normalTextureInfo.index;

    return output;
}

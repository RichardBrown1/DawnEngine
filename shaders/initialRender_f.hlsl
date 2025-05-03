#include "_definitions.hlsli"
#include "initialRender.hlsli"

StructuredBuffer<InstanceProperties> instanceProperties : register(t2, space0);
StructuredBuffer<Material> materials : register(t3, space0);

struct FSOutput
{
    float4 masterInfoTexture : SV_Target0;
    float4 baseColor : SV_Target1;
    float4 normal : SV_Target2;
};

FSOutput fs_main(VSOutput input)
{
    FSOutput output;

    const InstanceProperties ip = instanceProperties[input.instanceIndex];
    output.masterInfoTexture = float4(
        input.texcoord.x,
        input.texcoord.y,
        asfloat(ip.materialIndex),
        0.0f
    );

    const Material material = materials[ip.materialIndex];
    output.baseColor = material.pbrMetallicRoughness.baseColor;
    
    output.normal = float4(input.normal, 0);

    return output;
}

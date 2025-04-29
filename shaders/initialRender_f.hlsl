#include "_definitions.hlsli"
#include "initialRender.hlsli"

struct FSOutput
{
    uint4 masterInfoTexture : SV_Target0;
    float4 baseColor : SV_Target1;
};

FSOutput fs_main(VSOutput input) //: SV_Target
{
    FSOutput output;
    output.masterInfoTexture = (
        asuint(input.texcoord.x),
        asuint(input.texcoord.y),
        input.materialIndex,
        0
    );
    output.baseColor = input.color;
    return float4(color);
}

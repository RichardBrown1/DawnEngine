#include "definitions.hlsli"
struct VSOutput
{
    float4 cameraPosition : SV_Position;
    float4 color : COLOR0;
    float3 normal : NORMAL0;
    float2 texcoord : TEXCOORD0;
    uint materialIndex;
};

VSOutput VS_main(VSInput input, uint VertexIndex : SV_VertexID, uint InstanceIndex : SV_InstanceID)
{
    VSOutput output = (VSOutput) 0;    
    
    const float3 position = (float3) mul(transforms[InstanceIndex], float4(input.position, 1.0));
    output.cameraPosition = mul(
                            camera.projectionView,
                            float4(output.position, 1.0)
                          );
    output.texcoord = input.texcoord;
    output.normal = normalize((float3) mul(invertTranspose(transforms[InstanceIndex]), float4(input.normal, 0.0)));
    
    const InstanceProperties ip = instanceProperties[InstanceIndex];
    const Material material = materials[ip.materialIndex];
    output.color = material.baseColor;

    output.materialIndex = ip.materialIndex;

    return output;
}

float4 FS_main(VSOutput input ) : SV_Target
{
    const float3 lightColor = float3(1.0, 1.0, 1.0);
    const float ambientStrength = float(1.0);
    const float3 ambientLight = lightColor * ambientStrength;
    float3 result = ambientLight;
    
    uint lightSize;
    uint lightStride;
    lights.GetDimensions(lightSize, lightStride);
    
    for (uint i = 0; i < lightSize; i++)
    {
        switch (lights[i].type) //TODO: All of these lights should be pre-sorted into their own arrays.
        {
            case LIGHTTYPE_DIRECTIONAL:
                result += directionalLighting(input, lights[i]);
                break;
            case LIGHTTYPE_SPOT:
                result += spotLighting(input, lights[i]);
                break;
            case LIGHTTYPE_POINT:
                result += pointLighting(input, lights[i]);
                break;
        }
    }

    const InstanceProperties ip = instanceProperties[input.instanceIndex];
    const Material material = materials[ip.materialIndex];
    const uint hasBaseColorTexture = material.textureOptions << 31;
    float4 color = input.color;
    
    float2 texcoord = input.texcoord * 0.5 + 0.5;
    texcoord.y = 1.0 - texcoord.y; // Flip Y
    const SamplerTexturePair stp = samplerTexturePair[material.baseColorTextureInfo.index];
    float4 samplerColor = textures.Sample(textureSampler, float3(input.texcoord, stp.textureIndex), int2(0, 0));
    
    if (hasBaseColorTexture)
    {
        color = samplerColor;
    }
   
        //{
        result *= color.rgb;
   // }
   // else
   // {
   //   result *= material.baseColor.rgb;
   // }

    result *= calculateShadow(input, lights[0]); //TODO: Multiple light support
   
    return float4(result, 1.0);
}

#include "definitions.hlsli"
#include "helpers.hlsli"

cbuffer pv : register(b0, space0)
{
    ProjectionView pv;
};
StructuredBuffer<float4x4> transforms : register(t1, space0);
StructuredBuffer<InstanceProperties> instanceProperties : register(t2, space0);
StructuredBuffer<Material> materials : register(t3, space0);
StructuredBuffer<Light> lights : register(t4, space0);
Texture2D shadowMap : register(t5, space0);
SamplerComparisonState depthSampler : register(t6, space0);
StructuredBuffer<SamplerTexturePair> samplerTexturePair : register(t7, space0);
Texture2DArray textures : register(t8, space0);
SamplerState textureSampler : register(t9, space0);

struct VSInput
{
    float3 position : POSTION0;
    float3 normal : NORMAL0;
    float2 texcoord : TEXCOORD0;
};

struct VSOutput
{
    float4 cameraPosition : SV_Position;
    float3 position : POSITION0;
    float4 lightPosition : POSITION1;
    float3 shadowMapPosition : POSITION2;
    float3 normal : NORMAL0;
    float4 color : COLOR0;
    uint instanceIndex : SV_InstanceID;
    //float2 texcoord : TEXCOORD0;
};

VSOutput VS_main(VSInput input, uint VertexIndex : SV_VertexID, uint InstanceIndex : SV_InstanceID)
{
    const float4x4 inverseTransposeMultiplier = float4x4(1.0f, 0.0f, 0.0f, 0.0f, 
                                                         0.0f, 1.0f, 0.0f, 0.0f,
                                                         0.0f, 0.0f, 1.0f, 0.0f, 
                                                         0.0f, 0.0f, 0.0f, 1.0f);
    
    VSOutput output = (VSOutput) 0;    
    
    output.position = (float3) mul(transforms[InstanceIndex], float4(input.position, 1.0));
    output.cameraPosition = mul(
                            mul(pv.projection, pv.view),
                            float4(output.position, 1.0)
                          );
    output.lightPosition = mul(lights[0].lightSpaceMatrix, float4(output.position, 1.0));
    output.shadowMapPosition = float3(output.lightPosition.xy * float2(0.5, -0.5) + float2(0.5, 0.5), output.lightPosition.z);
    output.normal = normalize((float3) mul(mul(inverseTransposeMultiplier, transforms[InstanceIndex]), float4(input.normal, 0.0)));
    output.color = float4(1.0, 1.0, 1.0, 1.0);
    // output.texcoord = input.texcoord;


    return output;
}

float getNDotL(float3 normal, float3 lightDir)
{
    return max(dot(normal, lightDir), 0.0);
};

float3 directionalLighting(VSOutput input, Light light)
{
    const float3 lightDir = ComputeLightDirection(light.rotation);
    const float NdotL = getNDotL(input.normal, lightDir);
    return light.color * NdotL;
}

float3 pointLighting(VSOutput input, Light light)
{
    const float3 toLight = light.position - input.position;
    const float3 lightDir = normalize(toLight);
    
    //using gltf spec sheet as reference
    const float distance = length(toLight);
    float attenuation = max(min(1.0 - pow(distance / light.range, 4.0), 2.0), 0.0);
    attenuation /= (distance * distance + 1e-6); // Prevent division by zero
    
    return light.color * getNDotL(input.normal, lightDir) * attenuation;
}

float3 spotLighting(VSOutput input, Light light)
{
    const float3 lightToFrag = normalize(input.position - light.position);
    const float3 lightForward = ComputeLightDirection(light.rotation);

    const float cosTheta = dot(lightToFrag, lightForward);
    const float cosInner = cos(light.innerConeAngle);
    const float cosOuter = cos(light.outerConeAngle);
    const float epsilon = cosInner - cosOuter;

    const float intensity = clamp((cosTheta - cosOuter) / epsilon, 0.0, 1.0);

    return pointLighting(input, light) * intensity;
}

float calculateShadow(VSOutput input, Light light)
{
    float shadowMapHeight;
    float shadowMapWidth;
    shadowMap.GetDimensions(shadowMapWidth, shadowMapHeight);
    
    const float3 projCoords = input.lightPosition.xyz / input.lightPosition.w;
    if (projCoords.z > 1.0)
    {
        return 1.0;
    }
        
    float2 shadowUV = projCoords.xy * 0.5 + 0.5;
    shadowUV.y = 1.0 - shadowUV.y; // Flip Y

    const float3 normalDirection = normalize(input.normal) * (1, 1, -1);
    const float3 fragToLight = normalize(light.position - input.position);
   // const float bias = 0.0000;
    const float currentDepth = projCoords.z; //- bias;
    
    const float oneOverShadowMapSize = 1.0 / shadowMapWidth;
    const float2 offset = normalDirection.xz * oneOverShadowMapSize * 2.0f;
    const float4 shadowValues = shadowMap.GatherCmp(
        depthSampler,
        shadowUV + offset,
        currentDepth
    );
    const float shadow = (shadowValues.x + shadowValues.y + shadowValues.z + shadowValues.w) / 4.0;

    return max(shadow, 0.25);
}


float4 FS_main(VSOutput input ) : SV_Target
{
    const float3 lightColor = float3(1.0, 1.0, 1.0);
    const float ambientStrength = float(0.1);
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

    //if(materials[input.materialIndex].baseColorTextureInfo.ind)
    const InstanceProperties ip = instanceProperties[input.instanceIndex];
    const Material material = materials[ip.materialIndex];
    const uint hasBaseColorTexture = material.textureOptions << 31;
//    if (hasBaseColorTexture)
//    {
//        const SamplerTexturePair stp = samplerTexturePair[material.baseColorTextureInfo.index];
//        const float4 color = textures.Gather(textureSampler, float3(input.texcoord, stp.textureIndex), int2(0, 0));
//        result *= color.rgb;
//    }
//    else
//    {
        result *= material.baseColor.rgb;
//    }

    result *= calculateShadow(input, lights[0]); //TODO: Multiple light support
   
    return float4(result, 1.0);
}

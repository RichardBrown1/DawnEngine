struct UniformBufferControl
{
    float4x4 projection;
    float4x4 model;
    float4x4 view;
    float4x4 inversedTransposedModel;
};
cbuffer ubo : register(b0, space0)
{
    UniformBufferControl ubo;
}

StructuredBuffer<float4x4> transforms : register(t1, space0);

struct InstanceProperties
{
    uint materialIndex;
    uint pad1;
    uint pad2;
    uint pad3;
};
StructuredBuffer<InstanceProperties> instanceProperties : register(t2, space0);

struct Material
{
    float4 baseColor;
};
StructuredBuffer<Material> materials : register(t0, space1);

struct VSInput
{
    [[vk::location(0)]] float3 Position : POSTION0;
    [[vk::location(1)]] float3 Normal : NORMAL0;
};

struct VSOutput
{
    [[vk::location(0)]] float4 Position : SV_Position;
    [[vk::location(1)]] float3 Normal : NORMAL0;
    [[vk::location(2)]] float4 Color : COLOR0;
};

VSOutput VS_main(VSInput input, uint VertexIndex : SV_VertexID, uint InstanceIndex : SV_InstanceID)
{
    VSOutput output = (VSOutput) 0;    
    
    output.Position = mul(ubo.projection, mul(ubo.view, mul(ubo.model, mul(transforms[InstanceIndex], float4(input.Position, 1.0)))));
    output.Normal = normalize((float3) mul(ubo.inversedTransposedModel, mul(transforms[InstanceIndex], float4(input.Normal, 1.0))));

    InstanceProperties ip = instanceProperties[InstanceIndex];
    output.Color = materials[ip.materialIndex].baseColor;
    return output;
}

float3 directionalLighting(float3 normal)
{
    float3 lightColor = float3(1.0, 1.0, 0.8);
    float3 lightDirection = float3(0.5, -0.9, 0.1);
    float diff = max(dot(normal, lightDirection), 0.0);
    return lightColor * diff;
}

float4 FS_main(VSOutput input) : SV_Target
{
    float3 lightColor = float3(1.0, 1.0, 1.0);
    
    float ambientStrength = float(0.1);
    float3 ambientLight = lightColor * ambientStrength;
    
    float3 result = (ambientLight + directionalLighting(input.Normal)) * input.Color.xyz;
    
    return float4(result, 1.0);
}

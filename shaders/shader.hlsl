struct UniformBufferControl
{
    float4x4 projection;
    float4x4 view;
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

struct Light
{
    float4x4 transform;
    float3 color;
    uint type;
    float intensity;
    float range;
    float innerConeAngle;
    float outerConeAngle;
};
StructuredBuffer<Light> lights : register(t1, space1);

struct VSInput
{
    [[vk::location(0)]] float3 Position : POSTION0;
    [[vk::location(1)]] float3 Normal : NORMAL0;
};

struct VSOutput
{
    [[vk::location(0)]] float4 Position : SV_Position;
    [[vk::location(1)]] float3 FragPosition : POSITION0;
    [[vk::location(2)]] float3 Normal : NORMAL0;
    [[vk::location(3)]] float4 Color : COLOR0;
};


VSOutput VS_main(VSInput input, uint VertexIndex : SV_VertexID, uint InstanceIndex : SV_InstanceID)
{
    const float4x4 inverseTransposeMultiplier = float4x4(1.0f, 0.0f, 0.0f, 0.0f, 
                                                         0.0f, 1.0f, 0.0f, 0.0f,
                                                         0.0f, 0.0f, 1.0f, 0.0f, 
                                                         0.0f, 0.0f, 0.0f, 1.0f);
    
    const float4x4 oneMatrix = float4x4(1.0f, 1.0f, 1.0f, 1.0f, 
                                                         1.0f, 1.0f, 1.0f, 1.0f,
                                                         1.0f, 1.0f, 1.0f, 1.0f, 
                                                         1.0f, 1.0f, 1.0f, 1.0f);

    VSOutput output = (VSOutput) 0;    
    
    output.FragPosition = (float3) mul(transforms[InstanceIndex], float4(input.Position, 1.0));
    output.Position = mul(ubo.projection,
                        mul(ubo.view,
                        float4(output.FragPosition, 1.0)));
    
    output.Normal = mul((float3x3) mul(inverseTransposeMultiplier, transforms[InstanceIndex]), input.Normal);

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

float3 spotLighting(VSOutput input, Light light)
{
    float3 lightColor = float3(1.0, 1.0, 1.0);
    float lightConstant = 1.0;
    float lightLinear = 0.5;
    float lightQuadratic = 0.0032;
    //const float4x4 lightTransform = mul(ubo.projection, mul(ubo.view, light.transform));
    const float4x4 lightTransform = light.transform;
    float3 lightPosition = lightTransform._m30_m31_m32;

    float3 norm = normalize(input.Normal);
  // float3 lightPosition = lightTransform._m03_m13_m23;
   // lightPosition.x = 0.0f;
    //lightPosition.y = 999.0f;
    //lightPosition.z = 0.0f;
    //lightPosition.z = 1000.0f;
    float3 lightDirection = normalize(lightPosition - input.FragPosition);
    float diff = max(dot(norm, lightDirection), 0.0);

    float distance = length(lightPosition - input.Position.xyz) / 1000;
    float attenuation = 1.0 / (lightConstant + lightLinear * distance + lightQuadratic * (distance * distance));

   // diff *= attenuation;
    
    return lightColor * diff;
    

}

float4 FS_main(VSOutput input) : SV_Target
{
    float3 lightColor = float3(1.0, 1.0, 1.0);
    float ambientStrength = float(0.1);
    float3 ambientLight = lightColor * ambientStrength;

    float3 result = (//ambientLight + 
    //directionalLighting(input.Normal) + 
    spotLighting(input, lights[0])) * input.Color.xyz;

    return float4(result, 1.0);
}

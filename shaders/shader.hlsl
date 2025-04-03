//from FASTGLTF LightType
#define LIGHTTYPE_DIRECTIONAL 0
#define LIGHTTYPE_SPOT 1
#define LIGHTTYPE_POINT 2

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
    uint PAD0;
    uint PAD1;
    uint PAD2;
};
StructuredBuffer<InstanceProperties> instanceProperties : register(t2, space0);

struct Material
{
    float4 baseColor;
};
StructuredBuffer<Material> materials : register(t3, space0);

struct Light
{
    float4x4 lightSpaceMatrix;
    float3 position;
    uint PAD0;
    float3 rotation;
    uint PAD1;
    float3 color;
    uint type;
    float intensity;
    float range;
    float innerConeAngle;
    float outerConeAngle;
};
StructuredBuffer<Light> lights : register(t4, space0);

Texture2D shadowMap : register(t5, space0);

SamplerComparisonState depthSampler : register(t6, space0);

struct VSInput
{
    [[vk::location(0)]] float3 Position : POSTION0;
    [[vk::location(1)]] float3 Normal : NORMAL0;
};

struct VSOutput
{
    [[vk::location(0)]] float4 CameraPosition : SV_Position;
    [[vk::location(1)]] float3 Position : POSITION0;
    [[vk::location(2)]] float4 LightPosition : POSITION1;
    [[vk::location(3)]] float3 ShadowMapPosition : POSITION2;
    [[vk::location(4)]] float3 Normal : NORMAL0;
    [[vk::location(5)]] float4 Color : COLOR0;
};

// Helper function to compute light direction from XYZ Euler angles (radians) in a left-handed system
float3 ComputeLightDirection(float3 eulerRadians)
{
    float x = eulerRadians.x; // Pitch (X-axis)
    float y = eulerRadians.y; // Yaw (Y-axis)
    float z = eulerRadians.z; // Roll (Z-axis)

    // Left-handed rotation matrices
    // X-axis rotation (pitch)
    float3x3 rotX = float3x3(
        1, 0, 0,
        0, cos(x), sin(x),
        0, -sin(x), cos(x)
    );

    // Y-axis rotation (yaw)
    float3x3 rotY = float3x3(
        cos(y), 0, -sin(y),
        0, 1, 0,
        sin(y), 0, cos(y)
    );

    // Z-axis rotation (roll)
    float3x3 rotZ = float3x3(
        cos(z), sin(z), 0,
        -sin(z), cos(z), 0,
        0, 0, 1
    );

    // Combine rotations: Z * Y * X (applied in XYZ order)
    float3x3 finalRot = mul(rotZ, mul(rotY, rotX));

    // Default forward direction in left-handed systems: positive Z
    float3 forward = float3(0, 0, 1);

    return normalize(mul(finalRot, forward));
}

VSOutput VS_main(VSInput input, uint VertexIndex : SV_VertexID, uint InstanceIndex : SV_InstanceID)
{
    const float4x4 inverseTransposeMultiplier = float4x4(1.0f, 0.0f, 0.0f, 0.0f, 
                                                         0.0f, 1.0f, 0.0f, 0.0f,
                                                         0.0f, 0.0f, 1.0f, 0.0f, 
                                                         0.0f, 0.0f, 0.0f, 1.0f);
    
    VSOutput output = (VSOutput) 0;    
    
    output.Position = (float3) mul(transforms[InstanceIndex], float4(input.Position, 1.0));
    output.CameraPosition = mul(
                            mul(ubo.projection, ubo.view),
                            float4(output.Position, 1.0)
                          );
    output.LightPosition = mul(lights[0].lightSpaceMatrix, float4(output.Position, 1.0));
    output.ShadowMapPosition = float3(output.LightPosition.xy * float2(0.5, -0.5) + float2(0.5, 0.5), output.LightPosition.z);
    output.Normal = normalize((float3) mul(mul(inverseTransposeMultiplier, transforms[InstanceIndex]), float4(input.Normal, 0.0)));

    InstanceProperties ip = instanceProperties[InstanceIndex];
    output.Color = materials[ip.materialIndex].baseColor;
    return output;
}

float3 directionalLighting(VSOutput input, Light light)
{
    // Get constant light direction from rotation
    float3 lightDir = ComputeLightDirection(light.rotation);
    float3 toLight = -lightDir; // Reverse direction for lighting calculation
    
    // Simple NdotL with no attenuation
    float NdotL = max(dot(input.Normal, toLight), 0.0);
    
    return light.color * NdotL;
}

float3 pointLighting(VSOutput input, Light light)
{
    // Calculate light vector and distance
    float3 toLight = light.position - input.Position;
    float distance = length(toLight);
    float3 lightDir = normalize(toLight);
    
    // Diffuse lighting calculation
    float NdotL = max(dot(input.Normal, lightDir), 0.0);
    
    // Attenuation using same falloff as reference
    float attenuation = max(min(1.0 - pow(distance / light.range, 4.0), 1.0), 0.0);
    attenuation /= (distance * distance + 1e-6); // Prevent division by zero
    
    return light.color * (attenuation * NdotL);
}

float3 spotLighting(VSOutput input, Light light)
{
    float3 lightToFrag = normalize(input.Position - light.position);
    float3 lightForward = ComputeLightDirection(light.rotation);

    float NdotL = max(dot(input.Normal, -lightToFrag), 0.0);

    float cosTheta = dot(lightToFrag, lightForward);

    float cosInner = cos(light.innerConeAngle);
    float cosOuter = cos(light.outerConeAngle);
    float epsilon = cosInner - cosOuter;

    float intensity = clamp((cosTheta - cosOuter) / epsilon, 0.0, 1.0);

    float distance = length(light.position - input.Position);
    float attenuation = max(min(1.0 - pow(distance / light.range, 4.0), 1.0), 0.0) / (distance * distance);

    return light.color * (attenuation * intensity * NdotL);
}

float calculateShadow(VSOutput input, Light light)
{
    float shadowMapHeight;
    float shadowMapWidth;
    shadowMap.GetDimensions(shadowMapWidth, shadowMapHeight);
    
    const float3 projCoords = input.LightPosition.xyz / input.LightPosition.w;
    if (projCoords.z > 1.0)
    {
        return 1.0;
    }
        
    float2 shadowUV = projCoords.xy * 0.5 + 0.5;
    shadowUV.y = 1.0 - shadowUV.y; // Flip Y

    const float3 normalDirection = normalize(input.Normal) * (1, 1, -1);
    const float3 fragToLight = normalize(light.position - input.Position);
   // const float bias = 0.0000;
    const float currentDepth = projCoords.z; //- bias;
    
    const float oneOverShadowMapSize = 1.0 / shadowMapWidth;
    const float2 offset = normalDirection.xz * oneOverShadowMapSize * 2.0f;
    float4 shadowValues = shadowMap.GatherCmp(
        depthSampler,
        shadowUV + offset,
        currentDepth
    );
    const float shadow = (shadowValues.x + shadowValues.y + shadowValues.z + shadowValues.w) / 4.0;

    return max(shadow, 0.25);
}


float4 FS_main(VSOutput input) : SV_Target
{
    float3 lightColor = float3(1.0, 1.0, 1.0);
    float ambientStrength = float(0.1);
    float3 ambientLight = lightColor * ambientStrength;
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

    result *= input.Color.xyz;

    result *= calculateShadow(input, lights[0]);
   
    return float4(result, 1.0);

}

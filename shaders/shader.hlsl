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
    [[vk::location(0)]] float4 ClipPosition : SV_Position;
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
    output.ClipPosition = mul(
                            mul(ubo.projection, ubo.view),
                            float4(output.Position, 1.0)
                          );
    output.LightPosition = mul(lights[0].lightSpaceMatrix, float4(output.Position, 1.0));
    output.ShadowMapPosition = float3(output.LightPosition.xy * float2(0.5, -0.5) + float2(0.5, 0.5), output.LightPosition.z);
    
    output.Normal = mul((float3x3) mul(inverseTransposeMultiplier, transforms[InstanceIndex]), input.Normal);

    InstanceProperties ip = instanceProperties[InstanceIndex];
    output.Color = materials[ip.materialIndex].baseColor;
    return output;
}

float3 pointLighting(VSOutput input, Light light)
{
    float3 lightToFrag = normalize(input.Position - light.position);
    float3 lightForward = ComputeLightDirection(light.rotation);

    float distance = length(light.position - input.Position);
    float attenuation = max(min(1.0 - pow(distance / light.range, 4), 1), 0) / (distance * distance);

    return light.color * attenuation;
}

float3 spotLighting(VSOutput input, Light light)
{
    float3 lightToFrag = normalize(input.Position - light.position);
    float3 lightForward = ComputeLightDirection(light.rotation);

    float cosTheta = dot(lightToFrag, lightForward);

    float cosInner = cos(light.innerConeAngle);
    float cosOuter = cos(light.outerConeAngle);
    float epsilon = cosInner - cosOuter;

    float intensity = clamp((cosTheta - cosOuter) / epsilon, 0.0, 1.0);

    float distance = length(light.position - input.Position);
    float attenuation = max(min(1.0 - pow(distance / light.range, 4), 1), 0) / (distance * distance);

    return light.color * (attenuation * intensity);
}

float calculateShadow(VSOutput input, Light light)
{
    const float DEPTH_TEXTURE_RESOLUTION = 1024.0;
    const float AMBIENT_FACTOR = 0.2;
    float shadow;
    //const float3 lightProjectionCoordinates = input.LightPosition.xyz / input.LightPosition.w;
    //
    //const float closestDepth = shadowMap.SampleCmpLevelZero(depthSampler, input.ShadowMapPosition.xy, 0.5);
    //const float currentDepth = lightProjectionCoordinates.z;
    //const float3 normal = normalize(input.Normal);
    //const float3 lightDir = normalize(input.LightPosition - input.Position);
    //shadow += currentDepth;
    float2 textureSize;
    float visibility = 0.0;
    float oneOverShadowDepthTextureSize = 1.0 / DEPTH_TEXTURE_RESOLUTION;
    for (uint y = -1; y <= 1; y++)
    {
        for (uint x = -1; x <= 1; x++)
        {
            float2 offset = float2(uint2(x, y)) * oneOverShadowDepthTextureSize;
            visibility += shadowMap.SampleCmp(
                depthSampler, 
                input.ShadowMapPosition.xy + offset, 
                input.ShadowMapPosition.z - 0.007
            );
        }
    }
    visibility /= 9.0;
    float lambertFactor = max(dot(normalize(light.position - input.Position), normalize(input.Normal)), 0.0);
    float lightingFactor = min(AMBIENT_FACTOR + visibility * lambertFactor, 1.0);
    return shadow;
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

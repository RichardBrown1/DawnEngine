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
StructuredBuffer<Material> materials : register(t0, space1);

struct Light
{
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
StructuredBuffer<Light> lights : register(t1, space1);

struct VSInput
{
    [[vk::location(0)]] float3 Position : POSTION0;
    [[vk::location(1)]] float3 Normal : NORMAL0;
};

struct VSOutput
{
    [[vk::location(0)]] float4 ClipPosition : SV_Position;
    [[vk::location(1)]] float3 Position : POSITION0;
    [[vk::location(2)]] float3 Normal : NORMAL0;
    [[vk::location(3)]] float4 Color : COLOR0;
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
    output.ClipPosition = mul(ubo.projection,
                        mul(ubo.view,
                        float4(output.Position, 1.0)));
    
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

    return float4(result, 1.0);
}

struct CameraProperties
{
    float4x4 projection;
    float4x4 view;
};
cbuffer camera : register(b0, space0)
{
    CameraProperties camera;
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


struct VSInput
{
    [[vk::location(0)]] float3 Position : POSTION0;
    [[vk::location(1)]] float3 Normal : NORMAL0;
};

struct VSOutput
{
    [[vk::location(0)]] float4 ClipPosition : SV_Position;
    [[vk::location(1)]] float4 Color : COLOR0;
};

VSOutput VS_main(VSInput input, uint VertexIndex : SV_VertexID, uint InstanceIndex : SV_InstanceID)
{
    const float4x4 inverseTransposeMultiplier = float4x4(1.0f, 0.0f, 0.0f, 0.0f,
                                                         0.0f, 1.0f, 0.0f, 0.0f,
                                                         0.0f, 0.0f, 1.0f, 0.0f,
                                                         0.0f, 0.0f, 0.0f, 1.0f);
    
    VSOutput output = (VSOutput) 0;
    
    //output.Position = (float3) mul(transforms[InstanceIndex], float4(input.Position, 1.0));
    float3 position = (float3) mul(transforms[InstanceIndex], float4(input.Position, 1.0));
    output.ClipPosition = mul(camera.projection,
                        mul(camera.view,
                        float4(position, 1.0)));
    
    //todo save normal to Texture
   // output.Normal = mul((float3x3) mul(inverseTransposeMultiplier, transforms[InstanceIndex]), input.Normal);


    InstanceProperties ip = instanceProperties[InstanceIndex];
    output.Color = materials[ip.materialIndex].baseColor;
    return output;
}


float4 FS_main(VSOutput input) : SV_Target
{
    return float4(input.Color.xyz, 1.0);
}

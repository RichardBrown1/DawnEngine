struct UniformBufferControl
{
    float4x4 projection;
    float4x4 model;
    float4x4 view;
};

cbuffer ubo 
{
    UniformBufferControl ubo;
}

struct VSInput
{
    [[vk::location(0)]] float3 Position : POSTION0;
};

struct VSOutput
{
    [[vk::location(0)]] float4 Position : SV_Position;
    [[vk::location(1)]] float4 Color : COLOR0;
};

VSOutput VS_main(VSInput input, uint VertexIndex : SV_VertexID)
{
    VSOutput output = (VSOutput) 0;
    output.Position = mul(ubo.projection, mul(ubo.view, mul(ubo.model, float4(input.Position, 1.0))));
    output.Color = float4(0.0, 0.4, (1.0 / float(VertexIndex)), 1.0); //    input.Color;
    return output;
}

float4 FS_main(VSOutput input) : SV_Target
{
    return input.Color;
}

Texture2D depthTexture: register(t0, space0);

struct VSInput
{
    [[vk::location(0)]] float2 Position : POSTION0;
};

struct VSOutput
{
    [[vk::location(0)]] float4 Position : SV_Position;
};

VSOutput VS_main(VSInput input, uint VertexIndex : SV_VertexID, uint InstanceIndex : SV_InstanceID)
{
    VSOutput output = (VSOutput) 0;
    output.Position = float4(input.Position, 0.0, 1.0);
    //todo save normal to Texture
   // output.Normal = mul((float3x3) mul(inverseTransposeMultiplier, transforms[InstanceIndex]), input.Normal);

    return output;
}

float4 FS_main(VSOutput input) : SV_Target
{
    int2 texelCoord = int2(input.Position.xy);
    float depth = depthTexture.Load(int3(texelCoord, 0)).r;

    depth = depth - 0.9;
    depth = depth * 10;
    return float4(depth, depth, depth, 1.0);
}

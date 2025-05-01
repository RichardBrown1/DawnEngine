struct VSOutput
{
    float4 cameraPosition : SV_Position;
    float3 normal : NORMAL0;
    float2 texcoord : TEXCOORD0;
    uint instanceIndex : SV_InstanceID;
};

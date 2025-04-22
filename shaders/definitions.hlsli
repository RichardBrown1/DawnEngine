//from FASTGLTF LightType
#define LIGHTTYPE_DIRECTIONAL 0
#define LIGHTTYPE_SPOT 1
#define LIGHTTYPE_POINT 2

struct ProjectionView
{
    float4x4 projectionView;
};

struct InstanceProperties
{
    uint materialIndex;
    uint PAD0;
    uint PAD1;
    uint PAD2;
};

struct Texture
{
    uint samplerIndex;
    uint imageIndex;
};

struct TextureInfo
{
	uint index;
    uint texCoord;
};

struct Material
{
    float4 baseColor;
    float metallicFactor;
    float roughnessFactor;
    TextureInfo baseColorTextureInfo;
    TextureInfo metallicRoughnessTextureInfo;
    uint textureOptions;
    uint PAD1;
};

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

struct SamplerTexturePair
{
    uint samplerIndex;
    uint textureIndex;
};

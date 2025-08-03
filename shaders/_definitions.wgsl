struct TextureInfo {
	index : u32,
	texCoord : u32,
};

struct PBRMetallicRoughness { 
	baseColor : vec4<f32>,
	metallicFactor : f32,
	roughnessFactor : f32,
	baseColorTextureInfo : TextureInfo,
	metallicRoughnessTextureInfo : TextureInfo,
	PAD0: u32,
	PAD1: u32,
};

struct Material {
	pbrMetallicRoughness : PBRMetallicRoughness,
	normalTextureInfo : TextureInfo,
	PAD0: u32,
	PAD1: u32,
};

struct PackedInfo {
	normal : u32, //unorm2x16 octahedral coords
	texCoord : u32, //unorm2x16 packed texCoords
	materialId : u32,
	PAD0 : u32,
};

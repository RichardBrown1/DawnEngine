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

struct VSOutput {
	@builtin(position) cameraPosition : vec4<f32>,
	@location(0) worldPosition : vec4<f32>,
	@location(1) normal : vec3<f32>,
	@location(2) texCoord : vec2<f32>,
	@location(3) @interpolate(flat) instanceIndex : u32,
};

struct FSOutput {
	@location(0) baseColor : vec4<f32>,
	@location(1) baseColorId : u32,
	@location(2) normalId : u32,
}

@group(0) @binding(3) var<storage, read> materialIds: array<u32>;
@group(0) @binding(4) var<storage, read> materials: array<Material>;

@fragment
fn fs_main(input : VSOutput) -> FSOutput {
	var output : FSOutput;

	let material : Material = materials[materialIds[input.instanceIndex]];
	output.baseColor = material.pbrMetallicRoughness.baseColor;
	output.baseColorId = material.pbrMetallicRoughness.baseColorTextureInfo.index;
	output.normalId = material.normalTextureInfo.index;
	return output;
}
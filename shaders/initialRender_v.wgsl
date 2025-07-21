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

@group(0) @binding(0) var<uniform> screenDimensions: vec2<u32>;
@group(0) @binding(1) var<uniform> camera: mat4x4<f32>;
@group(0) @binding(2) var<storage, read> transforms: array<mat4x4<f32>>;
@group(0) @binding(3) var<storage, read> materialIds: array<u32>;
@group(0) @binding(4) var<storage, read> materials: array<Material>;

struct VSInput {
	@location(0) position : vec3<f32>,
	@location(1) normal : vec3<f32>,
	@location(2) texCoord : vec2<f32>,
}; 

struct VSOutput {
	@builtin(position) cameraPosition : vec4<f32>,
	@location(0) worldPosition : vec4<f32>,
	@location(1) normal : vec3<f32>,
	@location(2) texCoord : vec2<f32>,
	@location(3) @interpolate(flat) instanceIndex : u32,
};

@vertex
fn vs_main(
	input : VSInput, 
	@builtin(vertex_index) vertexIndex : u32, 
	@builtin(instance_index) instanceIndex : u32
) -> VSOutput {
	var output : VSOutput;
	output.worldPosition = transforms[instanceIndex] * vec4<f32>(input.position, 1.0);
	output.cameraPosition = camera * output.worldPosition;
	output.texCoord = input.texCoord;
    output.normal = normalize((transforms[instanceIndex] * vec4<f32>(input.normal, 0.0)).xyz);
	output.instanceIndex = instanceIndex;
	return output;
}

//struct FSOutput {
//	@location(0) worldPosition : vec4<f32>,
//	@location(1) baseColor : vec4<f32>,
//	@location(2) normal : vec4<f32>,
//	@location(3) texCoord : u32,
//};

fn copyFour(input: u32) -> vec4<u32> {
	let output = vec4<u32>(input, input, input, input);
	return output;
}

@group(1) @binding(0) var worldPosition: texture_storage_2d<rgba32float, write>;
@group(1) @binding(1) var baseColor: texture_storage_2d<rgba32float, write>;
@group(1) @binding(2) var normal: texture_storage_2d<rgba32float, write>;
@group(1) @binding(3) var texCoords: texture_storage_2d<r32uint, write>;
@group(1) @binding(4) var baseColorId: texture_storage_2d<r32uint, write>;
@group(1) @binding(5) var normalId: texture_storage_2d<r32uint, write>;

@fragment
fn fs_main(input : VSOutput) {
	let coords : vec2<u32> = vec2<u32>(input.cameraPosition.xy);
	textureStore(worldPosition, coords, input.worldPosition);
	textureStore(normal, coords, vec4<f32>(input.normal, 1.0));

	let packedTexCoords: u32 = pack2x16unorm(input.texCoord);
	textureStore(texCoords, coords, copyFour(packedTexCoords));

	let material : Material = materials[materialIds[input.instanceIndex]];
	textureStore(baseColor, coords, material.pbrMetallicRoughness.baseColor);
	textureStore(baseColorId, coords, copyFour(material.pbrMetallicRoughness.baseColorTextureInfo.index));
	textureStore(normalId, coords, copyFour(material.normalTextureInfo.index));
}
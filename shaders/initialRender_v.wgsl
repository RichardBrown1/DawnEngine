#include "_definitions.wgsli"
#include "initialRender.wgsli"

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

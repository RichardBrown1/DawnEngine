struct VSOutput {
	@builtin(position) cameraPosition : vec4<f32>,
	@location(0) worldPosition : vec4<f32>,
	@location(1) normal : vec3<f32>,
	@location(2) texCoord : vec2<f32>,
	@location(3) @interpolate(flat) instanceIndex : u32,
};

struct FSOutput {
	@location(0) worldPosition : vec4<f32>,
	@location(1) normal : vec4<f32>,
	@location(2) texCoord : u32,
}

@fragment
fn fs_main(input : VSOutput) -> FSOutput {
    var output : FSOutput;
	output.worldPosition = input.worldPosition;
	output.normal = vec4<f32>(input.normal, 1.0 ); 
	output.texCoord = pack2x16unorm(input.texCoord);

	return output;
}
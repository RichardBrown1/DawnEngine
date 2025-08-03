#include "_definitions.wgsl"
#include "_helpers.wgsli"
#include "initialRender.wgsli"

struct FSOutput { //THIS IS LIMITED TO 4 OR DX12 TRIANGLE BUG WILL OCCUR
	@location(0) packedInfo : vec4<u32>,
	@location(1) worldPosition : vec4<f32>,
}

@group(0) @binding(3) var<storage, read> materialIds: array<u32>;
@group(0) @binding(4) var<storage, read> materials: array<Material>;

@fragment
fn fs_main(input : VSOutput) -> FSOutput {
    var output : FSOutput;
	output.worldPosition = input.worldPosition;
	output.packedInfo = vec4u(
		octEncodeAndPack(input.normal), 
		pack2x16unorm(input.texCoord),
		materialIds[input.instanceIndex],
		0u
	); 

	return output;
}
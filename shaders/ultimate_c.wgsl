@binding(0) @group(0) var surfaceTexture : texture_storage_2d<rgba32float, write>;
@binding(1) @group(0) var baseColorTexture : texture_storage_2d<bgra8unorm, read>;
@binding(2) @group(0) var lightingTexture : texture_storage_2d<r32uint, read>;


@compute @workgroup_size(1)
fn cs_main(@builtin(global_invocation_id) GlobalInvocationID : vec3u) {
    let coords = GlobalInvocationID.xy;
    let baseColor : vec4<f32> = textureLoad(baseColorTexture, coords);
    let lightingData : vec4<u32> = textureLoad(lightingTexture, coords);
    let lighting : vec4<f32> = unpack4x8unorm(lightingData.x);

    var result : vec4<f32> = baseColor;
    result = result * lighting;
    textureStore(surfaceTexture, coords, result);
}
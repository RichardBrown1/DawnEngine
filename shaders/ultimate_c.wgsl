@binding(0) @group(0) var baseColorTexture : texture_storage_2d<rgba16float, read>;

@binding(0) @group(1) var surfaceTexture : texture_storage_2d<bgra8unorm, write>;

@compute @workgroup_size(1)
fn cs_main(@builtin(global_invocation_id) GlobalInvocationID : vec3u) {
    let coords = GlobalInvocationID.xy;
    let result = textureLoad(baseColorTexture, coords);
    textureStore(surfaceTexture, coords, result);
}
@binding(0) @group(0) var surfaceTexture : texture_storage_2d<rgba32float, write>;
@binding(1) @group(0) var baseColorTexture : texture_storage_2d<rgba32float, read>;


@compute @workgroup_size(1)
fn cs_main(@builtin(global_invocation_id) GlobalInvocationID : vec3u) {
    let coords = GlobalInvocationID.xy;
    let result = textureLoad(baseColorTexture, coords);
    textureStore(surfaceTexture, coords, result);
}
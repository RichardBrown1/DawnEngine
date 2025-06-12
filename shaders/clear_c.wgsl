@binding(0) @group(0) var texture : texture_storage_2d<r32uint, write>;

@compute @workgroup_size(1)
fn cs_main(@builtin(global_invocation_id) GlobalInvocationID : vec3u) {
    let coords = GlobalInvocationID.xy;
    const clearValue : vec4<u32> = vec4<u32>(0, 0, 0, 0);
    textureStore(texture, coords, clearValue);
}
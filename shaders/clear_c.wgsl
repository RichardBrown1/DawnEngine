@binding(0) @group(0) var texture : texture_storage_2d<r32uint, write>;

@compute @workgroup_size(1)
fn cs_main(@builtin(global_invocation_id) GlobalInvocationID : vec3u) {
    let coords = GlobalInvocationID.xy;
    const clearValue : vec4<f32> = vec4<f32>(0.1, 0.1, 0.1, 0.1);
    const packedValue : u32 = pack4x8unorm(vec4<f32>(clearValue));

    const result : vec4<u32> = vec4<u32>(packedValue, packedValue, packedValue, packedValue);
    textureStore(texture, coords, result);
}
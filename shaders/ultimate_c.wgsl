@binding(0) @group(0) var surfaceTexture : texture_storage_2d<rgba32float, write>;
@binding(1) @group(0) var baseColorTexture : texture_storage_2d<rgba32float, read>;
@binding(2) @group(0) var lightingTexture : texture_storage_2d<r32uint, read>;
@binding(3) @group(0) var shadowTexture : texture_storage_2d<r32float, read>;


@compute @workgroup_size(1)
fn cs_main(@builtin(global_invocation_id) GlobalInvocationID : vec3u) {
    let coords = GlobalInvocationID.xy;
    let baseColor : vec4<f32> = textureLoad(baseColorTexture, coords);
    let lightingData : vec4<u32> = textureLoad(lightingTexture, coords);
    let lighting : vec4<f32> = unpack4x8unorm(lightingData.x);
    let shadow : f32 = textureLoad(shadowTexture, coords).x;

    var result : vec4<f32> = baseColor;
    result = result * lighting;
    result = result * shadow;
    textureStore(surfaceTexture, coords, result);
}
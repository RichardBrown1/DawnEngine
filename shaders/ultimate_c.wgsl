@binding(0) @group(0) var<storage, read> baseColorTexture : vec4;
@binding(1) @group(0) var<storage, read> shadowMapTexture : f32;

@binding(0) @group(1) var<storage, write> surfaceTexture : vec4;

@compute @workgroup_size(1)
fn cs_main(@builtin(global_invocation_id) GlobalInvocationID : vec2u) {
    vec4 result = baseColorTexture[GlobalInvocationID.xy];
    result *= shadowMapTexture[GlobalInvocationID.xy];
    surfaceTexture[GlobalInvocationID.xy];
}
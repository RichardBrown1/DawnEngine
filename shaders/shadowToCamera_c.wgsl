struct Light {
    lightSpaceMatrix: mat4x4<f32>,
    position: vec3<f32>,
    rotation: vec3<f32>,
    color: vec3<f32>,
    lightType: u32,
    intensity: f32,
    range: f32,
    innerConeAngle: f32,
    outerConeAngle: f32,
};

@group(0) @binding(0) var depthSampler: sampler;
@group(0) @binding(1) var shadowAccumulatorTexture: texture_storage_2d<r32float, read_write>;
@group(0) @binding(2) var worldPositionTexture: texture_storage_2d<rgba32float, read>;

@group(1) @binding(0) var shadowMapTexture: texture_depth_2d;
@group(1) @binding(1) var<uniform> lightViewProjectionMatrix : mat4x4f;

@compute @workgroup_size(1, 1, 1)
fn cs_main(@builtin(global_invocation_id) GlobalInvocationID : vec3u) {
    let coords = GlobalInvocationID.xy;
    
    //compare distance to the light
    let worldPosition : vec3<f32> = textureLoad(worldPositionTexture, coords).xyz;
    let lightPosition : vec4<f32> = lightViewProjectionMatrix * vec4(worldPosition, 1.0);

    let lightNDC : vec3<f32> = lightPosition.xyz / lightPosition.z;

    let uv : vec2<f32> = lightNDC.xy * 0.5 + 0.5;
    let depthFromLight : f32 = lightNDC.z;
    let sampledShadowDepth : f32 = textureSampleLevel(shadowMapTexture, depthSampler, uv, 0 );

    var output : f32 = 1.0;
    if(depthFromLight > sampledShadowDepth) {
       output = 0.0;
    }

    textureStore(shadowAccumulatorTexture, coords, vec4<f32>(output, output, output, output));
}

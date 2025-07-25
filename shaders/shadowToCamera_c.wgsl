struct Light {
    lightSpaceMatrix : mat4x4<f32>,
    position : vec3<f32>,
    PAD0 : u32,
    rotation : vec3<f32>,
    PAD1 : u32,
    color : vec3<f32>,
    lightType : u32,
    intensity : f32,
    range : f32,
    innerConeAngle : f32,
    outerConeAngle : f32,
};

@group(0) @binding(0) var depthSampler: sampler_comparison;
@group(0) @binding(1) var shadowAccumulatorTexture: texture_storage_2d<r32float, read_write>;
@group(0) @binding(2) var worldPositionTexture: texture_storage_2d<rgba32float, read>;
@group(0) @binding(3) var normalTexture : texture_storage_2d<rgba32float, read>;

@group(1) @binding(0) var shadowMapTexture: texture_depth_2d;
@group(1) @binding(1) var<uniform> light: Light;

@compute @workgroup_size(1, 1, 1)
fn cs_main(@builtin(global_invocation_id) GlobalInvocationID : vec3u) {
    let coords = vec2u(GlobalInvocationID.xy);
    var shadowFactor: f32 = 1.0;
    let shadowMapDimensions : vec2<u32> = textureDimensions(shadowMapTexture);
    let worldPos : vec4<f32> = vec4f(textureLoad(worldPositionTexture, coords).xyz, 1.0);
    let lightPos : vec4<f32> = light.lightSpaceMatrix * worldPos;
    let normal : vec3<f32> = textureLoad(normalTexture, coords).xyz;

    let projCoords : vec3<f32> = lightPos.xyz / lightPos.w;
    let currentDepth : f32 = projCoords.z;
    if (currentDepth > 1.0) {
        textureStore(shadowAccumulatorTexture, coords, vec4f(1.0));
        return;
    }
    var uv : vec2<f32> = projCoords.xy * 0.5 + 0.5;
    uv.y = 1.0 - uv.y;

    let normalDirection : vec3<f32> = normalize(normal);
    let fragToLight : vec3<f32> = normalize(light.position - worldPos.xyz);
    
    let oneOverShadowMapSize : f32 = 1.0 / f32(shadowMapDimensions.x);
    let offset : vec2<f32> = normalDirection.xz * oneOverShadowMapSize;
    uv = uv + offset;

    if (0.0 > uv.x || uv.x > 1.0 || 0.0 > uv.y || uv.y > 1.0) {
       textureStore(shadowAccumulatorTexture, coords, vec4f(1.0));
       return;
    }
    
    let shadow = textureSampleCompareLevel(
        shadowMapTexture,
        depthSampler,
        uv,
        currentDepth,
    );
       
    textureStore(shadowAccumulatorTexture, coords, vec4f(shadow));
}
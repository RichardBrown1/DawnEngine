struct VSOutput {
    @builtin(position) position: vec4<f32>,
    lightPosition: vec4<f32>,
    normal: vec3<f32>,
    position_ws: vec3<f32>,
};

struct Light {
    position: vec3<f32>,
};

@group(0) @binding(0) var depthSampler: sampler_comparison;
@group(0) @binding(1) var shadow: texture_storage_2d<r32float, readwrite>;
@group(0) @binding(2) var worldPosition: texture_storage_2d<rgba32float, read>;
@group(0) @binding(3) var normalPosition: texture_storage_2d<rgba32float, write>;

@group(1) @binding(0) var shadowMap: texture_depth_2d;
@group(1) @binding(1) var<uniform> lightViewProjectionMatrix : mat4x4f;

fn cs_main(@builtin(global_invocation_id) GlobalInvocationID : vec3u) -> f32 {
    let coords = GlobalInvocationID.xy;
    let shadowMapDimensions = textureDimensions(shadowMap);
    let shadowMapWidth = f32(shadowMapDimensions.x);

    let projCoords = input.lightPosition.xyz / input.lightPosition.w;

    if (projCoords.z > 1.0) {
        return 1.0;
    }

    var shadowUV = projCoords.xy * 0.5 + 0.5;
    shadowUV.y = 1.0 - shadowUV.y;

    let normalDirection = normalize(input.normal) * vec3<f32>(1.0, 1.0, -1.0);
    let fragToLight = normalize(light.position - input.position_ws);

    let currentDepth = projCoords.z;

    let oneOverShadowMapSize = 1.0 / shadowMapWidth;

    let offset = normalDirection.xz * oneOverShadowMapSize * 2.0;

    let shadowValues = textureGatherCompare(
        shadowMap,
        depthSampler,
        shadowUV + offset,
        currentDepth
    );

    let shadow = (shadowValues[0] + shadowValues[1] +
                  shadowValues[2] + shadowValues[3]) / 4.0;

    return max(shadow, 0.25);
}

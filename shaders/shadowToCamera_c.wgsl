@group(0) @binding(0) var depthSampler: sampler_comparison;
@group(0) @binding(1) var shadowAccumulatorTexture: texture_storage_2d<r32float, read_write>;
@group(0) @binding(2) var worldPositionTexture: texture_storage_2d<rgba32float, read>;
@group(0) @binding(3) var 

@group(1) @binding(0) var shadowMapTexture: texture_depth_2d;
@group(1) @binding(1) var<uniform> lightViewProjectionMatrix : mat4x4f;

@compute @workgroup_size(1, 1, 1)
fn cs_main(@builtin(global_invocation_id) GlobalInvocationID : vec3u) {
    let coords = vec2u(GlobalInvocationID.xy);
    let bias = 0.01;
    var shadowFactor: f32 = 1.0;

    // Read world position from storage texture
    let worldPos = textureLoad(worldPositionTexture, coords).xyz;
    
    // Transform world position to light clip space
    let lightClip = lightViewProjectionMatrix * vec4f(worldPos, 1.0);
    
    // Only process points in front of the light
    if (lightClip.w > 0.0) {
        // Perspective division to get NDC
       let lightNDC = lightClip.xyz / lightClip.w;
        
        // Convert to UV texture coordinates [0,1]
        var uv = lightNDC.xy * 0.5 + 0.5;
        uv.y = 1.0 - uv.y;
        let currentDepth = lightNDC.z * 0.5 + 0.5;
        
        // Check if within light frustum bounds
        if ((all(uv >= vec2f(0.0)) && 
           (all(uv <= vec2f(1.0)) &&
           (currentDepth >= 0.0) && 
           (currentDepth <= 1.0)))) {
            
            // Sample shadow map depth
            let shadowMapDepth = textureSampleCompareLevel(
                shadowMapTexture, 
                depthSampler, 
                uv + offset, 
                currentDepth,
            );

            shadowFactor = shadowMapDepth;
        }
    }
    
    // Update shadow accumulator
    let currentShadow = textureLoad(shadowAccumulatorTexture, coords).x;
    textureStore(shadowAccumulatorTexture, coords, vec4f(shadowFactor));
}
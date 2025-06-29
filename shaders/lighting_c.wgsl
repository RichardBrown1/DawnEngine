const LIGHTTYPE_DIRECTIONAL = 0u;
const LIGHTTYPE_SPOT = 1u;
const LIGHTTYPE_POINT = 2u;

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

@group(0) @binding(0) var accumulatorTexture: texture_storage_2d<r32uint, read_write>;
@group(0) @binding(1) var worldPositionTexture: texture_storage_2d<rgba32float, read>;
@group(0) @binding(2) var normalTexture: texture_storage_2d<rgba32float>, read>;

@group(1) @binding(0) var<uniform> light: Light;


fn directionalLight(light:Light, normal:vec3<f32>) -> vec3<f32> {
    let lightDir:vec3<f32> = computeLightDirection(light.rotation);
    let nDotL:f32 = getNDotL(normal, lightDir);
    return light.color * nDotL;
}

fn pointLight(light:Light, normal:vec3<f32>, worldPosition:vec3<f32>) -> vec3<f32> {
    let toLight:vec3<f32> = light.position - worldPosition;
    let lightDir:vec3<f32> = normalize(toLight);
    let distance:f32 = length(toLight);
    var attenuation:f32 = max(min(1.0 - pow(distance / light.range, 4.0), 2.0), 0.0);
    attenuation = attenuation / (distance * distance + 1e-6);
    return light.color * getNDotL(normal, lightDir) * attenuation;
}

fn spotLight(light:Light, normal:vec3<f32>, worldPosition:vec3<f32>) -> vec3<f32> {
    let lightToFrag:vec3<f32> = normalize(worldPosition - light.position);
    let lightForward:vec3<f32> = computeLightDirection(light.rotation);
    let cosTheta:f32 = dot(lightToFrag, lightForward);
    let cosInner:f32 = cos(light.innerConeAngle);
    let cosOuter:f32 = cos(light.outerConeAngle);
    let epsilon:f32 = cosInner - cosOuter;
    let intensity:f32 = clamp((cosTheta - cosOuter)/ epsilon, 0.0, 1.0);
    
    return pointLight(light, normal, worldPosition) * intensity;
}

@compute @workgroup_size(1, 1, 1)
fn cs_main(@builtin(global_invocation_id) GlobalInvocationID: vec3<u32>) {
    let loadAccumulator : vec4<u32> = (textureLoad(accumulatorTexture, GlobalInvocationID.xy));
    var accumulator : vec3<f32> = unpack4x8unorm(loadAccumulator.x).xyz;
    let worldPosition : vec3<f32> = textureLoad(worldPositionTexture, GlobalInvocationID.xy).xyz;
    let normal : vec3<f32> = textureLoad(normalTexture, GlobalInvocationID.xy).xyz;
    switch(light.lightType) {
        case LIGHTTYPE_DIRECTIONAL {
            accumulator = accumulator + directionalLight(light, normal);
        }
        case LIGHTTYPE_POINT {
            accumulator = accumulator + pointLight(light, normal, worldPosition);
        }
        case LIGHTTYPE_SPOT {
            accumulator = accumulator + spotLight(light, normal, worldPosition);
        }
        case default: {}
    }
    let result : u32 = pack4x8unorm(vec4<f32>(accumulator, 1.0));
    textureStore(accumulatorTexture, GlobalInvocationID.xy, vec4<u32>(result, result, result, result));
}

fn getNDotL(normal:vec3<f32>, lightDir:vec3<f32>) -> f32 {
    return max(dot(normal, lightDir), 0.0);
};

fn computeLightDirection(eulerRadians: vec3<f32>) -> vec3<f32> {
    let x = eulerRadians.x; // Pitch (X-axis)
    let y = eulerRadians.y; // Yaw (Y-axis)
    let z = eulerRadians.z; // Roll (Z-axis)

    // X-axis rotation (pitch)
    let rotX = mat3x3<f32>(
        vec3<f32>(1.0, 0.0, 0.0),
        vec3<f32>(0.0, cos(x), sin(x)),
        vec3<f32>(0.0, -sin(x), cos(x))
    );

    // Y-axis rotation (yaw)
    let rotY = mat3x3<f32>(
        vec3<f32>(cos(y), 0.0, -sin(y)),
        vec3<f32>(0.0, 1.0, 0.0),
        vec3<f32>(sin(y), 0.0, cos(y))
    );

    // Z-axis rotation (roll)
    let rotZ = mat3x3<f32>(
        vec3<f32>(cos(z), sin(z), 0.0),
        vec3<f32>(-sin(z), cos(z), 0.0),
        vec3<f32>(0.0, 0.0, 1.0)
    );

    // Combine rotations: Z * Y * X (applied in XYZ order)
    let finalRot = rotZ * (rotY * rotX);

    // Default forward direction in left-handed systems: positive Z
    let forward = vec3<f32>(0.0, 0.0, 1.0);

    return normalize(finalRot * forward);
}

#include "_definitions.wgsli"
#include "_helpers.wgsli"

@group(0) @binding(0) var packedInfoTexture : texture_storage_2d<rgba32uint, read>;
@group(0) @binding(1) var<storage, read> materials : array<Material>;
@group(0) @binding(2) var normalTexture : texture_storage_2d<rgba32float, write>;
@group(0) @binding(3) var texCoordTexture : texture_storage_2d<r32uint, write>;
@group(0) @binding(4) var baseColorTexture : texture_storage_2d<rgba32float, write>;
@group(0) @binding(5) var baseColorIdTexture : texture_storage_2d<r32uint, write>;
@group(0) @binding(6) var normalIdTexture : texture_storage_2d<r32uint, write>;


@compute @workgroup_size(1)
fn cs_main(@builtin(global_invocation_id) GlobalInvocationID : vec3u) {
    let coords = GlobalInvocationID.xy;
    let packedInfo : vec4<u32> = textureLoad(packedInfoTexture, coords);
    let normal : vec3<f32> = octDecodeAndUnpack(packedInfo.x);
    let texCoord : u32 = packedInfo.y;
    let materialId : u32 = packedInfo.z;

    textureStore(normalTexture, coords, vec4f(normal, 1.0));
    textureStore(texCoordTexture, coords, vec4u(texCoord));

    let material : Material = materials[materialId];
    textureStore(baseColorTexture, coords, material.pbrMetallicRoughness.baseColor);
    textureStore(baseColorIdTexture, coords, vec4u(material.pbrMetallicRoughness.baseColorTextureInfo.index));
    textureStore(normalIdTexture, coords, vec4u(material.normalTextureInfo.index));
}
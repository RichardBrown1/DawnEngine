struct InputInfo {
    stpIndex: u32,
    PAD0: u32,
    PAD1: u32,
    PAD2: u32,
};

@group(0) @binding(0) var accumulatorTexture: texture_storage_2d<rgba32float, write>;
@group(0) @binding(1) var texCoordTexture: texture_storage_2d<r32uint, read>;
@group(0) @binding(2) var textureIdTexture : texture_storage_2d<r32uint, read>;
@group(0) @binding(3) var<uniform> screenDimensions : vec2<u32>;

@group(1) @binding(0) var<uniform> inputInfo: InputInfo;
@group(1) @binding(1) var inputTexture: texture_2d<f32>;
@group(1) @binding(2) var inputSamplerState: sampler;

@compute @workgroup_size(1, 1, 1)
fn cs_main(@builtin(global_invocation_id) global_id: vec3<u32>) {
    let textureId : u32 = textureLoad(textureIdTexture, global_id.xy).x;

    if (textureId == inputInfo.stpIndex) {
        let packedTexCoord : u32 = textureLoad(texCoordTexture, global_id.xy).x;
        let texCoord : vec2<f32> = unpack2x16unorm(packedTexCoord);

        let sampledColor = textureSampleLevel(
            inputTexture,
            inputSamplerState,
            texCoord,
            0.0
        );

        textureStore(
            accumulatorTexture,
            global_id.xy,
            sampledColor
        );
    }
}

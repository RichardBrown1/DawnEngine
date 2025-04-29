#include <vector>
#include <webgpu/webgpu_cpp.h>
#include <fastgltf/types.hpp>
#include "constants.hpp"
#include "structs.hpp"

struct TextureSamplerManagerDescriptor {
	wgpu::Device device;
	wgpu::Extent2D accumulatorTextureDimensions;
	uint32_t invocationSize;
	std::unordered_map<uint32_t, DawnEngine::TextureType>& textureIndicesMap;
	wgpu::TextureFormat baseColorAccumulatorTextureFormat;
};
struct GenerateTexturePipelineDescriptor {
	wgpu::TextureView colorTextureView;
	wgpu::TextureFormat colorTextureFormat;
};
struct GenerateAccumulatorAndInfoBindGroupDescriptor {
	wgpu::TextureView accumulatorTextureView;
	wgpu::Buffer infoBuffer;
};
struct GenerateInputTextureBindGroupDescriptor {
	wgpu::Buffer textureInputInfoBuffer;
	wgpu::TextureView inputTexture;
	wgpu::Sampler inputSampler;
};
struct DoTextureCommandsBindGroupDescriptor {
	wgpu::CommandEncoder commandEncoder;
	wgpu::TextureView accumulatorTextureView;
	wgpu::Buffer infoBuffer;
};

class TextureSamplerManager {
public:
	TextureSamplerManager(const TextureSamplerManagerDescriptor* descriptor);
	void addAsset(fastgltf::Asset& asset, std::string gltfDirectory);
	void doTextureCommands(const DoTextureCommandsBindGroupDescriptor* descriptor);
	wgpu::ComputePipeline generateTexturePipeline(const GenerateTexturePipelineDescriptor* descriptor);
	wgpu::BindGroup generateAccumulatorAndInfoBindGroup(const GenerateAccumulatorAndInfoBindGroupDescriptor* descriptor);
	wgpu::BindGroup generateInputTextureBindGroup(const GenerateInputTextureBindGroupDescriptor* descriptor);
	void addTextureInputInfoBuffer(uint32_t samplerTexturePairIndex);

private:
	const wgpu::StringView BASE_COLOR_ACCUMULATOR_SHADER_LABEL = "base color accumulator shader";
	const std::string BASE_COLOR_ACCUMULATOR_SHADER_PATH = "shaders/c_colorAccumulator.spv";
	wgpu::ShaderModule _baseColorAccumulatorShaderModule;
	wgpu::TextureFormat baseColorAccumulatorTextureFormat;

	wgpu::Device _device;
	wgpu::ComputePipeline _computePipeline;
	wgpu::Extent2D _accumulatorTextureDimensions;
	uint32_t _invocationSize;
	std::unordered_map<uint32_t, DawnEngine::TextureType> _textureIndicesMap;
	std::vector<DawnEngine::SamplerTexturePair> _samplerTexturePairs;
	std::vector<wgpu::Buffer> _textureInputInfoBuffers;
	std::vector<wgpu::Texture> _textures;
	std::vector<wgpu::TextureView> _textureViews;
	std::vector<wgpu::Sampler> _samplers;

	wgpu::BindGroupLayout getAccumulatorAndInfoBindGroupLayout(wgpu::TextureFormat accumulatorTextureFormat);
	wgpu::BindGroupLayout getInputBindGroupLayout();
	void addSamplerTexturePair(fastgltf::Texture texture, DawnEngine::TextureType textureType);
	void addTexture(fastgltf::DataSource dataSource, std::string gltfDirectory);
	void addSampler(fastgltf::Sampler sampler);
};

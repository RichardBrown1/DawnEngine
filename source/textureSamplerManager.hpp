#include <vector>
#include <webgpu/webgpu_cpp.h>
#include <fastgltf/types.hpp>
#include "constants.hpp"
#include "structs.hpp"

struct TextureSamplerManagerDescriptor {
	wgpu::Device device;
	wgpu::Extent2D workgroupSize = {
		.width = 1024,
		.height = 720,
	};
	uint32_t invocationSize;
	std::unordered_map<uint32_t, DawnEngine::TextureType>& textureIndicesMap;
};
struct GenerateTexturePipelineDescriptor {
	wgpu::TextureView colorTextureView;
	wgpu::TextureFormat colorTextureFormat;
};

struct TextureBindGroupLayoutDescriptor {
	wgpu::TextureFormat accumulatorTextureFormat;
	wgpu::SamplerBindingType inputSamplerBindingType;
};

class TextureSamplerManager {
public:
	TextureSamplerManager(const TextureSamplerManagerDescriptor& textureSamplerManagerDescriptor);
	void addAsset(fastgltf::Asset& asset, std::string gltfDirectory);
	void doTextureCommands(wgpu::CommandEncoder& commandEncoder);
	wgpu::ComputePipeline generateTexturePipeline(GenerateTexturePipelineDescriptor descriptor);

private:
	const wgpu::StringView BASE_COLOR_ACCUMULATOR_SHADER_LABEL = "base color accumulator shader";
	const std::string BASE_COLOR_ACCUMULATOR_SHADER_PATH = "../shaders/baseColor.hlsl";
	wgpu::ShaderModule _baseColorAccumulatorShaderModule;

	wgpu::Device _device;
	wgpu::ComputePipeline _computePipeline;
	wgpu::Extent2D _workgroupSize;
	uint32_t _invocationSize;
	std::unordered_map<uint32_t, DawnEngine::TextureType> _textureIndicesMap;
	std::vector<DawnEngine::SamplerTexturePair> _samplerTexturePair;
	std::vector<wgpu::Texture> _textures;
	std::vector<wgpu::TextureView> _textureViews;
	std::vector<wgpu::Sampler> _samplers;

	wgpu::BindGroupLayout getBindGroupLayout(wgpu::TextureFormat accumulatorTextureFormat);
	void addTextureSamplerPair(fastgltf::Texture texture, DawnEngine::TextureType textureType);
	void addTexture(fastgltf::DataSource dataSource, std::string gltfDirectory);
	void addSampler(fastgltf::Sampler sampler);
};

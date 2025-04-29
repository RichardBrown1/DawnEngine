#include <vector>
#include <webgpu/webgpu_cpp.h>
#include <fastgltf/types.hpp>
#include "constants.hpp"
#include "structs.hpp"

namespace DawnEngine {

	struct TextureSamplerManagerDescriptor {
		wgpu::Device device;
		wgpu::Extent2D accumulatorTextureDimensions;
		uint32_t invocationSize;
		std::unordered_map<uint32_t, DawnEngine::TextureType>& textureIndicesMap;
		wgpu::TextureFormat baseColorAccumulatorTextureFormat;
	};
	struct CreateTexturePipelineDescriptor {
		wgpu::TextureView colorTextureView;
		wgpu::TextureFormat colorTextureFormat;
	};
	struct CreateAccumulatorAndInfoBindGroupDescriptor {
		wgpu::TextureView accumulatorTextureView;
		wgpu::TextureView masterTextureInfoTextureView;
	};
	struct CreateInputTextureBindGroupDescriptor {
		wgpu::Buffer textureInputInfoBuffer;
		wgpu::TextureView inputTexture;
		wgpu::Sampler inputSampler;
	};
	struct DoTextureSamplerCommandsDescriptor {
		wgpu::CommandEncoder commandEncoder;
		wgpu::TextureView accumulatorTextureView;
		wgpu::TextureView masterTextureInfoTextureView;
	};

	class TextureSamplerManager {
	public:
		TextureSamplerManager(const TextureSamplerManagerDescriptor* descriptor);
		void addAsset(fastgltf::Asset& asset, std::string gltfDirectory);
		void doCommands(const DoTextureSamplerCommandsDescriptor* descriptor);

	private:
		const wgpu::StringView BASE_COLOR_ACCUMULATOR_SHADER_LABEL = "base color accumulator shader";
		const std::string BASE_COLOR_ACCUMULATOR_SHADER_PATH = "shaders/c_colorAccumulator.spv";
		wgpu::ShaderModule _baseColorAccumulatorShaderModule;
		wgpu::TextureFormat baseColorAccumulatorTextureFormat;

		wgpu::Device _device;
		wgpu::Queue _queue;
		wgpu::ComputePipeline _computePipeline;
		wgpu::Extent2D _accumulatorTextureDimensions;
		uint32_t _invocationSize;
		std::unordered_map<uint32_t, DawnEngine::TextureType> _textureIndicesMap;
		std::vector<DawnEngine::SamplerTexturePair> _samplerTexturePairs;
		std::vector<wgpu::Buffer> _textureInputInfoBuffers;
		std::vector<wgpu::Texture> _textures;
		std::vector<wgpu::TextureView> _textureViews;
		std::vector<wgpu::Sampler> _samplers;

		wgpu::ComputePipeline createTexturePipeline(const CreateTexturePipelineDescriptor* descriptor);
		wgpu::BindGroupLayout getAccumulatorAndInfoBindGroupLayout(wgpu::TextureFormat accumulatorTextureFormat);
		wgpu::BindGroupLayout getInputBindGroupLayout();
		wgpu::BindGroup createAccumulatorAndInfoBindGroup(const CreateAccumulatorAndInfoBindGroupDescriptor* descriptor);
		wgpu::BindGroup createInputTextureBindGroup(const CreateInputTextureBindGroupDescriptor* descriptor);
		void addSamplerTexturePair(fastgltf::Texture texture, DawnEngine::TextureType textureType);
		void addTextureInputInfoBuffer(uint32_t samplerTexturePairIndex);
		void addTexture(fastgltf::DataSource dataSource, std::string gltfDirectory);
		void addSampler(fastgltf::Sampler sampler);
	};
}

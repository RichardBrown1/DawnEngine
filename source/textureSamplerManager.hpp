#pragma once
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
	struct CreateInputTextureBindGroupsDescriptor {
		std::vector<SamplerTexturePair> samplerTexturePairs;
	};
	struct GenerateGpuObjectsDescriptor {
		CreateTexturePipelineDescriptor texturePipelineDescriptor;
		CreateAccumulatorAndInfoBindGroupDescriptor accumulatorAndInfoBindGroupDescriptor;
		CreateInputTextureBindGroupsDescriptor inputTextureBindGroupsDescriptor;
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
		void generateGpuObjects(const GenerateGpuObjectsDescriptor* descriptor);
		void doCommands(const DoTextureSamplerCommandsDescriptor* descriptor);

	private:
		const wgpu::StringView BASE_COLOR_ACCUMULATOR_SHADER_LABEL = "base color accumulator shader";
		const std::string BASE_COLOR_ACCUMULATOR_SHADER_PATH = "shaders/c_colorAccumulator.spv";
		wgpu::ShaderModule _baseColorAccumulatorShaderModule;
		wgpu::TextureFormat baseColorAccumulatorTextureFormat;

		wgpu::Device _device;
		wgpu::Queue _queue;
		wgpu::ComputePipeline _computePipeline;
		wgpu::BindGroup _accumulatorAndInfoBindGroup;
		std::vector<wgpu::BindGroup> _inputTextureBindGroups;
		wgpu::Extent2D _accumulatorTextureDimensions;
		uint32_t _invocationSize;
		std::unordered_map<uint32_t, DawnEngine::TextureType> _textureIndicesMap;
		std::vector<DawnEngine::SamplerTexturePair> _samplerTexturePairs;
		std::vector<wgpu::Buffer> _textureInputInfoBuffers;
		std::vector<wgpu::Texture> _textures;
		std::vector<wgpu::TextureView> _textureViews;
		std::vector<wgpu::Sampler> _samplers;

		void createTexturePipeline(const CreateTexturePipelineDescriptor* descriptor);
		wgpu::BindGroupLayout getAccumulatorAndInfoBindGroupLayout(wgpu::TextureFormat accumulatorTextureFormat);
		wgpu::BindGroupLayout getInputBindGroupLayout();
		void createAccumulatorAndInfoBindGroup(const CreateAccumulatorAndInfoBindGroupDescriptor* descriptor);
		void createInputTextureBindGroups(const CreateInputTextureBindGroupsDescriptor* descriptor);
		void addSamplerTexturePair(fastgltf::Texture texture, DawnEngine::TextureType textureType);
		void addTextureInputInfoBuffer(uint32_t samplerTexturePairIndex);
		void addTexture(fastgltf::DataSource dataSource, std::string gltfDirectory);
		void addSampler(fastgltf::Sampler sampler);
	};
}

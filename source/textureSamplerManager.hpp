#pragma once
#include <vector>
#include <webgpu/webgpu_cpp.h>
#include <fastgltf/types.hpp>
#include "constants.hpp"
#include "structs.hpp"


namespace {
	struct CreateTexturePipelineDescriptor {
		wgpu::TextureView colorTextureView;
		wgpu::TextureFormat colorTextureFormat;
	};
	struct CreateAccumulatorAndInfoBindGroupDescriptor {
		wgpu::TextureView accumulatorTextureView;
		wgpu::TextureView masterTextureInfoTextureView;
	};
	struct CreateInputTextureBindGroupsDescriptor {
		std::vector<DawnEngine::SamplerTexturePair> samplerTexturePairs;
	};
}


namespace DawnEngine {
	namespace Descriptors {
		namespace TextureSamplerManager {
			struct Constructor {
				wgpu::Device *device;
			};

			struct AddAsset {
				fastgltf::Asset& asset;
				std::string gltfDirectory;
				std::unordered_map<uint32_t, DawnEngine::TextureType>& textureIndicesMap;
			};

			struct GenerateGpuObjects {
				wgpu::Extent2D screenDimensions;
				wgpu::TextureFormat baseColorAccumulatorTextureFormat;
				CreateTexturePipelineDescriptor texturePipelineDescriptor;
				CreateAccumulatorAndInfoBindGroupDescriptor accumulatorAndInfoBindGroupDescriptor;
				CreateInputTextureBindGroupsDescriptor inputTextureBindGroupsDescriptor;
			};

			struct DoCommands {
				wgpu::CommandEncoder commandEncoder;
				wgpu::TextureView accumulatorTextureView;
				wgpu::TextureView masterTextureInfoTextureView;
			};
		}
	}
	class TextureSamplerManager {
	public:
		TextureSamplerManager(const DawnEngine::Descriptors::TextureSamplerManager::Constructor* descriptor);
		void addAsset(const DawnEngine::Descriptors::TextureSamplerManager::AddAsset* descriptor);
		void generateGpuObjects(const DawnEngine::Descriptors::TextureSamplerManager::GenerateGpuObjects* descriptor);
		void doCommands(const DawnEngine::Descriptors::TextureSamplerManager::DoCommands* descriptor);

	private:
		const wgpu::StringView BASE_COLOR_ACCUMULATOR_SHADER_LABEL = "base color accumulator shader";
		const std::string BASE_COLOR_ACCUMULATOR_SHADER_PATH = "shaders/c_colorAccumulator.spv";
		wgpu::ShaderModule _baseColorAccumulatorShaderModule;

		wgpu::Device* _device;
		wgpu::Queue _queue;

		wgpu::ComputePipeline _computePipeline;
		wgpu::BindGroup _accumulatorAndInfoBindGroup;
		std::vector<wgpu::BindGroup> _inputTextureBindGroups;
		wgpu::Extent2D _screenDimensions;
		std::unordered_map<uint32_t, DawnEngine::TextureType> _stpIndexToTextureTypeMap;
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

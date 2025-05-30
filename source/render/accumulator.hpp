#pragma once
#include <vector>
#include <unordered_map>
#include <fastgltf/types.hpp>
#include "../structs/structs.hpp"
#include <string>
#include "../wgpuContext/wgpuContext.hpp"
#include <dawn/webgpu_cpp.h>

namespace render {
	namespace accumulator {
		namespace descriptor {
			struct GenerateGpuObjects {
				WGPUContext& wgpuContext;

				wgpu::TextureView& accumulatorTextureView;
				wgpu::TextureFormat accumulatorTextureFormat;
				wgpu::TextureView& texCoordTextureView;
				wgpu::TextureFormat texCoordTextureFormat;
				wgpu::TextureView& textureIdTextureView;
				wgpu::TextureFormat textureIdTextureFormat;

				//for input bind group
				std::vector<structs::SamplerTexturePair>& inputSTPs;
				std::vector<wgpu::TextureView>& allTextureViews;
				std::vector<wgpu::Sampler>& allSamplers;
			};

			struct DoCommands {
				wgpu::CommandEncoder& commandEncoder;
			};
		}
	}

	class Accumulator {
	public:
		Accumulator(wgpu::Device* device);
		void generateGpuObjects(const render::accumulator::descriptor::GenerateGpuObjects* descriptor);
		void doCommands(const render::accumulator::descriptor::DoCommands* descriptor);

	private:
		const wgpu::StringView BASE_COLOR_ACCUMULATOR_SHADER_LABEL = "base color accumulator shader";
		const std::string BASE_COLOR_ACCUMULATOR_SHADER_PATH = "shaders/c_colorAccumulator.spv";
		wgpu::ShaderModule _computeShaderModule;

		wgpu::Device* _device;
		wgpu::Extent2D _screenDimensions;

		std::vector<wgpu::Buffer> _infoBuffer;

		wgpu::BindGroupLayout _accumulatorBindGroupLayout;
		wgpu::BindGroupLayout _inputBindGroupLayout;
		wgpu::BindGroup _accumulatorBindGroup;
		std::vector<wgpu::BindGroup> _inputBindGroups;
		wgpu::ComputePipeline _computePipeline;

		void createAccumulatorBindGroupLayout(
			wgpu::TextureFormat accumulatorTextureFormat,
			wgpu::TextureFormat texCoordTextureFormat,
			wgpu::TextureFormat textureIdTextureFormat
		);
		void createInputBindGroupLayout();
		wgpu::PipelineLayout getPipelineLayout();
		void createComputePipeline();

		void createAccumulatorBindGroup(
			wgpu::TextureView& accumulatorTextureView,
			wgpu::TextureView& texCoordTextureFormat,
			wgpu::TextureView& textureIdTextureFormat
		);
		void insertInputBindGroup(
			wgpu::Buffer& buffer,
			wgpu::TextureView& inputTextureView,
			wgpu::Sampler& sampler
		);
	};
}
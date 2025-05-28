#pragma once
#include <vector>
#include <unordered_map>
#include <webgpu/webgpu_cpp.h>
#include <fastgltf/types.hpp>
#include "../structs/structs.hpp"

namespace render {
	namespace accumulator {
		namespace descriptor {
			struct GenerateGpuObjects {
				wgpu::TextureView& accumulatorTextureView;
				wgpu::TextureFormat accumulatorTextureFormat;
				wgpu::TextureView& infoTextureView;
				wgpu::TextureFormat infoTextureFormat;

				//for input bind group
				std::vector<structs::SamplerTexturePair>& inputSTPs;
				std::vector<wgpu::TextureView>& allTextureViews;
				std::vector<wgpu::Sampler>& allSamplers;
			};

			struct DoCommands {
				wgpu::CommandEncoder& commandEncoder;
				std::vector<wgpu::TextureView>& inputTextureViews;
				wgpu::TextureView& accumulatorTextureView;
				wgpu::TextureView& masterTextureInfoTextureView;
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

		wgpu::BindGroupLayout _accumulatorBindGroupLayout;
		wgpu::BindGroupLayout _inputBindGroupLayout;
		wgpu::BindGroup _accumulatorBindGroup;
		std::vector<wgpu::BindGroup> _inputBindGroups;
		wgpu::ComputePipeline _computePipeline;

		void createAccumulatorBindGroupLayout(
			wgpu::TextureFormat accumulatorTextureFormat,
			wgpu::TextureFormat infoTextureFormat
		);
		void createInputBindGroupLayout();
		wgpu::PipelineLayout getPipelineLayout();
		void createComputePipeline();

		void createAccumulatorBindGroup(
			wgpu::TextureView& accumulatorTextureView,
			wgpu::TextureView& infoTextureView
		);
		void insertInputBindGroup(wgpu::TextureView& inputTextureView, wgpu::Sampler& sampler);
	};
}
#pragma once
#include <vector>
#include <unordered_map>
#include "../../structs/structs.hpp"
#include <string>
#include "../../wgpuContext/wgpuContext.hpp"
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
				std::vector<uint32_t>& stpIds;
				std::vector<structs::SamplerTexturePair>& inputSTPs;
				std::vector<wgpu::TextureView>& allTextureViews;
				std::unordered_map<uint32_t, wgpu::Sampler>& allSamplers;
			};

			struct DoCommands {
				wgpu::CommandEncoder& commandEncoder;
				wgpu::Extent2D& screenDimensions;
			};
		}
	}

	template <typename Derived>
	class BaseAccumulator {
	public:
		BaseAccumulator(WGPUContext* wgpuContext);
		void generateGpuObjects(const render::accumulator::descriptor::GenerateGpuObjects* descriptor);
		void doCommands(const render::accumulator::descriptor::DoCommands* descriptor);

	private:
				wgpu::ShaderModule _computeShaderModule;

		WGPUContext* _wgpuContext;

		std::vector<wgpu::Buffer> _infoBuffer;

		wgpu::BindGroupLayout _accumulatorBindGroupLayout;
		wgpu::BindGroupLayout _inputBindGroupLayout;
		wgpu::BindGroup _accumulatorBindGroup;
		std::vector<wgpu::BindGroup> _inputBindGroups;
		wgpu::ComputePipeline _computePipeline;

		void createAccumulatorBindGroupLayout(
			const wgpu::TextureFormat accumulatorTextureFormat,
			const wgpu::TextureFormat texCoordTextureFormat,
			const wgpu::TextureFormat textureIdTextureFormat
		);
		void createInputBindGroupLayout();
		wgpu::PipelineLayout getPipelineLayout();
		void createComputePipeline();

		void createAccumulatorBindGroup(
			const wgpu::TextureView& accumulatorTextureView,
			const wgpu::TextureView& texCoordTextureFormat,
			const wgpu::TextureView& textureIdTextureFormat
		);
		void insertInputBindGroup(
			const wgpu::Buffer& buffer,
			const wgpu::TextureView& inputTextureView,
			const wgpu::Sampler& sampler
		);
	};
}
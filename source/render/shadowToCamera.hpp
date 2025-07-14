#pragma once
#include "../wgpuContext/wgpuContext.hpp"
#include "../device/resources.hpp"

namespace render {
	namespace shadowToCamera {
		namespace descriptor {
			struct DoCommands {
				wgpu::CommandEncoder& commandEncoder;
			};
		}
	}

	class ShadowToCamera {
	public:
		ShadowToCamera(WGPUContext* wgpuContext);
		void generateGpuObjects(const DeviceResources* deviceResources);
		void doCommands(const render::shadowToCamera::descriptor::DoCommands* descriptor);

	private:
		const wgpu::StringView SHADOWTOCAMERA_SHADER_LABEL = "shadowToCamera render compute shader";
		const std::string SHADOWTOCAMERA_SHADER_PATH = "shaders/shadowToCamera_c.wgsl";
		wgpu::ShaderModule _computeShaderModule;

		WGPUContext* _wgpuContext;

		wgpu::BindGroupLayout _inputBindGroupLayout;
		wgpu::BindGroupLayout _accumulatorBindGroupLayout;
		wgpu::ComputePipeline _computePipeline;

		std::vector<wgpu::BindGroup> _inputBindGroups;
		wgpu::BindGroup _accumulatorBindGroup;

		wgpu::PipelineLayout getPipelineLayout();
		void createAccumulatorBindGroupLayout(
			wgpu::TextureFormat shadowMapTextureFormat,
			wgpu::TextureFormat worldPositionTextureFormat,
			wgpu::TextureFormat normalTextureFormat
		);
		void createInputBindGroupLayout();
		void createPipeline();
		void createAccumulatorBindGroup(
			wgpu::Sampler& shadowMapSampler,
			wgpu::TextureView& shadowTextureView,
			wgpu::TextureView& worldPositionTextureFormat,
			wgpu::TextureView& normalTextureFormat
		);
		void insertInputBindGroup(
			wgpu::TextureView& shadowMapTextureView,
			wgpu::Buffer& light
		);


	};
}

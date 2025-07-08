#pragma once
#include "../wgpuContext/wgpuContext.hpp"
#include "../device/resources.hpp"
/*
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
		const std::string _displayTextureViewLabel = "display ";

		const wgpu::StringView SHADOWTOCAMERA_SHADER_LABEL = "shadowToCamera render compute shader";
		const std::string SHADOWTOCAMERA_SHADER_PATH = "shaders/shadowToCamera_c.wgsl";
		wgpu::ShaderModule _computeShaderModule;

		WGPUContext* _wgpuContext;

		wgpu::ComputePipeline _computePipeline;
		wgpu::BindGroupLayout _bindGroupLayout;
		wgpu::BindGroup _bindGroup;

		wgpu::PipelineLayout getPipelineLayout();
		void createBindGroupLayout(
			wgpu::TextureFormat ultimateTextureFormat,
			wgpu::TextureFormat baseColorTextureFormat,
			wgpu::TextureFormat lightingTextureFormat
		);
		void createPipeline();
		void createBindGroup(
			wgpu::TextureView& shadowTextureView,
			wgpu::TextureView& lightingTextureView
		);

		void createInputBindGroup() {
			wgpu::TextureView& shadowMapTextureView,
			wgpu::Buffer 
		}


	};
}
*/
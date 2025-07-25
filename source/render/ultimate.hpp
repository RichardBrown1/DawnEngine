#pragma once
#include <vector>
#include <string>
#include <iostream>
#include <dawn/webgpu_cpp.h>
#include <fastgltf/types.hpp>
#include "../constants.hpp"
#include "../structs/host.hpp"
#include "../wgpuContext/wgpuContext.hpp"
#include "../device/resources.hpp"

namespace render {
	namespace ultimate::descriptor {

		struct GenerateGpuObjects {
			wgpu::TextureFormat baseColorTextureFormat;
			wgpu::TextureView& baseColorTextureView;
			wgpu::TextureFormat lightingTextureFormat;
			wgpu::TextureView& lightingTextureView;
			wgpu::TextureFormat shadowTextureFormat;
			wgpu::TextureView& shadowTextureView;
		};

		struct DoCommands {
			wgpu::CommandEncoder& commandEncoder;
		};
	}

	class Ultimate {
	public:
		Ultimate(WGPUContext* wgpuContext);
		void generateGpuObjects(const DeviceResources* deviceResources);
		void doCommands(const render::ultimate::descriptor::DoCommands* descriptor);

	private:
		const std::string _displayTextureViewLabel = "display ";

		const wgpu::StringView ULTIMATE_SHADER_LABEL = "ultimate render compute shader";
		const std::string ULTIMATE_SHADER_PATH = "shaders/ultimate_c.wgsl";
		wgpu::ShaderModule _computeShaderModule;

		WGPUContext* _wgpuContext;

		wgpu::ComputePipeline _computePipeline;
		wgpu::BindGroupLayout _bindGroupLayout;
		wgpu::BindGroup _bindGroup;

		wgpu::PipelineLayout getPipelineLayout();
		void createBindGroupLayout(
			wgpu::TextureFormat ultimateTextureFormat,
			wgpu::TextureFormat baseColorTextureFormat,
			wgpu::TextureFormat lightingTextureFormat,
			wgpu::TextureFormat shadowTextureFormat
		);
		void createPipeline();
		void createBindGroup(
			wgpu::TextureView& ultimateTextureView,
			wgpu::TextureView& baseColorTextureView,
			wgpu::TextureView& lightingTextureView,
			wgpu::TextureView& shadowTextureView
		);
	};
}

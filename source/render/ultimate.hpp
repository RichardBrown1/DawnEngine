#pragma once
#include <vector>
#include <string>
#include <iostream>
#include <dawn/webgpu_cpp.h>
#include <fastgltf/types.hpp>
#include "../constants.hpp"
#include "../structs/host.hpp"
#include "../wgpuContext/wgpuContext.hpp"

namespace render {
	namespace ultimate::descriptor {

		struct GenerateGpuObjects {
			wgpu::TextureFormat baseColorTextureFormat;
			wgpu::TextureView& baseColorTextureView;
			wgpu::TextureFormat lightingTextureFormat;
			wgpu::TextureView& lightingTextureView;

			wgpu::TextureView& shadowMapTextureView;
		};

		struct DoCommands {
			wgpu::CommandEncoder& commandEncoder;
		};
	}

	class Ultimate {
	public:
		Ultimate(WGPUContext* wgpuContext);
		void generateGpuObjects(const render::ultimate::descriptor::GenerateGpuObjects* descriptor);
		void doCommands(const render::ultimate::descriptor::DoCommands* descriptor);

		wgpu::TextureView ultimateTextureView;
		wgpu::TextureFormat ultimateTextureFormat = wgpu::TextureFormat::RGBA32Float;

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
			wgpu::TextureFormat baseColorTextureFormat,
			wgpu::TextureFormat lightingTextureFormat
		);
		void createPipeline();
		void createBindGroup(
			wgpu::TextureView& baseColorTextureView,
			wgpu::TextureView& lightingTextureView
		);
	};
}

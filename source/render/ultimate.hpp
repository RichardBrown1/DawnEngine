#pragma once
#include <vector>
#include <string>
#include <iostream>
#include <webgpu/webgpu_cpp.h>
#include <fastgltf/types.hpp>
#include "../constants.hpp"
#include "../structs/host.hpp"

namespace render {
	namespace ultimate::descriptor {

		struct GenerateGpuObjects {
			wgpu::Extent2D screenDimensions;
			wgpu::TextureFormat baseColorTextureFormat;
			wgpu::TextureView& baseColorTextureView;
			wgpu::TextureView& shadowMapTextureView;
		};

		struct DoCommands {
			wgpu::CommandEncoder& commandEncoder;
		};
	}

	class Ultimate {
	public:
		Ultimate(wgpu::Device* device);
		void generateGpuObjects(const render::ultimate::descriptor::GenerateGpuObjects* descriptor);
		void doCommands(const render::ultimate::descriptor::DoCommands* descriptor);

		wgpu::TextureView ultimateTextureView;
		wgpu::TextureFormat ultimateTextureFormat = wgpu::TextureFormat::RGBA32Float;

	private:
		const std::string _displayTextureViewLabel = "display ";

		const wgpu::StringView ULTIMATE_SHADER_LABEL = "ultimate render compute shader";
		const std::string ULTIMATE_SHADER_PATH = "shaders/ultimate_c.wgsl";
		wgpu::ShaderModule _computeShaderModule;

		wgpu::Device* _device;
		wgpu::Extent2D _screenDimensions;

		wgpu::ComputePipeline _computePipeline;
		wgpu::BindGroupLayout _bindGroupLayout;
		wgpu::BindGroup _bindGroup;

		wgpu::PipelineLayout getPipelineLayout();
		void createBindGroupLayout(wgpu::TextureFormat baseColorTextureFormat);
		void createPipeline();
		void createBindGroup(wgpu::TextureView& baseColorTextureView);
	};
}

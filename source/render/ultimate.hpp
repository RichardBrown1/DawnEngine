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
			wgpu::TextureFormat surfaceTextureFormat;
			wgpu::TextureView& baseColorTextureView;
			wgpu::TextureView& shadowMapTextureView;
		};

		struct DoCommands {
			wgpu::CommandEncoder& commandEncoder;
			wgpu::Buffer& vertexBuffer;
			wgpu::Buffer& indexBuffer;
			std::vector<structs::host::DrawCall>& drawCalls;
		};
	}

	class Ultimate {
	public:
		Ultimate(wgpu::Device* device);
		void generateGpuObjects(const render::ultimate::descriptor::GenerateGpuObjects* descriptor);
		void doCommands(const render::ultimate::descriptor::DoCommands* descriptor);


	private:
		const std::string _displayTextureViewLabel = "display ";

		const wgpu::StringView ULTIMATE_SHADER_LABEL = "ultimate render compute shader";
		const std::string ULTIMATE_SHADER_PATH = "shaders/ultimate_c.spv";
		wgpu::ShaderModule _computeShaderModule;

		wgpu::Device* _device;
		wgpu::Extent2D _screenDimensions;
		wgpu::TextureFormat _surfaceTextureFormat;	

		wgpu::ComputePipeline _computePipeline;
		wgpu::BindGroupLayout _inputBindGroupLayout;
		wgpu::BindGroupLayout _outputBindGroupLayout;
		wgpu::BindGroup _inputBindGroup;
		wgpu::BindGroup _outputBindGroup;

		wgpu::TextureView _displayTextureView;

		wgpu::PipelineLayout getPipelineLayout();
		void createInputBindGroupLayout();
		void createOutputBindGroupLayout();
		void createPipeline();
		void createInputBindGroup(
			wgpu::TextureView& baseColorTextureView,
			wgpu::TextureView& shadowMapTextureView
			);
		void createOutputBindGroup(
			wgpu::TextureView& surfaceTextureView
			);


	};
}

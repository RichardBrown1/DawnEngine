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
	namespace initial::descriptor {
		struct GenerateGpuObjects {
			wgpu::Buffer& cameraBuffer;
			wgpu::Buffer& transformBuffer;
			wgpu::Buffer& instancePropertiesBuffer;
			wgpu::Buffer& materialBuffer;
		};

		struct DoCommands {
			wgpu::CommandEncoder& commandEncoder;
			wgpu::Buffer& vertexBuffer;
			wgpu::Buffer& indexBuffer;
			std::vector<structs::host::DrawCall>& drawCalls;
		};
	}

	class Initial {
	public:
		Initial(WGPUContext* wgpuContext);
		void generateGpuObjects(const render::initial::descriptor::GenerateGpuObjects* descriptor);
		void doCommands(const render::initial::descriptor::DoCommands* descriptor);

	private:
		const wgpu::StringView VERTEX_SHADER_LABEL = "initial render vertex shader";
		const std::string VERTEX_SHADER_PATH = "shaders/initialRender_vf.wgsl";

		const wgpu::StringView FRAGMENT_SHADER_LABEL = "initial render fragment shader";
		const std::string FRAGMENT_SHADER_PATH = "shaders/initialRender_vf.wgsl";


		WGPUContext* _wgpuContext;

		wgpu::RenderPipeline _renderPipeline;
		wgpu::BindGroupLayout _bindGroupLayout;
		wgpu::BindGroup _bindGroup;

		wgpu::ShaderModule _vertexShaderModule;
		wgpu::ShaderModule _fragmentShaderModule;

		std::array<wgpu::RenderPassColorAttachment, 4> _renderPassColorAttachments;

		wgpu::PipelineLayout getPipelineLayout();
		void createBindGroupLayout();
		void createPipeline();
		void createBindGroup(
			const wgpu::Buffer& cameraBuffer,
			const wgpu::Buffer& transformBuffer,
			const wgpu::Buffer& instancePropertiesBuffer,
			const wgpu::Buffer& materialBuffer
		);
	};
}
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
	namespace initial::descriptor {
		struct DoCommands {
			wgpu::CommandEncoder& commandEncoder;
			wgpu::Buffer& vertexBuffer;
			wgpu::Buffer& indexBuffer;
			std::vector<structs::host::DrawCall>& drawCalls;
			wgpu::TextureView& depthTextureView;
		};
	}

	class Initial {
	public:
		Initial(WGPUContext* wgpuContext);
		void generateGpuObjects(const DeviceResources* deviceResources);
		void doCommands(const render::initial::descriptor::DoCommands* descriptor);

	private:
		WGPUContext* _wgpuContext;

		const wgpu::StringView VERTEX_SHADER_LABEL = "initial render vertex shader";
		const std::string VERTEX_SHADER_PATH = "shaders/initialRender_v.wgsl";
		wgpu::ShaderModule _vertexShaderModule;

		const wgpu::StringView ONE_FRAGMENT_SHADER_LABEL = "initial render fragment shader ONE";
		const std::string ONE_FRAGMENT_SHADER_PATH = "shaders/initialRenderOne_f.wgsl";
		wgpu::ShaderModule _oneFragmentShaderModule;
		
		const wgpu::StringView TWO_FRAGMENT_SHADER_LABEL = "initial render fragment shader TWO";
		const std::string TWO_FRAGMENT_SHADER_PATH = "shaders/initialRenderTwo_f.wgsl";
		wgpu::ShaderModule _twoFragmentShaderModule;

		const wgpu::StringView THREE_FRAGMENT_SHADER_LABEL = "initial render fragment shader THREE";
		const std::string THREE_FRAGMENT_SHADER_PATH = "shaders/initialRenderThree_f.wgsl";
		wgpu::ShaderModule _threeFragmentShaderModule;

		wgpu::RenderPipeline _renderPipelineOne;
		wgpu::RenderPipeline _renderPipelineTwo;
		wgpu::RenderPipeline _renderPipelineThree;

		wgpu::BindGroupLayout _inputBindGroupLayout;
		wgpu::BindGroup _inputBindGroup;

		wgpu::ShaderModule _baseColorTexCoordsFragmentShaderModule;
		wgpu::ShaderModule _worldNormalFragmentShaderModule;

		std::array<wgpu::RenderPassColorAttachment, 0> _renderPassColorAttachments;

		wgpu::PipelineLayout getPipelineLayout();
		void createInputBindGroupLayout();
		void createPipelines(const DeviceResources* deviceResources);
		void createInputBindGroup(const DeviceResources* deviceResources);
	};
}
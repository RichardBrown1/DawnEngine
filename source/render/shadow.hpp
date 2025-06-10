#pragma once
#include <dawn/webgpu_cpp.h>
#include <vector>
#include <string>
#include "../structs/host.hpp"
#include "../wgpuContext/wgpuContext.hpp"

namespace render {
	namespace shadow {
		namespace descriptor {
			struct GenerateGpuObjects {
				wgpu::Buffer& transformBuffer;
				wgpu::Buffer& lightBuffer;
			};

			struct DoCommands {
				wgpu::CommandEncoder& commandEncoder;
				wgpu::Buffer& vertexBuffer;
				wgpu::Buffer& indexBuffer;
				std::vector<structs::host::DrawCall>& drawCalls;
			};
		}
	}

	class Shadow {
	public:
		Shadow(WGPUContext* wgpuContext);
		void generateGpuObjects(const render::shadow::descriptor::GenerateGpuObjects* descriptor);
		void doCommands(const render::shadow::descriptor::DoCommands* descriptor);

		wgpu::TextureView shadowMapTextureView;
	private:
		const wgpu::StringView VERTEX_SHADER_LABEL = "shadow render vertex shader";
		const std::string VERTEX_SHADER_PATH = "shaders/shadowMap_v.spv";

		const wgpu::StringView FRAGMENT_SHADER_LABEL = "shadow render fragment shader";
		const std::string FRAGMENT_SHADER_PATH = "shaders/shadowMap_f.spv";

		WGPUContext* _wgpuContext;
		wgpu::Extent2D _shadowDimensions = wgpu::Extent2D{ 2048, 2048 };
		wgpu::TextureUsage _shadowTextureUsage = wgpu::TextureUsage::RenderAttachment;

		wgpu::RenderPipeline _renderPipeline;
		wgpu::BindGroupLayout _bindGroupLayout;
		wgpu::BindGroup _bindGroup;

		wgpu::ShaderModule _vertexShaderModule;
		wgpu::ShaderModule _fragmentShaderModule;

		wgpu::PipelineLayout getPipelineLayout();
		void createBindGroupLayout();
		void createPipeline();
		void createBindGroup(wgpu::Buffer& transformBuffer, wgpu::Buffer& lightBuffer);
		void createShadowMapTextureView();
	};
}
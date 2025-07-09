#pragma once
#include <dawn/webgpu_cpp.h>
#include <vector>
#include <string>
#include "../structs/host.hpp"
#include "../wgpuContext/wgpuContext.hpp"
#include "../device/resources.hpp"

namespace render {
	namespace shadowMap {
		namespace descriptor {
			struct GenerateGpuObjects {
				wgpu::Buffer& transformBuffer;
				std::vector<wgpu::Buffer>& lightBuffers;
			};

			struct DoCommands {
				wgpu::CommandEncoder& commandEncoder;
				wgpu::Buffer& vertexBuffer;
				wgpu::Buffer& indexBuffer;
				std::vector<structs::host::DrawCall>& drawCalls;
				std::vector<wgpu::TextureView>& shadowMapTextureViews;
			};
		}
	}

	class ShadowMap {
	public:
		ShadowMap(WGPUContext* wgpuContext);
		void generateGpuObjects(const DeviceResources* deviceResources);
		void doCommands(const render::shadowMap::descriptor::DoCommands* descriptor);

	private:
		const wgpu::StringView VERTEX_SHADER_LABEL = "shadow render vertex shader";
		const std::string VERTEX_SHADER_PATH = "shaders/shadowMap_v.spv";

		const wgpu::StringView FRAGMENT_SHADER_LABEL = "shadow render fragment shader";
		const std::string FRAGMENT_SHADER_PATH = "shaders/shadowMap_f.spv";

		WGPUContext* _wgpuContext;
		uint32_t _shadowMapCount = 0;

		wgpu::RenderPipeline _renderPipeline;
		wgpu::BindGroupLayout _transformBindGroupLayout;
		wgpu::BindGroupLayout _lightBindGroupLayout;
		wgpu::BindGroup _transformBindGroup;
		std::vector<wgpu::BindGroup> _lightBindGroups;

		wgpu::ShaderModule _vertexShaderModule;
		wgpu::ShaderModule _fragmentShaderModule;

		wgpu::PipelineLayout getPipelineLayout();
		void createTransformBindGroupLayout();
		void createLightBindGroupLayout();
		void createPipeline();
		void createTransformBindGroup(
			const wgpu::Buffer& transformBuffer
		);
		void insertLightBindGroup(
			const wgpu::Buffer& lightBuffer
		);
	};
}
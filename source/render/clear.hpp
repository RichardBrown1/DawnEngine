#pragma once
#include <vector>
#include <string>
#include <dawn/webgpu_cpp.h>
#include "../structs/structs.hpp"
#include "../wgpuContext/wgpuContext.hpp"

namespace render {
	namespace clear::descriptor {

		struct GenerateGpuObjects {
			wgpu::TextureFormat textureFormat;
			wgpu::TextureView& textureView;
		};

		struct DoCommands {
			wgpu::CommandEncoder& commandEncoder;
		};
	}

	class Clear {
	public:
		Clear(WGPUContext* wgpuContext);
		void generateGpuObjects(const render::clear::descriptor::GenerateGpuObjects* descriptor);
		void doCommands(const render::clear::descriptor::DoCommands* descriptor);

	private:
		const std::string _accumulatorTextureViewLabel = "light accumulator ";

		const wgpu::StringView CLEAR_SHADER_LABEL = "clear render compute shader";
		const std::string CLEAR_SHADER_PATH = "shaders/clear_c.wgsl";
		wgpu::ShaderModule _computeShaderModule;

		WGPUContext* _wgpuContext;

		wgpu::ComputePipeline _computePipeline;
		wgpu::BindGroupLayout _bindGroupLayout;
		wgpu::BindGroup _bindGroup;

		wgpu::PipelineLayout getPipelineLayout();
		void createBindGroupLayout(
			const wgpu::TextureFormat textureFormat
		);
		void createComputePipeline();

		void createBindGroup(
			const wgpu::TextureView& textureView
		);
	};
}

#pragma once
#include <vector>
#include <dawn/webgpu_cpp.h>
#include "../wgpuContext/wgpuContext.hpp"
#include "../device/resources.hpp"

namespace render {
	namespace unpack::descriptor {
		struct DoCommands {
			wgpu::CommandEncoder& commandEncoder;
		};
	}

	class Unpack {
	public:
		Unpack(WGPUContext* wgpuContext);
		void generateGpuObjects(const DeviceResources* p_deviceResources);
		void doCommands(const render::unpack::descriptor::DoCommands* descriptor);

	private:
		const std::string _accumulatorTextureViewLabel = "light accumulator ";

		const wgpu::StringView UNPACK_SHADER_LABEL = "unpack render compute shader";
		const std::string UNPACK_SHADER_PATH = "shaders/unpack_c.wgsl";
		wgpu::ShaderModule _computeShaderModule;

		WGPUContext* _wgpuContext;

		wgpu::ComputePipeline _computePipeline;
		wgpu::BindGroupLayout _bindGroupLayout;
		wgpu::BindGroup _bindGroup;

		wgpu::PipelineLayout getPipelineLayout();
		void createBindGroupLayout(const DeviceResources* p_deviceResources);
		void createComputePipeline();

		void createBindGroup(const DeviceResources* p_deviceResources);
	};
}

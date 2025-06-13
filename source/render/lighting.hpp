#pragma once
#include <vector>
#include <string>
#include <dawn/webgpu_cpp.h>
#include "../structs/structs.hpp"
#include "../wgpuContext/wgpuContext.hpp"

namespace render {
	namespace lighting::descriptor {

		struct GenerateGpuObjects {
			wgpu::TextureFormat worldTextureFormat;
			wgpu::TextureView& worldTextureView;
			wgpu::TextureFormat normalTextureFormat;
			wgpu::TextureView& normalTextureView;

			std::vector<wgpu::Buffer> lightBuffers;
		};

		struct DoCommands {
			wgpu::CommandEncoder& commandEncoder;
		};
	}

	class Lighting {
	public:
		Lighting(WGPUContext* wgpuContext);
		void generateGpuObjects(const render::lighting::descriptor::GenerateGpuObjects* descriptor);
		void doCommands(const render::lighting::descriptor::DoCommands* descriptor);

		wgpu::TextureView lightingTextureView;
		wgpu::TextureFormat lightingTextureFormat = wgpu::TextureFormat::R32Uint;

	private:
		const std::string _accumulatorTextureViewLabel = "light accumulator ";

		const wgpu::StringView LIGHTING_SHADER_LABEL = "lighting render compute shader";
		const std::string LIGHTING_SHADER_PATH = "shaders/lighting_c.wgsl";
		wgpu::ShaderModule _computeShaderModule;

		WGPUContext* _wgpuContext;

		wgpu::ComputePipeline _computePipeline;
		wgpu::BindGroupLayout _accumulatorBindGroupLayout;
		wgpu::BindGroupLayout _inputBindGroupLayout;
		wgpu::BindGroup _accumulatorBindGroup;
		std::vector<wgpu::BindGroup> _inputBindGroups;

		wgpu::PipelineLayout getPipelineLayout();
		void createAccumulatorBindGroupLayout(
			const wgpu::TextureFormat worldPositionTextureFormat,
			const wgpu::TextureFormat normalTextureFormat);
		void createInputBindGroupLayout();
		void createComputePipeline();

		void createAccumulatorBindGroup(
			const wgpu::TextureView& worldPositionTextureView,
			const wgpu::TextureView& normalTextureView
		);
		void insertInputBindGroup(
			const wgpu::Buffer& lightBuffer
		);

	};
}

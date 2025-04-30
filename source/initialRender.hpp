#include <vector>
#include <string>
#include <iostream>
#include <webgpu/webgpu_cpp.h>
#include <fastgltf/types.hpp>
#include "constants.hpp"
#include "structs.hpp"

namespace DawnEngine {

	struct InitialRenderDescriptor {
		wgpu::Device device;
	};

	struct GenerateGpuObjectsDescriptor {
		InitialRenderCreateBindGroupDescriptor initialRenderCreateBindGroupDescriptor;
	};

	struct InitialRenderCreateBindGroupDescriptor {
		wgpu::Buffer cameraBuffer;
		wgpu::Buffer transformsBuffer;
		wgpu::Buffer instancePropertiesBuffer;
		wgpu::Buffer materialsBuffer;
	};

	struct DoInitialRenderCommandsDescriptor {
		wgpu::CommandEncoder commandEncoder;
		wgpu::Buffer vertexBuffer;
		wgpu::Buffer indexBuffer;
	};

	class InitialRender {
	public:
		InitialRender(const InitialRenderDescriptor* descriptor);
		void generateGpuObjects(const GenerateGpuObjectsDescriptor* descriptor);
		void doCommands(const DoInitialRenderCommandsDescriptor* descriptor);

	private:
		const wgpu::StringView VERTEX_SHADER_LABEL = "initial render vertex shader";
		const std::string VERTEX_SHADER_PATH = "shaders/v_initialRender.spv";

		const wgpu::StringView VERTEX_SHADER_LABEL = "initial render fragment shader";
		const std::string INITIAL_SHADER_PATH = "shaders/f_initialRener.spv";

		wgpu::Device _device;
		wgpu::Queue _queue;

		wgpu::RenderPipeline _renderPipeline;
		wgpu::BindGroupLayout _bindGroupLayout;
		wgpu::BindGroup _bindGroup;

		wgpu::ShaderModule _vertexShaderModule;
		wgpu::ShaderModule _fragmentShaderModule;

		wgpu::TextureFormat _baseColorAccumulatorTextureFormat;

		wgpu::PipelineLayout getPipelineLayout();
		void createBindGroupLayout();
		void createPipeline();
		void createBindGroup(const InitialRenderCreateBindGroupDescriptor* descriptor);
	};
}
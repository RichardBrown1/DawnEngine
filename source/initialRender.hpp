#include <vector>
#include <webgpu/webgpu_cpp.h>
#include <fastgltf/types.hpp>
#include "constants.hpp"
#include "structs.hpp"

namespace DawnEngine {

	struct InitialRenderDescriptor {
		wgpu::Device device;
	};

	struct DoInitialRenderCommandsDescriptor {
		wgpu::CommandEncoder commandEncoder;
	};

	class InitialRender {
	public:
		InitialRender(const InitialRenderDescriptor* descriptor);
		wgpu::RenderPipeline createPipeline();
		void doCommands(const DoInitialRenderCommandsDescriptor* descriptor);

	private:
		const wgpu::StringView VERTEX_SHADER_LABEL = "initial render vertex shader";
		const std::string INITIAL_RENDER_SHADER_PATH = "shaders/v_initialRender.spv";

		const wgpu::StringView VERTEX_SHADER_LABEL = "initial render fragment shader";
		const std::string INITIAL_RENDER_SHADER_PATH = "shaders/f_initialRener.spv";

		wgpu::Device _device;
		wgpu::Queue _queue;

		wgpu::ShaderModule _initialRenderVertexShaderModule;
		wgpu::ShaderModule _initialRenderFragmentShaderModule;
		wgpu::TextureFormat _baseColorAccumulatorTextureFormat;

		wgpu::PipelineLayout getPipelineLayout();
	};
}
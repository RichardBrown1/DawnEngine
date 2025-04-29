#include "initialRender.hpp"

namespace DawnEngine {
	InitialRender::InitialRender(const InitialRenderDescriptor* descriptor) {
		_device = descriptor.device;
		_queue = descriptor.queue;
		
		_initialRenderVertexShaderModule = Utilities::createShaderModule(
			_device,
			VERTEX_SHADER_LABEL,
			INITIAL_RENDER_SHADER_PATH,
		);
	}
}
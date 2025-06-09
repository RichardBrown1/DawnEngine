#pragma once
#include "base.hpp"

namespace render {
	class FourChannel : public BaseAccumulator<FourChannel> {
	public:
		FourChannel(WGPUContext* wgpuContext) : BaseAccumulator(wgpuContext) {
			computeShaderModule = device::createWGSLShaderModule(
				wgpuContext->device,
				ACCUMULATOR_SHADER_LABEL,
				ACCUMULATOR_SHADER_PATH
			);
		};

		wgpu::ShaderModule computeShaderModule;

	private:
		const wgpu::StringView ACCUMULATOR_SHADER_LABEL = "four channel accumulator shader";
		const std::string ACCUMULATOR_SHADER_PATH = "shaders/fourChannelAccumulator_c.wgsl";

	};
}
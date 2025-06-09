#pragma once
#include "base.hpp"

namespace render {
	class FourChannel : public BaseAccumulator<FourChannel> {
	public:
		// Constructor for FourChannel that calls the base constructor
		FourChannel(WGPUContext* wgpuContext) : BaseAccumulator(wgpuContext) {};

		// The friend declaration for BaseAccumulator is not needed here
		// friend BaseAccumulator; // Remove this line if it's not used elsewhere

		const wgpu::StringView ACCUMULATOR_SHADER_LABEL = "four channel accumulator shader";
		const std::string ACCUMULATOR_SHADER_PATH = "shaders/fourChannelAccumulator_c.wgsl";

		// Make these methods public and accessible
		wgpu::StringView getAccumulatorShaderLabel() {
			return ACCUMULATOR_SHADER_LABEL;
		}
		std::string getAccumulatorShaderPath() {
			return ACCUMULATOR_SHADER_PATH;
		}
	};
}
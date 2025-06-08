#pragma once
#include "base.hpp"

namespace render {
	class FourChannel : public BaseAccumulator<FourChannel> {
		friend BaseAccumulator;
	protected:
		const wgpu::StringView ACCUMULATOR_SHADER_LABEL = "four channel accumulator shader";
		const std::string ACCUMULATOR_SHADER_PATH = "shaders/fourChannelAccumulator_c.wgsl";

	};
}
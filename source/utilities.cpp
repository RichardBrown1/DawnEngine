#include "../include/utilities.hpp"
#include <iostream>
#include <fstream>

namespace Utilities {

	void checkFastGltfError(const fastgltf::Error& error, const std::string& additionalMessage) {
		if (error == fastgltf::Error::None) {
			return;
		}
		throw std::runtime_error(
			std::format(" \
				fastgltf error name: {} \r\n \
				additionalMessage: {} \r\n \
				errorMessage: {} \r\n",
				getErrorName(error),
				additionalMessage,
				getErrorMessage(error)
			)
		);
	};

	glm::f32mat4x4 toGlmFormat(fastgltf::math::fmat4x4& matrix) {
		glm::f32mat4x4& output = reinterpret_cast<glm::f32mat4x4&>(matrix);
		//	memcpy(&output, &matrix, sizeof(glm::f32mat4x4));
		return output;
	};
};


namespace {
	std::vector<uint32_t> readShader(const std::string& filename) {
		std::ifstream file(filename, std::ios::binary | std::ios::ate);
		if (!file.is_open()) {
			throw std::runtime_error("Failed to open SPIR-V Shader file.");
		}

		size_t fileSize = file.tellg();
		file.seekg(0, std::ios::beg);

		std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));
		file.read(reinterpret_cast<char*>(buffer.data()), fileSize);
		file.close();

		return buffer;
	}
};

namespace DawnEngine {
	wgpu::ShaderModule DawnEngine::createShaderModule(wgpu::Device& device, const wgpu::StringView& label, const std::string& filename)
	{
		const std::vector<uint32_t> shaderCode = readShader(std::string(filename));
		wgpu::ShaderSourceSPIRV shaderSource = wgpu::ShaderSourceSPIRV();
		shaderSource.codeSize = static_cast<uint32_t>(shaderCode.size());
		shaderSource.code = shaderCode.data();
		const wgpu::ShaderModuleDescriptor shaderModuleDescriptor = {
			.nextInChain = &shaderSource,
			.label = label,
		};
		return device.CreateShaderModule(&shaderModuleDescriptor);
	}
};

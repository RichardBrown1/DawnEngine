#include "../include/utilities.hpp"
#include <iostream>
#include <fstream>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

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

	DawnEngine::Camera getDefaultCamera(wgpu::SurfaceConfiguration surfaceConfiguration)
	{
		constexpr auto eye = glm::vec3(0.0, 0.0, -10.0f);
		constexpr auto origin = glm::vec3(0.0, 0.0, 0.0);
		constexpr auto up = glm::vec3(0.0, 1.0, 0.0);
		return DawnEngine::Camera{
			.projection = glm::perspectiveRH_ZO(45.0f, surfaceConfiguration.width / (float)surfaceConfiguration.height, 0.1f, 1024.0f),
			.view = glm::lookAt(eye, origin, up),
		};
	}
};

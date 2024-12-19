#include "../include/utilities.hpp"
#include <iostream>
#include <fstream>

namespace Utilities {
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
	}

	glm::f32mat4x4 convertFastGltfToGlm(fastgltf::math::fmat4x4& matrix) {
		glm::f32mat4x4 output;

		output[0][0] = matrix[0][0];
		output[0][1] = matrix[0][1];
		output[0][2] = matrix[0][2];
		output[0][3] = matrix[0][3];

		output[1][0] = matrix[1][0];
		output[1][1] = matrix[1][1];
		output[1][2] = matrix[1][2];
		output[1][3] = matrix[1][3];

		output[2][0] = matrix[2][0];
		output[2][1] = matrix[2][1];
		output[2][2] = matrix[2][2];
		output[2][3] = matrix[2][3];

		output[3][0] = matrix[3][0];
		output[3][1] = matrix[3][1];
		output[3][2] = matrix[3][2];
		output[3][3] = matrix[3][3];

		return output;
	}

};
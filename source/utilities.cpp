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

};
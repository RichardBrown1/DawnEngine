#include "../include/utilities.hpp"
#include <iostream>
#include <fstream>

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
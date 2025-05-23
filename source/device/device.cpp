#pragma once
#include <iostream>
#include <string>
#include <fstream>
#include <absl/log/log.h>
#include "device.hpp"


namespace {
	std::vector<uint32_t> readShader(const std::string& filename) {
			std::ifstream file(filename, std::ios::binary | std::ios::ate);
			if (!file.is_open()) {
				LOG(ERROR) << "Failed to open SPIR-V Shader file at: " << filename;
			}

			size_t fileSize = file.tellg();
			file.seekg(0, std::ios::beg);

			std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));
			file.read(reinterpret_cast<char*>(buffer.data()), fileSize);
			file.close();

			return buffer;
		}

	std::string readShaderToString(const std::string& filename) {
			std::ifstream file(filename, std::ios::binary | std::ios::ate);
			if (!file.is_open()) {
				LOG(ERROR) << "Failed to open WGSL Shader file at: " << filename;
			}

			size_t fileSize = file.tellg();
			file.seekg(0, std::ios::beg);

			std::string string = std::string(fileSize, ' ');
			file.read(string.data(), fileSize);
			file.close();

			return string;
		}

}

namespace device {
	wgpu::ShaderModule createShaderModule(wgpu::Device& device, const wgpu::StringView& label, const std::string& filename)
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

	wgpu::ShaderModule createWGSLShaderModule(wgpu::Device& device, const wgpu::StringView& label, const std::string& filename)
		{
			const std::string shaderCode = readShaderToString(std::string(filename));
			wgpu::ShaderSourceWGSL shaderSource = wgpu::ShaderSourceWGSL();
			shaderSource.code = wgpu::StringView(shaderCode);
			const wgpu::ShaderModuleDescriptor shaderModuleDescriptor = {
				.nextInChain = &shaderSource,
				.label = label,
			};
			return device.CreateShaderModule(&shaderModuleDescriptor);
		}
}
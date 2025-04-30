#pragma once
#include <vector>
#include <string>
#include <iostream>
#include <webgpu/webgpu_cpp.h>
#include <fastgltf/types.hpp>
#include "constants.hpp"
#include "structs.hpp"
#include "utilities.hpp"

namespace DawnEngine {

	struct InitialRenderCreateBindGroupDescriptor {
		wgpu::Buffer& cameraBuffer;
		wgpu::Buffer& transformsBuffer;
		wgpu::Buffer& instancePropertiesBuffer;
		wgpu::Buffer& materialsBuffer;
	};

	struct GenerateGpuObjectsDescriptor {
		InitialRenderCreateBindGroupDescriptor initialRenderCreateBindGroupDescriptor;
	};

	struct DoInitialRenderCommandsDescriptor {
		wgpu::CommandEncoder commandEncoder;
		wgpu::Buffer vertexBuffer;
		wgpu::Buffer indexBuffer;
	};

	class InitialRender {
	public:
		InitialRender() = delete;
		InitialRender(wgpu::Device* device);
		void generateGpuObjects(const GenerateGpuObjectsDescriptor* descriptor);
		void doCommands(const DoInitialRenderCommandsDescriptor* descriptor);

		//TODO Other Texture Formats for other texture accumulators
		const wgpu::TextureFormat baseColorAccumulatorTextureFormat = wgpu::TextureFormat::RGBA32Float;
		//const wgpu::TextureFormat metallicRoughnessAccumulatorTextureFormat = wgpu::TextureFormat::RGBA32Float;

	private:
		static DawnEngine::InitialRender *instance;
		const wgpu::StringView VERTEX_SHADER_LABEL = "initial render vertex shader";
		const std::string VERTEX_SHADER_PATH = "shaders/v_initialRender.spv";

		const wgpu::StringView FRAGMENT_SHADER_LABEL = "initial render fragment shader";
		const std::string FRAGMENT_SHADER_PATH = "shaders/f_initialRener.spv";

		wgpu::Device* _device;

		wgpu::RenderPipeline _renderPipeline;
		wgpu::BindGroupLayout _bindGroupLayout;
		wgpu::BindGroup _bindGroup;

		wgpu::ShaderModule _vertexShaderModule;
		wgpu::ShaderModule _fragmentShaderModule;


		wgpu::PipelineLayout getPipelineLayout();
		void createBindGroupLayout();
		void createPipeline();
		void createBindGroup(const InitialRenderCreateBindGroupDescriptor* descriptor);
	};
}
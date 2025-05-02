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
		wgpu::Buffer& transformBuffer;
		wgpu::Buffer& instancePropertiesBuffer;
		wgpu::Buffer& materialBuffer;
	};

	struct GenerateGpuObjectsDescriptor {
		InitialRenderCreateBindGroupDescriptor initialRenderCreateBindGroupDescriptor;
		wgpu::Extent2D screenDimensions;
	};

	struct DoInitialRenderCommandsDescriptor {
		wgpu::CommandEncoder& commandEncoder;
		wgpu::Buffer& vertexBuffer;
		wgpu::Buffer& indexBuffer;
		std::vector<DawnEngine::DrawInfo>& drawCalls;
	};

	class InitialRender {
	public:
		InitialRender(wgpu::Device* device);
		void generateGpuObjects(const GenerateGpuObjectsDescriptor* descriptor);
		void doCommands(const DoInitialRenderCommandsDescriptor* descriptor);

		const wgpu::TextureFormat masterInfoTextureFormat = wgpu::TextureFormat::RGBA32Float;
		const wgpu::TextureFormat baseColorAccumulatorTextureFormat = wgpu::TextureFormat::BGRA8Unorm;
		//const wgpu::TextureFormat metallicRoughnessAccumulatorTextureFormat = wgpu::TextureFormat::RGBA32Float;

	private:
		const wgpu::StringView VERTEX_SHADER_LABEL = "initial render vertex shader";
		const std::string VERTEX_SHADER_PATH = "shaders/initialRender_v.spv";

		const wgpu::StringView FRAGMENT_SHADER_LABEL = "initial render fragment shader";
		const std::string FRAGMENT_SHADER_PATH = "shaders/initialRender_f.spv";

		wgpu::Device *_device;
		wgpu::RenderPipeline _renderPipeline;
		wgpu::BindGroupLayout _bindGroupLayout;
		wgpu::BindGroup _bindGroup;

		wgpu::ShaderModule _vertexShaderModule;
		wgpu::ShaderModule _fragmentShaderModule;

		wgpu::TextureView _masterInfoTextureView;
		wgpu::TextureView _baseColorAccumulatorTextureView;

		std::array<wgpu::RenderPassColorAttachment, 2> _renderPassColorAttachments;

		wgpu::PipelineLayout getPipelineLayout();
		void createBindGroupLayout();
		void createPipeline();
		void createBindGroup(const InitialRenderCreateBindGroupDescriptor* descriptor);
	};
}
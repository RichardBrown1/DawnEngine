#pragma once
#include <vector>
#include <string>
#include <iostream>
#include <webgpu/webgpu_cpp.h>
#include <fastgltf/types.hpp>
#include "../constants.hpp"
#include "../structs/host.hpp"

namespace render {
	namespace toSurface::descriptor {

		struct GenerateGpuObjects {
			wgpu::Extent2D screenDimensions;
			wgpu::TextureView& ultimateTextureView;
			wgpu::TextureFormat surfaceTextureFormat;
		};

		struct DoCommands {
			wgpu::CommandEncoder& commandEncoder;
			wgpu::TextureView& surfaceTextureView;
		};
	}

	class ToSurface {
	public:
		ToSurface(wgpu::Device* device);
		void generateGpuObjects(const render::toSurface::descriptor::GenerateGpuObjects* descriptor);
		void doCommands(const render::toSurface::descriptor::DoCommands* descriptor);

	private:
		const std::string _displayTextureViewLabel = "display ";

		const wgpu::StringView VERTEX_SHADER_LABEL = "toSurface vertex render shader";
		const std::string VERTEX_SHADER_PATH = "shaders/canvas_v.spv";
		wgpu::ShaderModule _vertexShaderModule;

		const wgpu::StringView FRAGMENT_SHADER_LABEL = "toSurface fragment render shader";
		const std::string FRAGMENT_SHADER_PATH = "shaders/toSurface_f.spv";
		wgpu::ShaderModule _fragmentShaderModule;

		wgpu::Device* _device;
		wgpu::Extent2D _screenDimensions;

		wgpu::RenderPipeline _renderPipeline;
		wgpu::BindGroupLayout _bindGroupLayout;
		wgpu::BindGroup _bindGroup;

		wgpu::Sampler _ultimateSampler;

		wgpu::PipelineLayout getPipelineLayout();
		void createBindGroupLayout();
		void createPipeline(wgpu::TextureFormat surfaceTextureFormat);
		void createBindGroup(
			wgpu::TextureView& baseColorTextureView
		//	wgpu::TextureView& shadowMapTextureView
			);
	};
}

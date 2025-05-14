#pragma once
#include <vector>
#include <string>
#include <iostream>
#include <webgpu/webgpu_cpp.h>
#include <fastgltf/types.hpp>
#include "../constants.hpp"
#include "../structs/host.hpp"

namespace {
	struct CreateTextureViewDescriptor {
		std::string label;
		wgpu::TextureView& outputTextureView;
		wgpu::TextureFormat textureFormat;
	};
}

namespace render {
	namespace initial::descriptor {
		struct Buffers {
			wgpu::Buffer& cameraBuffer;
			wgpu::Buffer& transformBuffer;
			wgpu::Buffer& instancePropertiesBuffer;
			wgpu::Buffer& materialBuffer;
		};

		struct GenerateGpuObjects {
			Buffers buffers;
			wgpu::Extent2D screenDimensions;
		};

		struct DoCommands {
			wgpu::CommandEncoder& commandEncoder;
			wgpu::Buffer& vertexBuffer;
			wgpu::Buffer& indexBuffer;
			std::vector<structs::host::DrawCall>& drawCalls;
		};
	}

class Initial {
	public:
		Initial(wgpu::Device* device);
		void generateGpuObjects(const render::initial::descriptor::GenerateGpuObjects* descriptor);
		void doCommands(const render::initial::descriptor::DoCommands* descriptor);

		const wgpu::TextureFormat masterInfoTextureFormat = wgpu::TextureFormat::RGBA32Float;
		const wgpu::TextureFormat baseColorTextureFormat = wgpu::TextureFormat::BGRA8Unorm;
		//TODO normal texture can be a RG format or bitpacked even. You can derive 3 coordinates from 2.
		const wgpu::TextureFormat normalTextureFormat = wgpu::TextureFormat::RGBA16Float;
		const wgpu::TextureFormat depthTextureFormat = wgpu::TextureFormat::Depth32Float;
		//const wgpu::TextureFormat metallicRoughnessAccumulatorTextureFormat = wgpu::TextureFormat::RGBA32Float;

	private:
		const wgpu::StringView VERTEX_SHADER_LABEL = "initial render vertex shader";
		const std::string VERTEX_SHADER_PATH = "shaders/initialRender_v.spv";

		const wgpu::StringView FRAGMENT_SHADER_LABEL = "initial render fragment shader";
		const std::string FRAGMENT_SHADER_PATH = "shaders/initialRender_f.spv";

		wgpu::Device* _device;
		wgpu::Extent2D _screenDimensions;

		wgpu::RenderPipeline _renderPipeline;
		wgpu::BindGroupLayout _bindGroupLayout;
		wgpu::BindGroup _bindGroup;

		wgpu::ShaderModule _vertexShaderModule;
		wgpu::ShaderModule _fragmentShaderModule;

		const std::string _masterInfoLabel = std::string("master info");
		wgpu::TextureView _masterInfoTextureView;

		const std::string _baseColorLabel = std::string("base color");
		wgpu::TextureView _baseColorAccumulatorTextureView;

		const std::string _normalLabel = std::string("normals ");
		wgpu::TextureView _normalAccumulatorTextureView;

		const std::string _depthTextureLabel = std::string("depth texture");
		wgpu::TextureView _depthTextureView;

		std::array<wgpu::RenderPassColorAttachment, 3> _renderPassColorAttachments;

		wgpu::PipelineLayout getPipelineLayout();
		void createBindGroupLayout();
		void createPipeline();
		void createBindGroup(const render::initial::descriptor::Buffers* descriptor);
		void createTextureView(const CreateTextureViewDescriptor* descriptor);
	};
}
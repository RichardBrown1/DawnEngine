#pragma once
#include <vector>
#include <string>
#include <iostream>
#include <webgpu/webgpu_cpp.h>
#include <fastgltf/types.hpp>
#include "../constants.hpp"
#include "../structs/host.hpp"

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

		const wgpu::TextureFormat worldPositionTextureFormat = wgpu::TextureFormat::RGBA32Float;
		const wgpu::TextureFormat baseColorTextureFormat = wgpu::TextureFormat::RGBA32Float;
		const wgpu::TextureFormat normalTextureFormat = wgpu::TextureFormat::RGBA32Float; //normal texture can tangent-ized	if I need it
		const wgpu::TextureFormat texCoordTextureFormat = wgpu::TextureFormat::RG32Float; //I would prefer unorm but I think its bugged
		const wgpu::TextureFormat baseColorTextureIdTextureFormat = wgpu::TextureFormat::R32Uint;
		const wgpu::TextureFormat normalTextureIdTextureFormat = wgpu::TextureFormat::R32Uint;
		const wgpu::TextureFormat depthTextureFormat = constants::DEPTH_FORMAT;

		wgpu::TextureView worldPositionTextureView;
		wgpu::TextureView baseColorTextureView;
		wgpu::TextureView normalTextureView;
		wgpu::TextureView texCoordTextureView;
		wgpu::TextureView baseColorTextureIdTextureView;
		wgpu::TextureView normalTextureIdTextureView;
		wgpu::TextureView depthTextureView;

	private:
		const wgpu::StringView VERTEX_SHADER_LABEL = "initial render vertex shader";
		const std::string VERTEX_SHADER_PATH = "shaders/initialRender_v.spv";

		const wgpu::StringView FRAGMENT_SHADER_LABEL = "initial render fragment shader";
		const std::string FRAGMENT_SHADER_PATH = "shaders/initialRender_f.spv";

		const std::string _worldPositionLabel = std::string("world position info");
		const std::string _baseColorLabel = std::string("base color");
		const std::string _normalLabel = std::string("normals");
		const std::string _texCoordLabel = std::string("texcoord");
		const std::string _baseColorTextureIdLabel = std::string("base color id");
		const std::string _normalTextureIdLabel = std::string("normal id");
		const std::string _depthTextureLabel = std::string("depth texture");

		const wgpu::TextureUsage _worldPositionTextureUsage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::StorageBinding;
		const wgpu::TextureUsage _baseColorTextureUsage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::StorageBinding;
		const wgpu::TextureUsage _normalTextureUsage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::StorageBinding;
		const wgpu::TextureUsage _texCoordTextureUsage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::StorageBinding;
		const wgpu::TextureUsage _baseColorTextureIdTextureUsage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::StorageBinding;
		const wgpu::TextureUsage _normalTextureIdTextureUsage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::StorageBinding;
		const wgpu::TextureUsage _depthTextureUsage = wgpu::TextureUsage::RenderAttachment;

		wgpu::Device* _device;
		wgpu::Extent2D _screenDimensions;

		wgpu::RenderPipeline _renderPipeline;
		wgpu::BindGroupLayout _bindGroupLayout;
		wgpu::BindGroup _bindGroup;

		wgpu::ShaderModule _vertexShaderModule;
		wgpu::ShaderModule _fragmentShaderModule;

		std::array<wgpu::RenderPassColorAttachment, 6> _renderPassColorAttachments;

		wgpu::PipelineLayout getPipelineLayout();
		void createBindGroupLayout();
		void createPipeline();
		void createBindGroup(const render::initial::descriptor::Buffers* descriptor);
	};
}
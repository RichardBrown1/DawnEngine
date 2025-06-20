#pragma once
#include <vector>
#include <string>
#include <iostream>
#include <dawn/webgpu_cpp.h>
#include <fastgltf/types.hpp>
#include "../constants.hpp"
#include "../structs/host.hpp"
#include "../wgpuContext/wgpuContext.hpp"

namespace render {
	namespace initial::descriptor {
		struct GenerateGpuObjects {
			wgpu::Buffer& cameraBuffer;
			wgpu::Buffer& transformBuffer;
			wgpu::Buffer& instancePropertiesBuffer;
			wgpu::Buffer& materialBuffer;
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
		Initial(WGPUContext* wgpuContext);
		void generateGpuObjects(const render::initial::descriptor::GenerateGpuObjects* descriptor);
		void doCommands(const render::initial::descriptor::DoCommands* descriptor);

		const wgpu::TextureFormat worldPositionTextureFormat = wgpu::TextureFormat::BGRA8Unorm;
		const wgpu::TextureFormat baseColorTextureFormat = wgpu::TextureFormat::BGRA8Unorm;
		const wgpu::TextureFormat normalTextureFormat = wgpu::TextureFormat::BGRA8Unorm; //normal texture can tangent-ized	if I need it
		const wgpu::TextureFormat texCoordTextureFormat = wgpu::TextureFormat::RG32Float; //I would prefer unorm but I think its bugged
		const wgpu::TextureFormat depthTextureFormat = constants::DEPTH_FORMAT;

		wgpu::TextureView worldPositionTextureView;
		wgpu::TextureView baseColorTextureView;
		wgpu::TextureView normalTextureView;
		wgpu::TextureView texCoordTextureView;
		wgpu::TextureView depthTextureView;
		wgpu::Buffer baseColorTextureIdBuffer;
	  wgpu::Buffer normalTextureIdBuffer;

	private:
		uint32_t _textureIdBufferSize = 0;
		const wgpu::StringView VERTEX_SHADER_LABEL = "initial render vertex shader";
		const std::string VERTEX_SHADER_PATH = "shaders/initialRender_vf.wgsl";

		const wgpu::StringView FRAGMENT_SHADER_LABEL = "initial render fragment shader";
		const std::string FRAGMENT_SHADER_PATH = "shaders/initialRender_vf.wgsl";

		const std::string _worldPositionLabel = std::string("world position info");
		const std::string _baseColorLabel = std::string("base color");
		const std::string _normalLabel = std::string("normals");
		const std::string _texCoordLabel = std::string("texcoord");
		const std::string _depthTextureLabel = "depth texture";
		const wgpu::StringView _baseColorTextureIdLabel = "base color id";
		const wgpu::StringView _normalTextureIdLabel = "normal id";

		const wgpu::TextureUsage _worldPositionTextureUsage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::StorageBinding;
		const wgpu::TextureUsage _baseColorTextureUsage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::StorageBinding;
		const wgpu::TextureUsage _normalTextureUsage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::StorageBinding;
		const wgpu::TextureUsage _texCoordTextureUsage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::StorageBinding;
		const wgpu::TextureUsage _depthTextureUsage = wgpu::TextureUsage::RenderAttachment;

		const wgpu::BufferUsage _baseColorTextureIdBufferUsage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Storage; 
		const wgpu::BufferUsage _normalTextureIdBufferUsage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Storage; 

		WGPUContext* _wgpuContext;

		wgpu::RenderPipeline _renderPipeline;
		wgpu::BindGroupLayout _bindGroupLayout;
		wgpu::BindGroup _bindGroup;

		wgpu::ShaderModule _vertexShaderModule;
		wgpu::ShaderModule _fragmentShaderModule;

		std::array<wgpu::RenderPassColorAttachment, 4> _renderPassColorAttachments;

		wgpu::PipelineLayout getPipelineLayout();
		void createBindGroupLayout();
		void createPipeline();
		void createBindGroup(
			const wgpu::Buffer& cameraBuffer,
			const wgpu::Buffer& transformBuffer,
			const wgpu::Buffer& instancePropertiesBuffer,
			const wgpu::Buffer& materialBuffer
		);
	};
}
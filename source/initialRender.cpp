#pragma once
#include "initialRender.hpp"
#include <array>
#include "utilities.hpp"

namespace DawnEngine {
	InitialRender::InitialRender(wgpu::Device* device) {
			_device = device;
			_screenDimensions = wgpu::Extent2D(0,0); //generateGpuObjects() will handle this
	};

	void InitialRender::generateGpuObjects(const GenerateGpuObjectsDescriptor* descriptor) {
		assert(descriptor->screenDimensions.width > 1);
		_screenDimensions = descriptor->screenDimensions;
		
		_vertexShaderModule = Utilities::createShaderModule(*_device, VERTEX_SHADER_LABEL, VERTEX_SHADER_PATH);
		_fragmentShaderModule = Utilities::createShaderModule(*_device, FRAGMENT_SHADER_LABEL, FRAGMENT_SHADER_PATH);

		createBindGroupLayout();
		createPipeline();
		createBindGroup(&descriptor->initialRenderCreateBindGroupDescriptor);
		
		CreateTextureViewDescriptor masterInfoTextureViewDescriptor = {
			.label = _masterInfoLabel,
			.outputTextureView = _masterInfoTextureView,
			.textureFormat = masterInfoTextureFormat,
		};
		createTextureView(&masterInfoTextureViewDescriptor);

		CreateTextureViewDescriptor baseColorAccumulatorTextureViewDescriptor = {
			.label = _baseColorLabel,
			.outputTextureView = _baseColorAccumulatorTextureView,
			.textureFormat = baseColorTextureFormat,
		};
		createTextureView(&baseColorAccumulatorTextureViewDescriptor);

		CreateTextureViewDescriptor normalAccumulatorTextureViewDescriptor = {
			.label = _normalLabel,
			.outputTextureView = _normalAccumulatorTextureView,
			.textureFormat = normalTextureFormat,
		};
		createTextureView(&normalAccumulatorTextureViewDescriptor);

		_renderPassColorAttachments = {
			wgpu::RenderPassColorAttachment {
				.view = _masterInfoTextureView,
				.loadOp = wgpu::LoadOp::Clear,
				.storeOp = wgpu::StoreOp::Store,
				.clearValue = wgpu::Color{0.0f, 0.0f, UINT32_MAX, 0.0f},
			},
			wgpu::RenderPassColorAttachment {
				.view = _baseColorAccumulatorTextureView,
				.loadOp = wgpu::LoadOp::Clear,
				.storeOp = wgpu::StoreOp::Store,
				.clearValue = wgpu::Color{0.0f, 0.0f, 0.0f, 0.0f},
			},
			wgpu::RenderPassColorAttachment {
				.view = _normalAccumulatorTextureView,
				.loadOp = wgpu::LoadOp::Clear,
				.storeOp = wgpu::StoreOp::Store,
				.clearValue = wgpu::Color{0.0f, 0.0f, 0.0f, 0.0f},
			},
		};

		CreateTextureViewDescriptor depthTextureViewDescriptor = {
			.label = _depthTextureLabel,
			.outputTextureView = _depthTextureView,
			.textureFormat = depthTextureFormat,
		};
		createTextureView(&depthTextureViewDescriptor);

	};

	void InitialRender::doCommands(const DoInitialRenderCommandsDescriptor* descriptor) {
		wgpu::RenderPassDepthStencilAttachment renderPassDepthStencilAttachment = {
			.view = _depthTextureView,
			.depthLoadOp = wgpu::LoadOp::Clear,
			.depthStoreOp = wgpu::StoreOp::Store,
			.depthClearValue = 1.0f,
		};

		//TODO move this to generated gpu objects
		wgpu::RenderPassDescriptor renderPassDescriptor = {
			.label = "initial render pass",
			.colorAttachmentCount = _renderPassColorAttachments.size(),
			.colorAttachments = _renderPassColorAttachments.data(),
			.depthStencilAttachment = &renderPassDepthStencilAttachment,
		};
		wgpu::RenderPassEncoder renderPassEncoder = descriptor->commandEncoder.BeginRenderPass(&renderPassDescriptor);
		renderPassEncoder.SetPipeline(_renderPipeline);
		renderPassEncoder.SetVertexBuffer(0, descriptor->vertexBuffer, 0, descriptor->vertexBuffer.GetSize());
		renderPassEncoder.SetIndexBuffer(
			descriptor->indexBuffer,
			wgpu::IndexFormat::Uint16, 
			0, 
			descriptor->indexBuffer.GetSize()
		);
		renderPassEncoder.SetBindGroup(0, _bindGroup);
		
		for (const auto& dc : descriptor->drawCalls) {
			renderPassEncoder.DrawIndexed(dc.indexCount, dc.instanceCount, dc.firstIndex, dc.baseVertex, dc.firstInstance);
		}
		renderPassEncoder.End();
	}

	void InitialRender::createPipeline() {
		const wgpu::PipelineLayout pipelineLayout = getPipelineLayout();

		constexpr wgpu::VertexAttribute positionAttribute = {
			.format = wgpu::VertexFormat::Float32x3,
			.offset = 0,
			.shaderLocation = 0,
		};
		constexpr wgpu::VertexAttribute normalAttribute = {
			.format = wgpu::VertexFormat::Float32x3,
			.offset = offsetof(DawnEngine::VBO, normal),
			.shaderLocation = 1,
		};
		constexpr wgpu::VertexAttribute texcoordAttribute = {
			.format = wgpu::VertexFormat::Float32x2,
			.offset = offsetof(DawnEngine::VBO, texcoord),
			.shaderLocation = 2,
		};
		const auto vertexAttributes = std::vector<wgpu::VertexAttribute>{ 
			positionAttribute, 
			normalAttribute, 
			texcoordAttribute
		};

		const	wgpu::VertexBufferLayout vertexBufferLayout = {
			.arrayStride = sizeof(DawnEngine::VBO),
			.attributeCount = vertexAttributes.size(),
			.attributes = vertexAttributes.data(),
		};
		const wgpu::VertexState vertexState = {
			.module = _vertexShaderModule,
			.entryPoint = DawnEngine::EntryPoint::VERTEX,
			.bufferCount = 1,
			.buffers = &vertexBufferLayout,
		};
		
		const wgpu::DepthStencilState depthStencilState = {
			.format = depthTextureFormat,
			.depthWriteEnabled = true,
			.depthCompare = wgpu::CompareFunction::Less,
		};

		const	wgpu::ColorTargetState masterInfoColorTargetState = {
			.format = masterInfoTextureFormat,
		};
		const wgpu::ColorTargetState baseColorColorTargetState = {
			.format = baseColorTextureFormat,
		};
		const wgpu::ColorTargetState normalColorTargetState = {
			.format = normalTextureFormat,
		};
		const std::array<wgpu::ColorTargetState, 3> colorTargetStates = {
			masterInfoColorTargetState,
			baseColorColorTargetState,
			normalColorTargetState,
		};
		const wgpu::FragmentState fragmentState = {
			.module = _fragmentShaderModule,
			.entryPoint = DawnEngine::EntryPoint::FRAGMENT,
			.targetCount = colorTargetStates.size(),
			.targets = colorTargetStates.data(),
		};
		const wgpu::RenderPipelineDescriptor renderPipelineDescriptor = {
			.label = "initial render pipeline",
			.layout = pipelineLayout,
			.vertex = vertexState,
			.primitive = wgpu::PrimitiveState {
				.topology = wgpu::PrimitiveTopology::TriangleList,
				.cullMode = wgpu::CullMode::None,
			},
			.depthStencil = &depthStencilState,
			.multisample = wgpu::MultisampleState {
				.count = 1,
				.mask = ~0u,
				.alphaToCoverageEnabled = false,
			},
			.fragment = &fragmentState,
		};
		_renderPipeline = _device->CreateRenderPipeline(&renderPipelineDescriptor);
	}

	void InitialRender::createBindGroup(const InitialRenderCreateBindGroupDescriptor* descriptor) {
		const wgpu::BindGroupEntry cameraBindGroupEntry = {
			.binding = 0,
			.buffer = descriptor->cameraBuffer,
			.size = descriptor->cameraBuffer.GetSize(),
		};
		const wgpu::BindGroupEntry transformsBindGroupEntry = {
			.binding = 1,
			.buffer = descriptor->transformBuffer,
			.size = descriptor->transformBuffer.GetSize(),
		};
		const wgpu::BindGroupEntry instancePropertiesBindGroupEntry = {
			.binding = 2,
			.buffer = descriptor->instancePropertiesBuffer,
			.size = descriptor->instancePropertiesBuffer.GetSize(),
		};
		const wgpu::BindGroupEntry materialsBindGroupEntry = {
			.binding = 3,
			.buffer = descriptor->materialBuffer,
			.size = descriptor->materialBuffer.GetSize(),
		};
		std::array<wgpu::BindGroupEntry, 4> bindGroupEntries = {
			cameraBindGroupEntry,
			transformsBindGroupEntry,
			instancePropertiesBindGroupEntry,
			materialsBindGroupEntry,
		};
		const wgpu::BindGroupDescriptor bindGroupDescriptor = {
			.label = "initial render group",
			.layout = _bindGroupLayout,
			.entryCount = bindGroupEntries.size(),
			.entries = bindGroupEntries.data(),
		};
		_bindGroup = _device->CreateBindGroup(&bindGroupDescriptor);
	}

	void InitialRender::createBindGroupLayout() {
		const wgpu::BindGroupLayoutEntry cameraBindGroupLayoutEntry = {
			.binding = 0,
			.visibility = wgpu::ShaderStage::Vertex,
			.buffer = {
				.type = wgpu::BufferBindingType::Uniform,
				.minBindingSize = sizeof(glm::f32mat4x4),
			},
		};
		const wgpu::BindGroupLayoutEntry transformsBindGroupLayoutEntry = {
			.binding = 1,
			.visibility = wgpu::ShaderStage::Vertex,
			.buffer = {
				.type = wgpu::BufferBindingType::ReadOnlyStorage,
				.minBindingSize = sizeof(glm::f32mat4x4),
			},
		};
		const wgpu::BindGroupLayoutEntry instancePropertiesBindGroupLayoutEntry = {
			.binding = 2,
			.visibility = wgpu::ShaderStage::Fragment,
			.buffer = {
				.type = wgpu::BufferBindingType::ReadOnlyStorage,
				.minBindingSize = sizeof(InstanceProperty),
			}
		};
		const wgpu::BindGroupLayoutEntry materialsBindGroupLayoutEntry = {
			.binding = 3,
			.visibility = wgpu::ShaderStage::Fragment,
			.buffer = {
				.type = wgpu::BufferBindingType::ReadOnlyStorage,
				.minBindingSize = sizeof(Material),
			},
		};

		std::array<wgpu::BindGroupLayoutEntry, 4> bindGroupLayoutEntries = {
			cameraBindGroupLayoutEntry,
			transformsBindGroupLayoutEntry,
			instancePropertiesBindGroupLayoutEntry,
			materialsBindGroupLayoutEntry,
		};
		const wgpu::BindGroupLayoutDescriptor bindGroupLayoutDescriptor = {
			.label = "initial render bind group layout",
			.entryCount = bindGroupLayoutEntries.size(),
			.entries = bindGroupLayoutEntries.data(),
		};
		_bindGroupLayout = _device->CreateBindGroupLayout(&bindGroupLayoutDescriptor);
	};

	wgpu::PipelineLayout InitialRender::getPipelineLayout() {

		const wgpu::PipelineLayoutDescriptor pipelineLayoutDescriptor = {
			.label = "initial render pipeline layout",
			.bindGroupLayoutCount = 1,
			.bindGroupLayouts = &_bindGroupLayout,
		};
		return _device->CreatePipelineLayout(&pipelineLayoutDescriptor);
	};

	void InitialRender::createTextureView(const CreateTextureViewDescriptor* descriptor) {
		assert(descriptor->textureFormat != wgpu::TextureFormat::Undefined);
		const wgpu::TextureDescriptor textureDescriptor = {
					.label = wgpu::StringView(descriptor->label + std::string(" texture")),
					.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding,
					.dimension = wgpu::TextureDimension::e2D,
					.size = {
						.width = _screenDimensions.width,
						.height = _screenDimensions.height,
					},
					.format = descriptor->textureFormat,
		};
		wgpu::Texture texture = _device->CreateTexture(&textureDescriptor);
		const wgpu::TextureViewDescriptor textureViewDescriptor = {
			.label = wgpu::StringView(descriptor->label + std::string(" texture view")),
			.format = textureDescriptor.format,
			.dimension = wgpu::TextureViewDimension::e2D,
			.mipLevelCount = 1,
			.arrayLayerCount = 1,
			.aspect = wgpu::TextureAspect::All,
			.usage = textureDescriptor.usage,
		};
		descriptor->outputTextureView = texture.CreateView(&textureViewDescriptor);
	}
}
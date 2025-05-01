#pragma once
#include "initialRender.hpp"
#include <array>
#include "utilities.hpp"

namespace DawnEngine {
	InitialRender::InitialRender(wgpu::Device* device) {
			_device = device;
	};

	void InitialRender::generateGpuObjects(const GenerateGpuObjectsDescriptor* descriptor) {
		_vertexShaderModule = Utilities::createShaderModule(*_device, VERTEX_SHADER_LABEL, VERTEX_SHADER_PATH);
		_fragmentShaderModule =  Utilities::createShaderModule(*_device, FRAGMENT_SHADER_LABEL, FRAGMENT_SHADER_PATH);

		createBindGroupLayout();
		createPipeline();
		createBindGroup(&descriptor->initialRenderCreateBindGroupDescriptor);

	};

	void InitialRender::doCommands(const DoInitialRenderCommandsDescriptor* descriptor) {
		wgpu::RenderPassDescriptor renderPassDescriptor = {
			.label = "initial render pass"
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
		
		constexpr wgpu::DepthStencilState depthStencilState = {
			.format = DawnEngine::DEPTH_FORMAT,
			.depthWriteEnabled = true,
			.depthCompare = wgpu::CompareFunction::Less,
		};

		const	wgpu::ColorTargetState textureMasterInfoColorTargetState = {
			.format = wgpu::TextureFormat::RGBA32Uint,
		};
		const wgpu::ColorTargetState baseColorColorTargetState = {
			.format = wgpu::TextureFormat::RGBA16Unorm,
		};
		const std::array<wgpu::ColorTargetState, 2> colorTargetStates = {
			textureMasterInfoColorTargetState,
			baseColorColorTargetState,
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
				.cullMode = wgpu::CullMode::Back,
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
			.visibility = wgpu::ShaderStage::Vertex,
			.buffer = {
				.type = wgpu::BufferBindingType::ReadOnlyStorage,
				.minBindingSize = sizeof(InstanceProperty),
			}
		};
		const wgpu::BindGroupLayoutEntry materialsBindGroupLayoutEntry = {
			.binding = 3,
			.visibility = wgpu::ShaderStage::Vertex,
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

}
#pragma once
#include "initial.hpp"
#include <array>
#include "../device/device.hpp"
#include "../enums.hpp"
#include "vertexBufferLayout.hpp"
#include "../texture/texture.hpp"
#include "../render/initial.hpp"
#include "../render/shadow.hpp"
#include "../render/ultimate.hpp"

namespace render {
	Initial::Initial(WGPUContext* wgpuContext) : _wgpuContext(wgpuContext) {
		_vertexShaderModule = device::createWGSLShaderModule(_wgpuContext->device, VERTEX_SHADER_LABEL, VERTEX_SHADER_PATH);
		_fragmentShaderModule = device::createWGSLShaderModule(_wgpuContext->device, FRAGMENT_SHADER_LABEL, FRAGMENT_SHADER_PATH);
	};

	void Initial::generateGpuObjects(const render::initial::descriptor::GenerateGpuObjects* descriptor) {
		createBindGroupLayout();
		createPipeline();
		createBindGroup(
			descriptor->cameraBuffer,
			descriptor->transformBuffer,
			descriptor->instancePropertiesBuffer,
			descriptor->materialBuffer
		);
	};

	void Initial::doCommands(const render::initial::descriptor::DoCommands* descriptor) {
		wgpu::RenderPassDepthStencilAttachment renderPassDepthStencilAttachment = {
			.view = depthTextureView,
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

	void Initial::createPipeline() {
		const wgpu::PipelineLayout pipelineLayout = getPipelineLayout();

		const wgpu::VertexState vertexState = {
			.module = _vertexShaderModule,
			.entryPoint = enums::EntryPoint::VERTEX,
			.bufferCount = 1,
			.buffers = &render::vertexBufferLayout,
		};

		const wgpu::DepthStencilState depthStencilState = {
			.format = depthTextureFormat,
			.depthWriteEnabled = true,
			.depthCompare = wgpu::CompareFunction::Less,
		};

		const wgpu::ColorTargetState texCoordColorTargetState = {
			.format = texCoordTextureFormat,
		};

		const std::array<wgpu::ColorTargetState, 1> colorTargetStates = {
			texCoordColorTargetState,
		};
		const wgpu::FragmentState fragmentState = {
			.module = _fragmentShaderModule,
			.entryPoint = enums::EntryPoint::FRAGMENT,
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
		_renderPipeline = _wgpuContext->device.CreateRenderPipeline(&renderPipelineDescriptor);
	}

	void Initial::createBindGroup(
		const wgpu::Buffer& cameraBuffer,
		const wgpu::Buffer& transformBuffer,
		const wgpu::Buffer& instancePropertiesBuffer,
		const wgpu::Buffer& materialBuffer
	) {
		const wgpu::BindGroupEntry screenDimensionsBindGroupEntry = {
			.binding = 0,
			.buffer = _wgpuContext->getScreenDimensionsBuffer(),
			.size = sizeof(_wgpuContext->getScreenDimensionsBuffer().GetSize()),
		};
		const wgpu::BindGroupEntry cameraBindGroupEntry = {
			.binding = 1,
			.buffer = cameraBuffer,
			.size = cameraBuffer.GetSize(),
		};
		const wgpu::BindGroupEntry transformBindGroupEntry = {
			.binding = 2,
			.buffer = transformBuffer,
			.size = transformBuffer.GetSize(),
		};
		const wgpu::BindGroupEntry instancePropertiesBindGroupEntry = {
			.binding = 3,
			.buffer = instancePropertiesBuffer,
			.size = instancePropertiesBuffer.GetSize(),
		};
		const wgpu::BindGroupEntry materialBindGroupEntry = {
			.binding = 4,
			.buffer = materialBuffer,
			.size = materialBuffer.GetSize(),
		};
		const wgpu::BindGroupEntry baseColorTextureIdBindGroupEntry = {
			.binding = 5,
			.buffer = baseColorTextureIdBuffer,
			.size = baseColorTextureIdBuffer.GetSize(),
		};
		const wgpu::BindGroupEntry normalIdTextureIdBindGroupEntry = {
			.binding = 6,
			.buffer = normalTextureIdBuffer,
			.size = normalTextureIdBuffer.GetSize(),
		};
		std::array<wgpu::BindGroupEntry, 7> bindGroupEntries = {
			screenDimensionsBindGroupEntry,
			cameraBindGroupEntry,
			transformBindGroupEntry,
			instancePropertiesBindGroupEntry,
			materialBindGroupEntry,
			baseColorTextureIdBindGroupEntry,
			normalIdTextureIdBindGroupEntry
		};
		const wgpu::BindGroupDescriptor bindGroupDescriptor = {
			.label = "initial render group",
			.layout = _bindGroupLayout,
			.entryCount = bindGroupEntries.size(),
			.entries = bindGroupEntries.data(),
		};
		_bindGroup = _wgpuContext->device.CreateBindGroup(&bindGroupDescriptor);
	}

	void Initial::createBindGroupLayout() {
		const wgpu::BindGroupLayoutEntry screenDimensionBindGroupLayoutEntry = {
			.binding = 0,
			.visibility = wgpu::ShaderStage::Fragment,
			.buffer = {
				.type = wgpu::BufferBindingType::Uniform,
				.minBindingSize = sizeof(_wgpuContext->getScreenDimensions())
			},
		};
		const wgpu::BindGroupLayoutEntry cameraBindGroupLayoutEntry = {
			.binding = 1,
			.visibility = wgpu::ShaderStage::Vertex,
			.buffer = {
				.type = wgpu::BufferBindingType::Uniform,
				.minBindingSize = sizeof(glm::f32mat4x4),
			},
		};
		const wgpu::BindGroupLayoutEntry transformBindGroupLayoutEntry = {
			.binding = 2,
			.visibility = wgpu::ShaderStage::Vertex,
			.buffer = {
				.type = wgpu::BufferBindingType::ReadOnlyStorage,
				.minBindingSize = sizeof(glm::f32mat4x4),
			},
		};
		const wgpu::BindGroupLayoutEntry instancePropertiesBindGroupLayoutEntry = {
			.binding = 3,
			.visibility = wgpu::ShaderStage::Fragment,
			.buffer = {
				.type = wgpu::BufferBindingType::ReadOnlyStorage,
				.minBindingSize = sizeof(uint32_t),
			}
		};
		const wgpu::BindGroupLayoutEntry materialBindGroupLayoutEntry = {
			.binding = 4,
			.visibility = wgpu::ShaderStage::Fragment,
			.buffer = {
				.type = wgpu::BufferBindingType::ReadOnlyStorage,
				.minBindingSize = sizeof(structs::Material),
			},
		};
		const wgpu::BindGroupLayoutEntry baseColorTextureIdBindGroupLayoutEntry = {
			.binding = 5,
			.visibility = wgpu::ShaderStage::Fragment,
			.buffer = {
				.type = wgpu::BufferBindingType::Storage,
				.minBindingSize = baseColorTextureIdBuffer.GetSize(),
			},
		};
		const wgpu::BindGroupLayoutEntry normalTextureIdBindGroupLayoutEntry = {
			.binding = 6,
			.visibility = wgpu::ShaderStage::Fragment,
			.buffer = {
				.type = wgpu::BufferBindingType::Storage,
				.minBindingSize = normalTextureIdBuffer.GetSize(),
			},
		};

		std::array<wgpu::BindGroupLayoutEntry, 7> bindGroupLayoutEntries = {
			screenDimensionBindGroupLayoutEntry,
			cameraBindGroupLayoutEntry,
			transformBindGroupLayoutEntry,
			instancePropertiesBindGroupLayoutEntry,
			materialBindGroupLayoutEntry,
			baseColorTextureIdBindGroupLayoutEntry,
			normalTextureIdBindGroupLayoutEntry,
		};
		const wgpu::BindGroupLayoutDescriptor bindGroupLayoutDescriptor = {
			.label = "initial render bind group layout",
			.entryCount = bindGroupLayoutEntries.size(),
			.entries = bindGroupLayoutEntries.data(),
		};
		_bindGroupLayout = _wgpuContext->device.CreateBindGroupLayout(&bindGroupLayoutDescriptor);
	};

	wgpu::PipelineLayout Initial::getPipelineLayout() {

		const wgpu::PipelineLayoutDescriptor pipelineLayoutDescriptor = {
			.label = "initial render pipeline layout",
			.bindGroupLayoutCount = 1,
			.bindGroupLayouts = &_bindGroupLayout,
		};
		return _wgpuContext->device.CreatePipelineLayout(&pipelineLayoutDescriptor);
	};

}
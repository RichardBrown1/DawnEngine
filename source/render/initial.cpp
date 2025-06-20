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
		_vertexShaderModule = device::createShaderModule(_wgpuContext->device, VERTEX_SHADER_LABEL, VERTEX_SHADER_PATH);
		_fragmentShaderModule = device::createShaderModule(_wgpuContext->device, FRAGMENT_SHADER_LABEL, FRAGMENT_SHADER_PATH);
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

		texture::descriptor::CreateTextureView worldPositionTextureViewDescriptor = {
			.label = _worldPositionLabel,
			.device = &_wgpuContext->device,
			.textureUsage = _worldPositionTextureUsage,
			.textureDimensions = _wgpuContext->screenDimensions,
			.textureFormat = worldPositionTextureFormat,
			.outputTextureView = worldPositionTextureView,
		};
		texture::createTextureView(&worldPositionTextureViewDescriptor);

		texture::descriptor::CreateTextureView baseColorTextureViewDescriptor = {
			.label = _baseColorLabel,
			.device = &_wgpuContext->device,
			.textureUsage = _baseColorTextureUsage,
			.textureDimensions = _wgpuContext->screenDimensions,
			.textureFormat = baseColorTextureFormat,
			.outputTextureView = baseColorTextureView,
		};
		texture::createTextureView(&baseColorTextureViewDescriptor);

		texture::descriptor::CreateTextureView normalTextureViewDescriptor = {
			.label = _normalLabel,
			.device = &_wgpuContext->device,
			.textureUsage = _normalTextureUsage,
			.textureDimensions = _wgpuContext->screenDimensions,
			.textureFormat = normalTextureFormat,
			.outputTextureView = normalTextureView,
		};
		texture::createTextureView(&normalTextureViewDescriptor);

		texture::descriptor::CreateTextureView texCoordTextureViewDescriptor = {
			.label = _texCoordLabel,
			.device = &_wgpuContext->device,
			.textureUsage = _texCoordTextureUsage,
			.textureDimensions = _wgpuContext->screenDimensions,
			.textureFormat = texCoordTextureFormat,
			.outputTextureView = texCoordTextureView,
		};
		texture::createTextureView(&texCoordTextureViewDescriptor);

		texture::descriptor::CreateTextureView baseColorTextureIdTextureViewDescriptor = {
					.label = _baseColorTextureIdLabel,
					.device = &_wgpuContext->device,
					.textureUsage = _baseColorTextureIdTextureUsage,
					.textureDimensions = _wgpuContext->screenDimensions,
					.textureFormat = baseColorTextureIdTextureFormat,
					.outputTextureView = baseColorTextureIdTextureView,
		};
		texture::createTextureView(&baseColorTextureIdTextureViewDescriptor);

		texture::descriptor::CreateTextureView normalTextureIdTextureViewDescriptor = {
					.label = _normalTextureIdLabel,
					.device = &_wgpuContext->device,
					.textureUsage = _normalTextureIdTextureUsage,
					.textureDimensions = _wgpuContext->screenDimensions,
					.textureFormat = normalTextureIdTextureFormat,
					.outputTextureView = normalTextureIdTextureView,
		};
		texture::createTextureView(&normalTextureIdTextureViewDescriptor);


		_renderPassColorAttachments = {
			wgpu::RenderPassColorAttachment {
				.view = worldPositionTextureView,
				.loadOp = wgpu::LoadOp::Clear,
				.storeOp = wgpu::StoreOp::Store,
				.clearValue = wgpu::Color{0.0f, 0.0f, 0.0f, 0.0f},
			},
			wgpu::RenderPassColorAttachment {
				.view = baseColorTextureView,
				.loadOp = wgpu::LoadOp::Clear,
				.storeOp = wgpu::StoreOp::Store,
				.clearValue = wgpu::Color{0.3f, 0.3f, 1.0f, 1.0f},
			},
			wgpu::RenderPassColorAttachment {
				.view = normalTextureView,
				.loadOp = wgpu::LoadOp::Clear,
				.storeOp = wgpu::StoreOp::Store,
				.clearValue = wgpu::Color{0.0f, 0.0f, 0.0f, 0.0f},
			},
			wgpu::RenderPassColorAttachment {
				.view = texCoordTextureView,
				.loadOp = wgpu::LoadOp::Clear,
				.storeOp = wgpu::StoreOp::Store,
				.clearValue = wgpu::Color{0.0f, 0.0f, 0.0f, 0.0f},
			},
			wgpu::RenderPassColorAttachment {
				.view = baseColorTextureIdTextureView,
				.loadOp = wgpu::LoadOp::Clear,
				.storeOp = wgpu::StoreOp::Store,
				.clearValue = wgpu::Color(UINT32_MAX),
			},
			wgpu::RenderPassColorAttachment {
				.view = normalTextureIdTextureView,
				.loadOp = wgpu::LoadOp::Clear,
				.storeOp = wgpu::StoreOp::Store,
				.clearValue = wgpu::Color(UINT32_MAX),
			},
		};

		texture::descriptor::CreateTextureView depthTextureViewDescriptor = {
			.label = _depthTextureLabel,
			.device = &_wgpuContext->device,
			.textureUsage = _depthTextureUsage,
			.textureDimensions = _wgpuContext->screenDimensions,
			.textureFormat = depthTextureFormat,
			.outputTextureView = depthTextureView,
		};
		texture::createTextureView(&depthTextureViewDescriptor);

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

		const	wgpu::ColorTargetState worldPositionColorTargetState = {
			.format = worldPositionTextureFormat,
		};
		const wgpu::ColorTargetState baseColorColorTargetState = {
			.format = baseColorTextureFormat,
		};
		const wgpu::ColorTargetState normalColorTargetState = {
			.format = normalTextureFormat,
		};
		const wgpu::ColorTargetState texCoordColorTargetState = {
			.format = texCoordTextureFormat,
		};
		const wgpu::ColorTargetState baseColorTextureIdColorTargetState = {
			.format = baseColorTextureIdTextureFormat,
		};
		const wgpu::ColorTargetState normalTextureIdColorTargetState = {
			.format = normalTextureIdTextureFormat,
		};

		const std::array<wgpu::ColorTargetState, 6> colorTargetStates = {
			worldPositionColorTargetState,
			baseColorColorTargetState,
			normalColorTargetState,
			texCoordColorTargetState,
			baseColorTextureIdColorTargetState,
			normalTextureIdColorTargetState,
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
		const wgpu::BindGroupEntry cameraBindGroupEntry = {
			.binding = 0,
			.buffer = cameraBuffer,
			.size = cameraBuffer.GetSize(),
		};
		const wgpu::BindGroupEntry transformBindGroupEntry = {
			.binding = 1,
			.buffer = transformBuffer,
			.size = transformBuffer.GetSize(),
		};
		const wgpu::BindGroupEntry instancePropertiesBindGroupEntry = {
			.binding = 2,
			.buffer = instancePropertiesBuffer,
			.size = instancePropertiesBuffer.GetSize(),
		};
		const wgpu::BindGroupEntry materialBindGroupEntry = {
			.binding = 3,
			.buffer = materialBuffer,
			.size = materialBuffer.GetSize(),
		};
		std::array<wgpu::BindGroupEntry, 4> bindGroupEntries = {
			cameraBindGroupEntry,
			transformBindGroupEntry,
			instancePropertiesBindGroupEntry,
			materialBindGroupEntry,
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
		const wgpu::BindGroupLayoutEntry cameraBindGroupLayoutEntry = {
			.binding = 0,
			.visibility = wgpu::ShaderStage::Vertex,
			.buffer = {
				.type = wgpu::BufferBindingType::Uniform,
				.minBindingSize = sizeof(glm::f32mat4x4),
			},
		};
		const wgpu::BindGroupLayoutEntry transformBindGroupLayoutEntry = {
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
				.minBindingSize = sizeof(structs::InstanceProperty),
			}
		};
		const wgpu::BindGroupLayoutEntry materialBindGroupLayoutEntry = {
			.binding = 3,
			.visibility = wgpu::ShaderStage::Fragment,
			.buffer = {
				.type = wgpu::BufferBindingType::ReadOnlyStorage,
				.minBindingSize = sizeof(structs::Material),
			},
		};

		std::array<wgpu::BindGroupLayoutEntry, 4> bindGroupLayoutEntries = {
			cameraBindGroupLayoutEntry,
			transformBindGroupLayoutEntry,
			instancePropertiesBindGroupLayoutEntry,
			materialBindGroupLayoutEntry,
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
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

	void Initial::generateGpuObjects(
		const DeviceResources* deviceResources
	) {
		createBindGroupLayout(deviceResources);
		createPipeline(deviceResources->render->depthTextureFormat);
		createBindGroup(deviceResources);
	};

	void Initial::doCommands(const render::initial::descriptor::DoCommands* descriptor) {
		wgpu::RenderPassDepthStencilAttachment renderPassDepthStencilAttachment = {
			.view = descriptor->depthTextureView,
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

	void Initial::createPipeline(const wgpu::TextureFormat depthTextureFormat) {
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

		const std::array<wgpu::ColorTargetState, 0> colorTargetStates = {};

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
		const DeviceResources* deviceResources
	) {
		const wgpu::BindGroupEntry screenDimensionsBindGroupEntry = {
			.binding = 0,
			.buffer = _wgpuContext->getScreenDimensionsBuffer(),
			.size = sizeof(_wgpuContext->getScreenDimensionsBuffer().GetSize()),
		};
		const wgpu::BindGroupEntry cameraBindGroupEntry = {
			.binding = 1,
			.buffer = deviceResources->scene->cameras,
			.size = deviceResources->scene->cameras.GetSize(),
		};
		const wgpu::BindGroupEntry transformBindGroupEntry = {
			.binding = 2,
			.buffer = deviceResources->scene->transforms,
			.size = deviceResources->scene->transforms.GetSize(),
		};
		const wgpu::BindGroupEntry materialIndicesBindGroupEntry = {
			.binding = 3,
			.buffer = deviceResources->scene->materialIndices,
			.size = deviceResources->scene->materialIndices.GetSize(),
		};
		const wgpu::BindGroupEntry materialBindGroupEntry = {
			.binding = 4,
			.buffer = deviceResources->scene->materials,
			.size = deviceResources->scene->materials.GetSize(),
		};
		const wgpu::BindGroupEntry baseColorIdBindGroupEntry = {
			.binding = 5,
			.textureView = deviceResources->render->baseColorIdTextureView,
		};
		const wgpu::BindGroupEntry normalIdBindGroupEntry = {
			.binding = 6,
			.textureView = deviceResources->render->normalIdTextureView,
		};

		std::array<wgpu::BindGroupEntry, 7> bindGroupEntries = {
			screenDimensionsBindGroupEntry,
			cameraBindGroupEntry,
			transformBindGroupEntry,
			materialIndicesBindGroupEntry,
			materialBindGroupEntry,
			baseColorIdBindGroupEntry,
			normalIdBindGroupEntry,
		};
		const wgpu::BindGroupDescriptor bindGroupDescriptor = {
			.label = "initial render group",
			.layout = _bindGroupLayout,
			.entryCount = bindGroupEntries.size(),
			.entries = bindGroupEntries.data(),
		};
		_bindGroup = _wgpuContext->device.CreateBindGroup(&bindGroupDescriptor);
	}

	void Initial::createBindGroupLayout(const DeviceResources* deviceResources) {
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
		const wgpu::BindGroupLayoutEntry baseColorIdBindGroupLayoutEntry = {
			.binding = 5,
			.visibility = wgpu::ShaderStage::Fragment,
			.storageTexture = {
				.access = wgpu::StorageTextureAccess::WriteOnly,
				.format = deviceResources->render->baseColorIdTextureFormat,
				.viewDimension = wgpu::TextureViewDimension::e2D,
			}
		};
		const wgpu::BindGroupLayoutEntry normalIdBindGroupLayoutEntry = {
			.binding = 6,
			.visibility = wgpu::ShaderStage::Fragment,
			.storageTexture = {
				.access = wgpu::StorageTextureAccess::WriteOnly,
				.format = deviceResources->render->normalIdTextureFormat,
				.viewDimension = wgpu::TextureViewDimension::e2D,
			}
		};

		std::array<wgpu::BindGroupLayoutEntry, 7> bindGroupLayoutEntries = {
			screenDimensionBindGroupLayoutEntry,
			cameraBindGroupLayoutEntry,
			transformBindGroupLayoutEntry,
			instancePropertiesBindGroupLayoutEntry,
			materialBindGroupLayoutEntry,
			baseColorIdBindGroupLayoutEntry,
			normalIdBindGroupLayoutEntry,
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
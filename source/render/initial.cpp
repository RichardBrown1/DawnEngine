#pragma once
#include "initial.hpp"
#include <array>
#include "../device/device.hpp"
#include "../enums.hpp"
#include "vertexBufferLayout.hpp"

namespace render {
	Initial::Initial(WGPUContext* wgpuContext) : _wgpuContext(wgpuContext) {
		_vertexShaderModule = device::createWGSLShaderModule(_wgpuContext->device, VERTEX_SHADER_LABEL, VERTEX_SHADER_PATH);
		_fragmentShaderModule = device::createWGSLShaderModule(_wgpuContext->device, FRAGMENT_SHADER_LABEL, FRAGMENT_SHADER_PATH);
	};

	void Initial::generateGpuObjects(
		const DeviceResources* deviceResources
	) {
		createInputBindGroupLayout();
		createPipelines(deviceResources);
		createInputBindGroup(deviceResources);
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
		renderPassEncoder.SetBindGroup(0, _inputBindGroup);

		for (const auto& dc : descriptor->drawCalls) {
			renderPassEncoder.DrawIndexed(dc.indexCount, dc.instanceCount, dc.firstIndex, dc.baseVertex, dc.firstInstance);
		}
		renderPassEncoder.End();
	}

	void Initial::createPipelines(const DeviceResources* deviceResources) {
		const wgpu::PipelineLayout pipelineLayout = getPipelineLayout();

		const wgpu::VertexState vertexState = {
			.module = _vertexShaderModule,
			.entryPoint = enums::EntryPoint::VERTEX,
			.bufferCount = 1,
			.buffers = &render::vertexBufferLayout,
		};

		const wgpu::DepthStencilState depthStencilState = {
			.format = deviceResources->render->depthTextureFormat,
			.depthWriteEnabled = true,
			.depthCompare = wgpu::CompareFunction::Less,
		};

		wgpu::RenderPipelineDescriptor renderPipelineDescriptor = {
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
		};

		{
			renderPipelineDescriptor.label = "initial render pipeline one";
			const std::array<wgpu::ColorTargetState, 2> colorTargetStates = {
				wgpu::ColorTargetState {.format = deviceResources->render->worldPositionTextureFormat},
				wgpu::ColorTargetState {.format = deviceResources->render->normalTextureFormat}
			};
			const wgpu::FragmentState fragmentState = {
				.module = _oneFragmentShaderModule,
				.entryPoint = enums::EntryPoint::FRAGMENT,
				.targetCount = colorTargetStates.size(),
				.targets = colorTargetStates.data(),
			};
			renderPipelineDescriptor.fragment = &fragmentState;
			_renderPipelineOne = _wgpuContext->device.CreateRenderPipeline(&renderPipelineDescriptor);
		}


	}

	void Initial::createInputBindGroup(
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

		std::array<wgpu::BindGroupEntry, 5> bindGroupEntries = {
			screenDimensionsBindGroupEntry,
			cameraBindGroupEntry,
			transformBindGroupEntry,
			materialIndicesBindGroupEntry,
			materialBindGroupEntry,
		};
		const wgpu::BindGroupDescriptor bindGroupDescriptor = {
			.label = "initial render input bind group",
			.layout = _inputBindGroupLayout,
			.entryCount = bindGroupEntries.size(),
			.entries = bindGroupEntries.data(),
		};
		_inputBindGroup = _wgpuContext->device.CreateBindGroup(&bindGroupDescriptor);
	}

	void Initial::createInputBindGroupLayout() {
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

		std::array<wgpu::BindGroupLayoutEntry, 5> bindGroupLayoutEntries = {
			screenDimensionBindGroupLayoutEntry,
			cameraBindGroupLayoutEntry,
			transformBindGroupLayoutEntry,
			instancePropertiesBindGroupLayoutEntry,
			materialBindGroupLayoutEntry,
		};

		const wgpu::BindGroupLayoutDescriptor bindGroupLayoutDescriptor = {
			.label = "initial render input bind group layout",
			.entryCount = bindGroupLayoutEntries.size(),
			.entries = bindGroupLayoutEntries.data(),
		};
		_inputBindGroupLayout = _wgpuContext->device.CreateBindGroupLayout(&bindGroupLayoutDescriptor);
	};

	wgpu::PipelineLayout Initial::getPipelineLayout() {
		std::array<wgpu::BindGroupLayout, 2> bindGroupLayouts = { _inputBindGroupLayout };

		const wgpu::PipelineLayoutDescriptor pipelineLayoutDescriptor = {
			.label = "initial render pipeline layout",
			.bindGroupLayoutCount = bindGroupLayouts.size(),
			.bindGroupLayouts = bindGroupLayouts.data(),
		};
		return _wgpuContext->device.CreatePipelineLayout(&pipelineLayoutDescriptor);
	};

}
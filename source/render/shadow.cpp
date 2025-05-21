#pragma once
#include "shadow.hpp"
#include "vertexBufferLayout.hpp"
#include "../device/device.hpp"
#include "../enums.hpp"
#include "../constants.hpp"
#include "../texture/texture.hpp"

namespace render {

	Shadow::Shadow(wgpu::Device* device) {
		_device = device;
		_shadowDimensions = wgpu::Extent2D{ 2048, 2048 };

		_vertexShaderModule = device::createShaderModule(*_device, VERTEX_SHADER_LABEL, VERTEX_SHADER_PATH);
		_fragmentShaderModule = device::createShaderModule(*_device, FRAGMENT_SHADER_LABEL, FRAGMENT_SHADER_PATH);
	}

	void Shadow::generateGpuObjects(const render::shadow::descriptor::GenerateGpuObjects* descriptor) {
		createBindGroupLayout();
		createPipeline();
		createBindGroup(descriptor->transformBuffer, descriptor->lightBuffer);
		createShadowMapTextureView();
	}

	void Shadow::doCommands(const render::shadow::descriptor::DoCommands* descriptor) {
		const wgpu::RenderPassDepthStencilAttachment renderPassDepthStencilAttachment = {
			.view = shadowMapTextureView,
			.depthLoadOp = wgpu::LoadOp::Clear,
			.depthStoreOp = wgpu::StoreOp::Store,
			.depthClearValue = 1.0f,
		};

		const wgpu::RenderPassDescriptor renderPassDescriptor = {
			.label = "shadow render pass",
			.depthStencilAttachment = &renderPassDepthStencilAttachment,
		};
		wgpu::RenderPassEncoder renderPassEncoder = descriptor->commandEncoder.BeginRenderPass(&renderPassDescriptor);
		renderPassEncoder.SetPipeline(_renderPipeline);
		renderPassEncoder.SetBindGroup(0, _bindGroup);
		renderPassEncoder.SetVertexBuffer(0, descriptor->vertexBuffer, 0, descriptor->vertexBuffer.GetSize());
		renderPassEncoder.SetIndexBuffer(descriptor->indexBuffer, wgpu::IndexFormat::Uint16, 0, descriptor->indexBuffer.GetSize());

		for (auto& dc : descriptor->drawCalls) {
			renderPassEncoder.DrawIndexed(dc.indexCount, dc.instanceCount, dc.firstIndex, dc.baseVertex, dc.firstInstance);
		}
		renderPassEncoder.End();
	}

	void Shadow::createPipeline() {
		const wgpu::VertexState vertexState = {
			.module = _vertexShaderModule,
			.entryPoint = enums::EntryPoint::VERTEX,
			.bufferCount = 1,
			.buffers = &render::vertexBufferLayout,
		};

		constexpr wgpu::BlendState blendState = {
			.color = wgpu::BlendComponent{
				.operation = wgpu::BlendOperation::Add,
				.srcFactor = wgpu::BlendFactor::SrcAlpha,
				.dstFactor = wgpu::BlendFactor::OneMinusSrcAlpha,
			},
			.alpha = wgpu::BlendComponent{
				.operation = wgpu::BlendOperation::Add,
				.srcFactor = wgpu::BlendFactor::Zero,
				.dstFactor = wgpu::BlendFactor::One,
			}
		};
		const wgpu::FragmentState fragmentState = {
			.module = _fragmentShaderModule,
			.entryPoint = enums::EntryPoint::FRAGMENT,
			.constantCount = 0,
			.constants = nullptr,
		};

		constexpr wgpu::DepthStencilState depthStencilState = {
			.format = constants::DEPTH_FORMAT,
			.depthWriteEnabled = true,
			.depthCompare = wgpu::CompareFunction::Less,
		};

		wgpu::RenderPipelineDescriptor renderPipelineDescriptor = {
			.label = "shadow render pipeline descriptor",
			.layout = getPipelineLayout(),
			.vertex = vertexState,
			.primitive = wgpu::PrimitiveState {
				.topology = wgpu::PrimitiveTopology::TriangleList,
				.cullMode = wgpu::CullMode::Front,
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

	wgpu::PipelineLayout Shadow::getPipelineLayout() {
		const wgpu::PipelineLayoutDescriptor pipelineLayoutDescriptor = {
			.label = "shadow render pipeline layout",
			.bindGroupLayoutCount = 1,
			.bindGroupLayouts = &_bindGroupLayout,
		};
		return _device->CreatePipelineLayout(&pipelineLayoutDescriptor);
	};

	void Shadow::createBindGroupLayout() {
		const wgpu::BindGroupLayoutEntry transformBindGroupLayoutEntry = {
			.binding = 0,
			.visibility = wgpu::ShaderStage::Vertex,
			.buffer = {
				.type = wgpu::BufferBindingType::ReadOnlyStorage,
				.minBindingSize = sizeof(glm::f32mat4x4),
			},
		};
		const wgpu::BindGroupLayoutEntry lightBindGroupLayoutEntry = {
			.binding = 1,
			.visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment,
			.buffer = {
				.type = wgpu::BufferBindingType::ReadOnlyStorage,
				.minBindingSize = sizeof(structs::InstanceProperty),
			}
		};

		std::array<wgpu::BindGroupLayoutEntry, 2> bindGroupLayoutEntries = {
			transformBindGroupLayoutEntry,
			lightBindGroupLayoutEntry,
		};
		const wgpu::BindGroupLayoutDescriptor bindGroupLayoutDescriptor = {
			.label = "shadow render bind group layout",
			.entryCount = bindGroupLayoutEntries.size(),
			.entries = bindGroupLayoutEntries.data(),
		};
		_bindGroupLayout = _device->CreateBindGroupLayout(&bindGroupLayoutDescriptor);
	};

	void Shadow::createBindGroup(wgpu::Buffer& transformBuffer, wgpu::Buffer& lightBuffer) {
		const wgpu::BindGroupEntry transformBindGroupEntry = {
			.binding = 0,
			.buffer = transformBuffer,
			.size = transformBuffer.GetSize(),
		};
		const wgpu::BindGroupEntry lightBindGroupEntry = {
			.binding = 1,
			.buffer = lightBuffer,
			.size = lightBuffer.GetSize(),
		};
		std::array<wgpu::BindGroupEntry, 4> bindGroupEntries = {
			transformBindGroupEntry,
			lightBindGroupEntry,
		};
		const wgpu::BindGroupDescriptor bindGroupDescriptor = {
			.label = "shadow render group",
			.layout = _bindGroupLayout,
			.entryCount = bindGroupEntries.size(),
			.entries = bindGroupEntries.data(),
		};
		_bindGroup = _device->CreateBindGroup(&bindGroupDescriptor);
	}

	void Shadow::createShadowMapTextureView() {
		const texture::descriptor::CreateTextureView createTextureViewDescriptor = {
			.label = "shadow texture view",
			.device = _device,
			.textureDimensions = _shadowDimensions,
			.textureFormat = constants::DEPTH_FORMAT,
			.outputTextureView = shadowMapTextureView,
		};
		texture::createTextureView(&createTextureViewDescriptor);
	}
}
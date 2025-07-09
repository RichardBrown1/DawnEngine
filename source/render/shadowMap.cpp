#pragma once
#include "shadowMap.hpp"
#include "vertexBufferLayout.hpp"
#include "../device/device.hpp"
#include "../enums.hpp"
#include "../constants.hpp"
#include "../texture/texture.hpp"
#include "../structs/structs.hpp"
#include <array>

namespace render {

	ShadowMap::ShadowMap(WGPUContext* wgpuContext) : _wgpuContext(wgpuContext) {
		_vertexShaderModule = device::createShaderModule(_wgpuContext->device, VERTEX_SHADER_LABEL, VERTEX_SHADER_PATH);
		_fragmentShaderModule = device::createShaderModule(_wgpuContext->device, FRAGMENT_SHADER_LABEL, FRAGMENT_SHADER_PATH);
	}

	void ShadowMap::generateGpuObjects(const DeviceResources* deviceResources) {
		const uint32_t maxShadowMapCount = static_cast<uint32_t>(deviceResources->render->shadowMapTextureViews.size());
		const uint32_t lightCount = static_cast<uint32_t>(deviceResources->scene->lights.size());
		_shadowMapCount = maxShadowMapCount > lightCount ? lightCount : maxShadowMapCount;

		createTransformBindGroupLayout();
		createLightBindGroupLayout();
		createPipeline();
		createTransformBindGroup(
			deviceResources->scene->transforms
		);
		for (uint32_t i = 0; i < _shadowMapCount; ++i) {
			insertLightBindGroup(deviceResources->scene->lights[i]);
		}
	}

	void ShadowMap::doCommands(const render::shadowMap::descriptor::DoCommands* descriptor) {
		for (uint32_t i = 0; i < _shadowMapCount; ++i) {
			const wgpu::RenderPassDepthStencilAttachment renderPassDepthStencilAttachment = {
				.view = descriptor->shadowMapTextureViews[i],
				.depthLoadOp = wgpu::LoadOp::Clear,
				.depthStoreOp = wgpu::StoreOp::Store,
				.depthClearValue = 1.0f,
			};

			const wgpu::RenderPassDescriptor renderPassDescriptor = {
				.label = "shadow render pass #" + i,
				.depthStencilAttachment = &renderPassDepthStencilAttachment,
			};
			wgpu::RenderPassEncoder renderPassEncoder = descriptor->commandEncoder.BeginRenderPass(&renderPassDescriptor);
			renderPassEncoder.SetPipeline(_renderPipeline);
			renderPassEncoder.SetBindGroup(0, _transformBindGroup);
			renderPassEncoder.SetBindGroup(1, _lightBindGroups[i]);
			renderPassEncoder.SetVertexBuffer(0, descriptor->vertexBuffer, 0, descriptor->vertexBuffer.GetSize());
			renderPassEncoder.SetIndexBuffer(descriptor->indexBuffer, wgpu::IndexFormat::Uint16, 0, descriptor->indexBuffer.GetSize());

			for (auto& dc : descriptor->drawCalls) {
				renderPassEncoder.DrawIndexed(dc.indexCount, dc.instanceCount, dc.firstIndex, dc.baseVertex, dc.firstInstance);
			}
			renderPassEncoder.End();
		}
	}

	void ShadowMap::createPipeline() {
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

		_renderPipeline = _wgpuContext->device.CreateRenderPipeline(&renderPipelineDescriptor);
	}

	wgpu::PipelineLayout ShadowMap::getPipelineLayout() {
		std::array<wgpu::BindGroupLayout, 2> bindGroupLayouts = {
			_transformBindGroupLayout,
			_lightBindGroupLayout
		};
		const wgpu::PipelineLayoutDescriptor pipelineLayoutDescriptor = {
			.label = "shadow render pipeline layout",
			.bindGroupLayoutCount = bindGroupLayouts.size(),
			.bindGroupLayouts = bindGroupLayouts.data(),
		};
		return _wgpuContext->device.CreatePipelineLayout(&pipelineLayoutDescriptor);
	};

	void ShadowMap::createTransformBindGroupLayout() {
		const wgpu::BindGroupLayoutEntry transformBindGroupLayoutEntry = {
			.binding = 0,
			.visibility = wgpu::ShaderStage::Vertex,
			.buffer = {
				.type = wgpu::BufferBindingType::ReadOnlyStorage,
				.minBindingSize = sizeof(glm::f32mat4x4),
			},
		};

		std::array<wgpu::BindGroupLayoutEntry, 1> bindGroupLayoutEntries = {
			transformBindGroupLayoutEntry,
		};
		const wgpu::BindGroupLayoutDescriptor bindGroupLayoutDescriptor = {
			.label = "shadow render transform bind group layout",
			.entryCount = bindGroupLayoutEntries.size(),
			.entries = bindGroupLayoutEntries.data(),
		};
		_transformBindGroupLayout = _wgpuContext->device.CreateBindGroupLayout(&bindGroupLayoutDescriptor);
	};

	void ShadowMap::createLightBindGroupLayout() {
		const wgpu::BindGroupLayoutEntry lightBindGroupLayoutEntry = {
			.binding = 0,
			.visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment,
			.buffer = {
				.type = wgpu::BufferBindingType::Uniform,
				.minBindingSize = sizeof(structs::Light),
			}
		};
		std::array<wgpu::BindGroupLayoutEntry, 1> bindGroupLayoutEntries = {
			lightBindGroupLayoutEntry,
		};
		const wgpu::BindGroupLayoutDescriptor bindGroupLayoutDescriptor = {
			.label = "shadow render light bind group layout",
			.entryCount = bindGroupLayoutEntries.size(),
			.entries = bindGroupLayoutEntries.data(),
		};
		_lightBindGroupLayout = _wgpuContext->device.CreateBindGroupLayout(&bindGroupLayoutDescriptor);
	};

	void ShadowMap::createTransformBindGroup(
		const wgpu::Buffer& transformBuffer
	) {
		const wgpu::BindGroupEntry transformBindGroupEntry = {
			.binding = 0,
			.buffer = transformBuffer,
			.size = transformBuffer.GetSize(),
		};
		std::array<wgpu::BindGroupEntry, 1> bindGroupEntries = {
			transformBindGroupEntry
		};
		const wgpu::BindGroupDescriptor bindGroupDescriptor = {
			.label = "shadow transform render group",
			.layout = _transformBindGroupLayout,
			.entryCount = bindGroupEntries.size(),
			.entries = bindGroupEntries.data(),
		};
		_transformBindGroup = _wgpuContext->device.CreateBindGroup(&bindGroupDescriptor);
	}

	void ShadowMap::insertLightBindGroup(
		const wgpu::Buffer& lightBuffer
	) {
		const wgpu::BindGroupEntry lightBindGroupEntry = {
			.binding = 0,
			.buffer = lightBuffer,
			.size = lightBuffer.GetSize(),
		};
		std::array<wgpu::BindGroupEntry, 1> bindGroupEntries = {
			lightBindGroupEntry
		};
		const wgpu::BindGroupDescriptor bindGroupDescriptor = {
			.label = "shadow light render group",
			.layout = _lightBindGroupLayout,
			.entryCount = bindGroupEntries.size(),
			.entries = bindGroupEntries.data(),
		};
		_lightBindGroups.emplace_back(_wgpuContext->device.CreateBindGroup(&bindGroupDescriptor));
	}

}
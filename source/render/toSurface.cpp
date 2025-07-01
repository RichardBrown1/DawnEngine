#pragma once
#include "toSurface.hpp"
#include <array>
#include "../device/device.hpp"
#include "../enums.hpp"
#include "vertexBufferLayout.hpp"
#include "../texture/texture.hpp"

namespace render {
	ToSurface::ToSurface(WGPUContext* wgpuContext) : _wgpuContext(wgpuContext) {
		_vertexShaderModule = device::createShaderModule(_wgpuContext->device, VERTEX_SHADER_LABEL, VERTEX_SHADER_PATH);
		_fragmentShaderModule = device::createShaderModule(_wgpuContext->device, FRAGMENT_SHADER_LABEL, FRAGMENT_SHADER_PATH);
	};

	void ToSurface::generateGpuObjects(const DeviceResources* deviceResources) {
		createSampler();
		createBindGroupLayout();
		createPipeline(descriptor->surfaceTextureFormat);
		createBindGroup(
			descriptor->ultimateTextureView
		);

	};

	void ToSurface::doCommands(const render::toSurface::descriptor::DoCommands* descriptor) {
		const wgpu::RenderPassColorAttachment surfaceAttachment = {
			.view = descriptor->surfaceTextureView,
			.loadOp = wgpu::LoadOp::Clear,
			.storeOp = wgpu::StoreOp::Store,
			.clearValue = wgpu::Color{0.7, 0.4, 0.4, 1},
		};

		const wgpu::RenderPassDescriptor renderPassDescriptor = {
			.label = "toSurface render pass",
			.colorAttachmentCount = 1,
			.colorAttachments = &surfaceAttachment,
		};

		wgpu::RenderPassEncoder renderPassEncoder = descriptor->commandEncoder.BeginRenderPass(&renderPassDescriptor);
		renderPassEncoder.SetPipeline(_renderPipeline);
		renderPassEncoder.SetBindGroup(0, _bindGroup);
		renderPassEncoder.Draw(3, 1, 0, 0);
		renderPassEncoder.End();
	}

	void ToSurface::createPipeline(wgpu::TextureFormat surfaceTextureFormat) {
		const wgpu::PipelineLayout pipelineLayout = getPipelineLayout();
		wgpu::VertexState vertexState = {
			.module = _vertexShaderModule,
			.entryPoint = enums::EntryPoint::VERTEX,
		};

		wgpu::ColorTargetState surfaceColorTargetState = {
			.format = surfaceTextureFormat,
		};

		wgpu::FragmentState fragmentState = {
			.module = _fragmentShaderModule,
			.entryPoint = enums::EntryPoint::FRAGMENT,
			.targetCount = 1,
			.targets = &surfaceColorTargetState,
		};

		const wgpu::RenderPipelineDescriptor renderPipelineDescriptor = {
			.label = "toSurface render pipeline",
			.layout = pipelineLayout,
			.vertex = vertexState,
			.fragment = &fragmentState,
		};
		_renderPipeline = _wgpuContext->device.CreateRenderPipeline(&renderPipelineDescriptor);
	}

	void ToSurface::createBindGroup(
		wgpu::TextureView& ultimateTextureView
	//	wgpu::TextureView& shadowMapTextureView
	) {
		const wgpu::BindGroupEntry ultimateTextureViewBindGroupEntry = {
			.binding = 0,
			.textureView = ultimateTextureView,
		};
		const wgpu::BindGroupEntry ultimateSamplerBindGroupEntry = {
			.binding = 1,
			.sampler = _ultimateSampler,
		};

		std::array<wgpu::BindGroupEntry, 2> bindGroupEntries = {
			ultimateTextureViewBindGroupEntry,
			ultimateSamplerBindGroupEntry,
		};
		const wgpu::BindGroupDescriptor bindGroupDescriptor = {
			.label = "toSurface render group",
			.layout = _bindGroupLayout,
			.entryCount = bindGroupEntries.size(),
			.entries = bindGroupEntries.data(),
		};
		_bindGroup = _wgpuContext->device.CreateBindGroup(&bindGroupDescriptor);
	}

	void ToSurface::createBindGroupLayout() {
		const wgpu::BindGroupLayoutEntry ultimateTextureBindGroupLayoutEntry = {
			.binding = 0,
			.visibility = wgpu::ShaderStage::Fragment,
			.texture = {
				.sampleType = wgpu::TextureSampleType::UnfilterableFloat,
				.viewDimension = wgpu::TextureViewDimension::e2D,
			},
		};

		const wgpu::BindGroupLayoutEntry ultimateSamplerBindGroupLayoutEntry = {
			.binding = 1,
			.visibility = wgpu::ShaderStage::Fragment,
			.sampler = {
				.type = wgpu::SamplerBindingType::NonFiltering,
			},
		};

		std::array<wgpu::BindGroupLayoutEntry, 2> bindGroupLayoutEntries = {
			ultimateTextureBindGroupLayoutEntry,
			ultimateSamplerBindGroupLayoutEntry,
		};

		const wgpu::BindGroupLayoutDescriptor bindGroupLayoutDescriptor = {
			.label = "toSurface input bind group layout",
			.entryCount = bindGroupLayoutEntries.size(),
			.entries = bindGroupLayoutEntries.data(),
		};
		_bindGroupLayout = _wgpuContext->device.CreateBindGroupLayout(&bindGroupLayoutDescriptor);
	};

	wgpu::PipelineLayout ToSurface::getPipelineLayout() {
		std::array<wgpu::BindGroupLayout, 1> bindGroupLayout = {
			_bindGroupLayout,
		};
		const wgpu::PipelineLayoutDescriptor pipelineLayoutDescriptor = {
			.label = "toSurface render pipeline layout",
			.bindGroupLayoutCount = bindGroupLayout.size(),
			.bindGroupLayouts = bindGroupLayout.data(),
		};
		return _wgpuContext->device.CreatePipelineLayout(&pipelineLayoutDescriptor);
	};

	void ToSurface::createSampler() {
		constexpr wgpu::SamplerDescriptor samplerDescriptor = {
			.label = "toSurface sampler",
			.addressModeU = wgpu::AddressMode::ClampToEdge,
			.addressModeV = wgpu::AddressMode::ClampToEdge,
			.magFilter = wgpu::FilterMode::Nearest,
			.minFilter = wgpu::FilterMode::Nearest,
			.mipmapFilter = wgpu::MipmapFilterMode::Nearest,
		};
		_ultimateSampler = _wgpuContext->device.CreateSampler(&samplerDescriptor);
	}

}

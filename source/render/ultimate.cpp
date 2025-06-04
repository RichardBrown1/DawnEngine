#pragma once
#include "ultimate.hpp"
#include <array>
#include "../device/device.hpp"
#include "../enums.hpp"
#include "../texture/texture.hpp"
#include <dawn/webgpu_cpp.h>

namespace render {
	Ultimate::Ultimate(WGPUContext* wgpuContext) : _wgpuContext(wgpuContext) {
		_computeShaderModule = device::createWGSLShaderModule(wgpuContext->device, ULTIMATE_SHADER_LABEL, ULTIMATE_SHADER_PATH);
	};

	void Ultimate::generateGpuObjects(const render::ultimate::descriptor::GenerateGpuObjects* descriptor) {
		const texture::descriptor::CreateTextureView createTextureViewDescriptor = {
			.label = "ultimate texture view",
			.device = &_wgpuContext->device,
			.textureUsage = wgpu::TextureUsage::StorageBinding | wgpu::TextureUsage::TextureBinding,
			.textureDimensions = _wgpuContext->screenDimensions,
			.textureFormat = ultimateTextureFormat,
			.outputTextureView = ultimateTextureView,
		};
		texture::createTextureView(&createTextureViewDescriptor);
		createBindGroupLayout(descriptor->baseColorTextureFormat);
		createPipeline();
		createBindGroup(
			descriptor->baseColorTextureView
		);
	};

	void Ultimate::doCommands(const render::ultimate::descriptor::DoCommands* descriptor) {
		wgpu::ComputePassDescriptor computePassDescriptor = {
			.label = "ultimate compute pass",
		};
		wgpu::ComputePassEncoder computePassEncoder = descriptor->commandEncoder.BeginComputePass(&computePassDescriptor);
		computePassEncoder.SetPipeline(_computePipeline);
		computePassEncoder.SetBindGroup(0, _bindGroup);
		computePassEncoder.DispatchWorkgroups(_wgpuContext->screenDimensions.width, _wgpuContext->screenDimensions.height);
		computePassEncoder.End();
	}

	void Ultimate::createPipeline() {
		const wgpu::PipelineLayout pipelineLayout = getPipelineLayout();
		wgpu::ComputeState computeState = {
			.module = _computeShaderModule,
			.entryPoint = enums::EntryPoint::COMPUTE,
		};

		const wgpu::ComputePipelineDescriptor computePipelineDescriptor = {
			.label = "ultimate compute pipeline",
			.layout = pipelineLayout,
			.compute = computeState,
		};
		_computePipeline = _wgpuContext->device.CreateComputePipeline(&computePipelineDescriptor);
	}

	void Ultimate::createBindGroup(
		wgpu::TextureView& baseColorTextureView
	//	wgpu::TextureView& shadowMapTextureView
	) {
		const wgpu::BindGroupEntry ultimateBindGroupEntry = {
			.binding = 0,
			.textureView = ultimateTextureView,
		};
		const wgpu::BindGroupEntry baseColorBindGroupEntry = {
			.binding = 1,
			.textureView = baseColorTextureView,
		};

		std::array<wgpu::BindGroupEntry, 2> bindGroupEntries = {
			ultimateBindGroupEntry,
			baseColorBindGroupEntry,
		};
		const wgpu::BindGroupDescriptor bindGroupDescriptor = {
			.label = "ultimate bind group",
			.layout = _bindGroupLayout,
			.entryCount = bindGroupEntries.size(),
			.entries = bindGroupEntries.data(),
		};
		_bindGroup = _wgpuContext->device.CreateBindGroup(&bindGroupDescriptor);
	}

	void Ultimate::createBindGroupLayout(wgpu::TextureFormat baseColorTextureFormat) {
		const wgpu::BindGroupLayoutEntry ultimateBindGroupLayoutEntry = {
			.binding = 0,
			.visibility = wgpu::ShaderStage::Compute,
			.storageTexture = {
				.access = wgpu::StorageTextureAccess::WriteOnly, //this should be write only but I don't think I can do it on HLSL
				.format = ultimateTextureFormat,
				.viewDimension = wgpu::TextureViewDimension::e2D,
			},
		};

		const wgpu::BindGroupLayoutEntry baseColorBindGroupLayoutEntry = {
			.binding = 1,
			.visibility = wgpu::ShaderStage::Compute,
			.storageTexture = {
				.access = wgpu::StorageTextureAccess::ReadOnly,
				.format = baseColorTextureFormat,
				.viewDimension = wgpu::TextureViewDimension::e2D,
			},
		};

		std::array<wgpu::BindGroupLayoutEntry, 2> bindGroupLayoutEntries = {
			ultimateBindGroupLayoutEntry,
			baseColorBindGroupLayoutEntry,
		};

		const wgpu::BindGroupLayoutDescriptor bindGroupLayoutDescriptor = {
			.label = "ultimate input bind group layout",
			.entryCount = bindGroupLayoutEntries.size(),
			.entries = bindGroupLayoutEntries.data(),
		};
		_bindGroupLayout = _wgpuContext->device.CreateBindGroupLayout(&bindGroupLayoutDescriptor);
	};

	wgpu::PipelineLayout Ultimate::getPipelineLayout() {
		std::array<wgpu::BindGroupLayout, 1> bindGroupLayout = {
			_bindGroupLayout,
		};
		const wgpu::PipelineLayoutDescriptor pipelineLayoutDescriptor = {
			.label = "ultimate render pipeline layout",
			.bindGroupLayoutCount = bindGroupLayout.size(),
			.bindGroupLayouts = bindGroupLayout.data(),
		};
		return _wgpuContext->device.CreatePipelineLayout(&pipelineLayoutDescriptor);
	};

}

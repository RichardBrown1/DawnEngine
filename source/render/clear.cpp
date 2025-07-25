#pragma once
#include "clear.hpp"
#include <array>
#include "../device/device.hpp"
#include "../enums.hpp"
#include "../texture/texture.hpp"

//TODO Create render pipeline to clear the storage texture
namespace render {
	Clear::Clear(WGPUContext* wgpuContext) : _wgpuContext(wgpuContext) {
		_computeShaderModule = device::createWGSLShaderModule(wgpuContext->device, CLEAR_SHADER_LABEL, CLEAR_SHADER_PATH);
	};

	void Clear::generateGpuObjects(const render::clear::descriptor::GenerateGpuObjects* descriptor) {
		createBindGroupLayout(
			descriptor->textureFormat
		);
		createComputePipeline();

		createBindGroup(
			descriptor->textureView
		);
	}

	void Clear::doCommands(const render::clear::descriptor::DoCommands* descriptor) {
		wgpu::ComputePassDescriptor computePassDescriptor = {
			.label = "clear compute pass",
		};
		wgpu::ComputePassEncoder computePassEncoder = descriptor->commandEncoder.BeginComputePass(&computePassDescriptor);
		computePassEncoder.SetPipeline(_computePipeline);
		computePassEncoder.SetBindGroup(0, _bindGroup);
		computePassEncoder.DispatchWorkgroups(_wgpuContext->getScreenDimensions().width, _wgpuContext->getScreenDimensions().height);
		computePassEncoder.End();
	}

	void Clear::createBindGroupLayout(
		const wgpu::TextureFormat textureFormat
	) {
		const wgpu::BindGroupLayoutEntry bindGroupLayoutEntry = {
			.binding = 0,
			.visibility = wgpu::ShaderStage::Compute,
			.storageTexture = {
				.access = wgpu::StorageTextureAccess::WriteOnly,
				.format = textureFormat,
				.viewDimension = wgpu::TextureViewDimension::e2D,
			},
		};

		std::array<wgpu::BindGroupLayoutEntry, 1> bindGroupLayoutEntries = {
			bindGroupLayoutEntry
		};

		const wgpu::BindGroupLayoutDescriptor bindGroupLayoutDescriptor = {
			.label = "clear  bind group layout",
			.entryCount = bindGroupLayoutEntries.size(),
			.entries = bindGroupLayoutEntries.data(),
		};
		_bindGroupLayout = _wgpuContext->device.CreateBindGroupLayout(&bindGroupLayoutDescriptor);
	}

	void Clear::createComputePipeline() {
		const wgpu::PipelineLayout pipelineLayout = getPipelineLayout();
		wgpu::ComputeState computeState = {
			.module = _computeShaderModule,
			.entryPoint = enums::EntryPoint::COMPUTE,
		};

		const wgpu::ComputePipelineDescriptor computePipelineDescriptor = {
			.label = "clear compute pipeline",
			.layout = pipelineLayout,
			.compute = computeState,
		};
		_computePipeline = _wgpuContext->device.CreateComputePipeline(&computePipelineDescriptor);
	}

	wgpu::PipelineLayout Clear::getPipelineLayout() {
		std::array<wgpu::BindGroupLayout, 1> bindGroupLayout = {
			_bindGroupLayout
		};
		const wgpu::PipelineLayoutDescriptor pipelineLayoutDescriptor = {
			.label = "clear render pipeline layout",
			.bindGroupLayoutCount = bindGroupLayout.size(),
			.bindGroupLayouts = bindGroupLayout.data(),
		};
		return _wgpuContext->device.CreatePipelineLayout(&pipelineLayoutDescriptor);
	}

	void Clear::createBindGroup(
		const wgpu::TextureView& textureView
	) {
		const wgpu::BindGroupEntry bindGroupEntry = {
			.binding = 0,
			.textureView = textureView,
		};

		std::array<wgpu::BindGroupEntry, 1> bindGroupEntries = {
			bindGroupEntry,
		};
		const wgpu::BindGroupDescriptor bindGroupDescriptor = {
			.label = "clear  bind group",
			.layout = _bindGroupLayout,
			.entryCount = bindGroupEntries.size(),
			.entries = bindGroupEntries.data(),
		};
		_bindGroup = _wgpuContext->device.CreateBindGroup(&bindGroupDescriptor);
	}

}

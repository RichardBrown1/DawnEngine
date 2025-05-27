#pragma once
#include <iostream>
#include <ktx.h>
#include "accumulator.hpp"
#include "../device/device.hpp"
#include "../enums.hpp"

namespace render {
	Accumulator::Accumulator(wgpu::Device* device) {
		_device = device;
		_screenDimensions = wgpu::Extent2D{ 0, 0 };

		_computeShaderModule = device::createShaderModule(
			*_device,
			BASE_COLOR_ACCUMULATOR_SHADER_LABEL,
			BASE_COLOR_ACCUMULATOR_SHADER_PATH
		);
	};

	void Accumulator::generateGpuObjects(const accumulator::descriptor::GenerateGpuObjects* descriptor) {
		createAccumulatorBindGroupLayout(
			descriptor->accumulatorTextureFormat,
			descriptor->infoTextureFormat
		);
		createInputBindGroupLayout(descriptor->inputTextureFormat);
		createComputePipeline();
	}

	void Accumulator::doCommands(const accumulator::descriptor::DoCommands* descriptor) {
		wgpu::ComputePassDescriptor computePassDescriptor = {
			.label = "ultimate compute pass",
		};
		wgpu::ComputePassEncoder computePassEncoder = descriptor->commandEncoder.BeginComputePass(&computePassDescriptor);
		computePassEncoder.SetPipeline(_computePipeline);
		computePassEncoder.SetBindGroup(0, _accumulatorBindGroup);
		computePassEncoder.SetBindGroup(1, getInputBindGroup());
		computePassEncoder.DispatchWorkgroups(_screenDimensions.width, _screenDimensions.height);
		computePassEncoder.End();
	}

	wgpu::BindGroup Accumulator::getInputBindGroup(
			wgpu::TextureView& inputTextureView
	) {
		const wgpu::BindGroupEntry inputBindGroupEntry = {
			.binding = 0,
			.textureView = inputTextureView,
		};
		std::array<wgpu::BindGroupEntry, 1> bindGroupEntries = {
			inputBindGroupEntry,
		};
		const wgpu::BindGroupDescriptor bindGroupDescriptor = {
			.label = "accumulator input bind group",
			.layout = _inputBindGroupLayout,
			.entryCount = bindGroupEntries.size(),
			.entries = bindGroupEntries.data(),
		};
		return _device->CreateBindGroup(&bindGroupDescriptor);
	}

	void Accumulator::createAccumulatorBindGroup(
			wgpu::TextureView& inputTextureView,
			wgpu::TextureView& infoTextureView
	) {
		const wgpu::BindGroupEntry inputBindGroupEntry = {
			.binding = 0,
			.textureView = inputTextureView,
		};
		std::array<wgpu::BindGroupEntry, 1> bindGroupEntries = {
			inputBindGroupEntry,
		};
		const wgpu::BindGroupDescriptor bindGroupDescriptor = {
			.label = "accumulator accumulator bind group",
			.layout = _inputBindGroupLayout,
			.entryCount = bindGroupEntries.size(),
			.entries = bindGroupEntries.data(),
		};
		_accumulatorBindGroup = _device->CreateBindGroup(&bindGroupDescriptor);
	}


	void Accumulator::createAccumulatorBindGroupLayout(
		wgpu::TextureFormat accumulatorTextureFormat,
		wgpu::TextureFormat infoTextureFormat
	) {
			const wgpu::BindGroupLayoutEntry accumulatorBindGroupLayoutEntry = {
			.binding = 0,
			.visibility = wgpu::ShaderStage::Compute,
			.storageTexture = {
				.access = wgpu::StorageTextureAccess::ReadWrite,
				.format = accumulatorTextureFormat,
				.viewDimension = wgpu::TextureViewDimension::e2D,
			},
		};

		const wgpu::BindGroupLayoutEntry infoBindGroupLayoutEntry = {
			.binding = 1,
			.visibility = wgpu::ShaderStage::Compute,
			.storageTexture = {
				.access = wgpu::StorageTextureAccess::ReadOnly,
				.format = infoTextureFormat,
				.viewDimension = wgpu::TextureViewDimension::e2D,
			},
		};

		std::array<wgpu::BindGroupLayoutEntry, 2> bindGroupLayoutEntries = {
			accumulatorBindGroupLayoutEntry,
			infoBindGroupLayoutEntry,
		};

		const wgpu::BindGroupLayoutDescriptor bindGroupLayoutDescriptor = {
			.label = "accumulator accumulator bind group layout",
			.entryCount = bindGroupLayoutEntries.size(),
			.entries = bindGroupLayoutEntries.data(),
		};
		_accumulatorBindGroupLayout = _device->CreateBindGroupLayout(&bindGroupLayoutDescriptor);

	}

	void Accumulator::createInputBindGroupLayout(
		wgpu::TextureFormat inputTextureFormat
	) {
			const wgpu::BindGroupLayoutEntry inputBindGroupLayoutEntry = {
			.binding = 0,
			.visibility = wgpu::ShaderStage::Compute,
			.storageTexture = {
				.access = wgpu::StorageTextureAccess::ReadOnly,
				.format = inputTextureFormat,
				.viewDimension = wgpu::TextureViewDimension::e2D,
			},
		};
		std::array<wgpu::BindGroupLayoutEntry, 1> bindGroupLayoutEntries = {
			inputBindGroupLayoutEntry
		};

		const wgpu::BindGroupLayoutDescriptor bindGroupLayoutDescriptor = {
			.label = "accumulator input bind group layout",
			.entryCount = bindGroupLayoutEntries.size(),
			.entries = bindGroupLayoutEntries.data(),
		};
		_accumulatorBindGroupLayout = _device->CreateBindGroupLayout(&bindGroupLayoutDescriptor);

	}


	wgpu::PipelineLayout Accumulator::getPipelineLayout() {
		std::array<wgpu::BindGroupLayout, 2> bindGroupLayout = {
			_accumulatorBindGroupLayout,
			_inputBindGroupLayout,
		};
		const wgpu::PipelineLayoutDescriptor pipelineLayoutDescriptor = {
			.label = "accumulator render pipeline layout",
			.bindGroupLayoutCount = bindGroupLayout.size(),
			.bindGroupLayouts = bindGroupLayout.data(),
		};
		return _device->CreatePipelineLayout(&pipelineLayoutDescriptor);
	};

	void Accumulator::createComputePipeline() {
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
		_computePipeline = _device->CreateComputePipeline(&computePipelineDescriptor);
	}
	
}
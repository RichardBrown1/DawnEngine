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
			descriptor->texCoordTextureFormat,
			descriptor->textureIdTextureFormat
		);
		createInputBindGroupLayout();
		createComputePipeline();

		createAccumulatorBindGroup(
			descriptor->accumulatorTextureView,
			descriptor->texCoordTextureView,
			descriptor->textureIdTextureView
		);

		std::vector<wgpu::Buffer> inputInfoBuffers;
		for (uint32_t i = 0; descriptor->inputSTPs.size(); i++) {
			structs::InputInfo inputInfo = {
				.stpIndex = i
			};
			inputInfoBuffers.emplace_back(
				device::createBuffer(descriptor->wgpuContext, inputInfo, "input info", wgpu::BufferUsage::CopyDst)
			);
		};
		for (uint32_t i = 0; auto & stp : descriptor->inputSTPs) {
			insertInputBindGroup(
				inputInfoBuffers[i],
				descriptor->allTextureViews[stp.textureIndex],
				descriptor->allSamplers[stp.samplerIndex]
			);
			i++;
		}
	}

	void Accumulator::doCommands(const accumulator::descriptor::DoCommands* descriptor) {
		wgpu::ComputePassDescriptor computePassDescriptor = {
			.label = "ultimate compute pass",
		};
		wgpu::ComputePassEncoder computePassEncoder = descriptor->commandEncoder.BeginComputePass(&computePassDescriptor);
		computePassEncoder.SetPipeline(_computePipeline);
		computePassEncoder.SetBindGroup(0, _accumulatorBindGroup);
		for (auto& bg : _inputBindGroups) {
			computePassEncoder.SetBindGroup(1, bg);
			computePassEncoder.DispatchWorkgroups(_screenDimensions.width, _screenDimensions.height);
		}
		computePassEncoder.End();
	}


	void Accumulator::createAccumulatorBindGroup(
			wgpu::TextureView& accumulatorTextureView,
			wgpu::TextureView& texCoordTextureView,
			wgpu::TextureView& textureIdTextureView
	) {
		const wgpu::BindGroupEntry accumulatorBindGroupEntry = {
			.binding = 0,
			.textureView = accumulatorTextureView,
		};
		const wgpu::BindGroupEntry texCoordBindGroupEntry = {
			.binding = 1,
			.textureView = texCoordTextureView,
		};
		const wgpu::BindGroupEntry textureIdBindGroupEntry = {
			.binding = 2,
			.textureView = textureIdTextureView,
		};

		std::array<wgpu::BindGroupEntry, 3> bindGroupEntries = {
			accumulatorBindGroupEntry,
			texCoordBindGroupEntry,
			textureIdBindGroupEntry
		};
		const wgpu::BindGroupDescriptor bindGroupDescriptor = {
			.label = "accumulator accumulator bind group",
			.layout = _inputBindGroupLayout,
			.entryCount = bindGroupEntries.size(),
			.entries = bindGroupEntries.data(),
		};
		_accumulatorBindGroup = _device->CreateBindGroup(&bindGroupDescriptor);
	}

	void Accumulator::insertInputBindGroup(
			wgpu::Buffer& inputInfoBuffer,
			wgpu::TextureView& inputTextureView,
			wgpu::Sampler& sampler
	) {
		const wgpu::BindGroupEntry inputInfoBindGroupEntry = {
			.binding = 0,
			.buffer = inputInfoBuffer,
		};
		const wgpu::BindGroupEntry textureBindGroupEntry = {
			.binding = 1,
			.textureView = inputTextureView,
		};
		const wgpu::BindGroupEntry samplerBindGroupEntry = {
			.binding = 2,
			.sampler = sampler,
		};
		std::array<wgpu::BindGroupEntry, 3> bindGroupEntries = {
			inputInfoBindGroupEntry,
			textureBindGroupEntry,
			samplerBindGroupEntry,
		};
		const wgpu::BindGroupDescriptor bindGroupDescriptor = {
			.label = "accumulator input bind group",
			.layout = _inputBindGroupLayout,
			.entryCount = bindGroupEntries.size(),
			.entries = bindGroupEntries.data(),
		};
		_inputBindGroups.emplace_back(_device->CreateBindGroup(&bindGroupDescriptor));
	}


	void Accumulator::createAccumulatorBindGroupLayout(
		wgpu::TextureFormat accumulatorTextureFormat,
		wgpu::TextureFormat texCoordTextureFormat,
		wgpu::TextureFormat textureIdTextureFormat
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

		const wgpu::BindGroupLayoutEntry texCoordBindGroupLayoutEntry = {
			.binding = 1,
			.visibility = wgpu::ShaderStage::Compute,
			.storageTexture = {
				.access = wgpu::StorageTextureAccess::ReadOnly,
				.format = texCoordTextureFormat,
				.viewDimension = wgpu::TextureViewDimension::e2D,
			},
		};

		const wgpu::BindGroupLayoutEntry textureIdBindGroupLayoutEntry = {
			.binding = 2,
			.visibility = wgpu::ShaderStage::Compute,
			.storageTexture = {
				.access = wgpu::StorageTextureAccess::ReadOnly,
				.format = textureIdTextureFormat,
				.viewDimension = wgpu::TextureViewDimension::e2D,
			},
		};

		std::array<wgpu::BindGroupLayoutEntry, 3> bindGroupLayoutEntries = {
			accumulatorBindGroupLayoutEntry,
			texCoordBindGroupLayoutEntry,
			textureIdBindGroupLayoutEntry
		};

		const wgpu::BindGroupLayoutDescriptor bindGroupLayoutDescriptor = {
			.label = "accumulator accumulator bind group layout",
			.entryCount = bindGroupLayoutEntries.size(),
			.entries = bindGroupLayoutEntries.data(),
		};
		_accumulatorBindGroupLayout = _device->CreateBindGroupLayout(&bindGroupLayoutDescriptor);

	}

	void Accumulator::createInputBindGroupLayout() {
		const wgpu::BindGroupLayoutEntry textureBindGroupLayoutEntry = {
			.binding = 0,
			.visibility = wgpu::ShaderStage::Compute,
			.texture = {
					.sampleType = wgpu::TextureSampleType::Float,
					.viewDimension = wgpu::TextureViewDimension::e2D,
			},
		};
		const wgpu::BindGroupLayoutEntry samplerBindGroupLayoutEntry = {
			.binding = 1,
			.visibility = wgpu::ShaderStage::Compute,
			.sampler = {
				//TODO: This should depend on the sampler descriptor magfilter and comparison filter probably instead of being assumed
				.type = wgpu::SamplerBindingType::Filtering,	//ASSUMPTION
			},
		};
		std::array<wgpu::BindGroupLayoutEntry, 2> bindGroupLayoutEntries = {
			textureBindGroupLayoutEntry,
			samplerBindGroupLayoutEntry,
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
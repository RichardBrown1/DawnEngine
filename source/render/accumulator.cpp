#pragma once
#include <iostream>
#include <ktx.h>
#include "accumulator.hpp"
#include "../device/device.hpp"
#include "../enums.hpp"
#include <array>
#include <cstdint>
#include <vector>
#include "../structs/structs.hpp"
#include "../wgpuContext/wgpuContext.hpp"
#include <dawn/webgpu_cpp.h>

namespace render {
	Accumulator::Accumulator(WGPUContext* wgpuContext) : _wgpuContext(wgpuContext) {
		_computeShaderModule = device::createWGSLShaderModule(
			_wgpuContext->device,
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
		for (uint32_t i = 0; i < descriptor->inputSTPs.size(); i++) {
			structs::InputInfo inputInfo = {
				.stpIndex = i
			};
			inputInfoBuffers.emplace_back(
				device::createBuffer(descriptor->wgpuContext, inputInfo, "input info", wgpu::BufferUsage::Uniform)
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
			.label = "accumulator compute pass",
		};
		wgpu::ComputePassEncoder computePassEncoder = descriptor->commandEncoder.BeginComputePass(&computePassDescriptor);
		computePassEncoder.SetPipeline(_computePipeline);
		computePassEncoder.SetBindGroup(0, _accumulatorBindGroup);
		for (auto& bg : _inputBindGroups) {
			computePassEncoder.SetBindGroup(1, bg);
			computePassEncoder.DispatchWorkgroups(descriptor->screenDimensions.width, descriptor->screenDimensions.height);
		}
		computePassEncoder.End();
	}


	void Accumulator::createAccumulatorBindGroup(
			const wgpu::TextureView& accumulatorTextureView,
			const wgpu::TextureView& texCoordTextureView,
			const wgpu::TextureView& textureIdTextureView
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
			.layout = _accumulatorBindGroupLayout,
			.entryCount = bindGroupEntries.size(),
			.entries = bindGroupEntries.data(),
		};
		_accumulatorBindGroup = _wgpuContext->device.CreateBindGroup(&bindGroupDescriptor);
	}

	void Accumulator::insertInputBindGroup(
			const wgpu::Buffer& inputInfoBuffer,
			const wgpu::TextureView& inputTextureView,
			const wgpu::Sampler& sampler
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
		_inputBindGroups.emplace_back(_wgpuContext->device.CreateBindGroup(&bindGroupDescriptor));
	}


	void Accumulator::createAccumulatorBindGroupLayout(
		const wgpu::TextureFormat accumulatorTextureFormat,
		const wgpu::TextureFormat texCoordTextureFormat,
		const wgpu::TextureFormat textureIdTextureFormat
	) {
			const wgpu::BindGroupLayoutEntry accumulatorBindGroupLayoutEntry = {
			.binding = 0,
			.visibility = wgpu::ShaderStage::Compute,
			.storageTexture = {
				.access = wgpu::StorageTextureAccess::WriteOnly,
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
		_accumulatorBindGroupLayout = _wgpuContext->device.CreateBindGroupLayout(&bindGroupLayoutDescriptor);

	}

	void Accumulator::createInputBindGroupLayout() {
		const wgpu::BindGroupLayoutEntry inputInfoBindGroupLayoutEntry = {
			.binding = 0,
			.visibility = wgpu::ShaderStage::Compute,
			.buffer = {
				.type = wgpu::BufferBindingType::Uniform,
				.minBindingSize = 16,
			},
		};
		const wgpu::BindGroupLayoutEntry textureBindGroupLayoutEntry = {
			.binding = 1,
			.visibility = wgpu::ShaderStage::Compute,
			.texture = {
				.sampleType = wgpu::TextureSampleType::Float,
				.viewDimension = wgpu::TextureViewDimension::e2D,
			},
		};
		const wgpu::BindGroupLayoutEntry samplerBindGroupLayoutEntry = {
			.binding = 2,
			.visibility = wgpu::ShaderStage::Compute,
			.sampler = {
				//TODO: This should depend on the sampler descriptor magfilter and comparison filter probably instead of being assumed
				.type = wgpu::SamplerBindingType::Filtering,	//ASSUMPTION
			},
		};
		std::array<wgpu::BindGroupLayoutEntry, 3> bindGroupLayoutEntries = {
			inputInfoBindGroupLayoutEntry,
			textureBindGroupLayoutEntry,
			samplerBindGroupLayoutEntry,
		};

		const wgpu::BindGroupLayoutDescriptor bindGroupLayoutDescriptor = {
			.label = "input bind group layout",
			.entryCount = bindGroupLayoutEntries.size(),
			.entries = bindGroupLayoutEntries.data(),
		};
		_inputBindGroupLayout = _wgpuContext->device.CreateBindGroupLayout(&bindGroupLayoutDescriptor);
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
		return _wgpuContext->device.CreatePipelineLayout(&pipelineLayoutDescriptor);
	};

	void Accumulator::createComputePipeline() {
		const wgpu::PipelineLayout pipelineLayout = getPipelineLayout();
		wgpu::ComputeState computeState = {
			.module = _computeShaderModule,
			.entryPoint = enums::EntryPoint::COMPUTE,
		};

		const wgpu::ComputePipelineDescriptor computePipelineDescriptor = {
			.label = "accumulator compute pipeline",
			.layout = pipelineLayout,
			.compute = computeState,
		};
		_computePipeline = _wgpuContext->device.CreateComputePipeline(&computePipelineDescriptor);
	}
	
}
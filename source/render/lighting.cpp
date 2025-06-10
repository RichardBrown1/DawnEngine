#pragma once
#include "lighting.hpp"
#include "../device/device.hpp"
#include "../enums.hpp"

//TODO Create render pipeline to clear the storage texture
namespace render {
	Lighting::Lighting(WGPUContext* wgpuContext) : _wgpuContext(wgpuContext) {
		_computeShaderModule = device::createWGSLShaderModule(wgpuContext->device, LIGHTING_SHADER_LABEL, LIGHTING_SHADER_PATH);
	};

	void Lighting::generateGpuObjects(const render::lighting::descriptor::GenerateGpuObjects* descriptor) {
		createAccumulatorBindGroupLayout(descriptor->normalTextureFormat);
		createInputBindGroupLayout();
		createComputePipeline();

		createAccumulatorBindGroup(
			descriptor->normalTextureView
		);
		
		std::vector<wgpu::Buffer> lightBuffers;
		for (auto& light : descriptor->lights) {
			lightBuffers.emplace_back(device::createBuffer(*_wgpuContext, light, "lights", wgpu::BufferUsage::Uniform));
		}
		for (auto& buffer : lightBuffers) {
			insertInputBindGroup(buffer);
		}
	}

	void Lighting::doCommands(const render::lighting::descriptor::DoCommands* descriptor) {
		wgpu::ComputePassDescriptor computePassDescriptor = {
			.label = "lighting compute pass",
		};
		wgpu::ComputePassEncoder computePassEncoder = descriptor->commandEncoder.BeginComputePass(&computePassDescriptor);
		computePassEncoder.SetPipeline(_computePipeline);
		computePassEncoder.SetBindGroup(0, _accumulatorBindGroup);
		for (auto& bg : _inputBindGroups) {
			computePassEncoder.SetBindGroup(1, bg);
			computePassEncoder.DispatchWorkgroups(_wgpuContext->screenDimensions.width, _wgpuContext->screenDimensions.height);
		}
		computePassEncoder.End();
	}

	void Lighting::createAccumulatorBindGroupLayout(wgpu::TextureFormat normalTextureFormat) {
		const wgpu::BindGroupLayoutEntry accumulatorBindGroupLayoutEntry = {
			.binding = 0,
			.visibility = wgpu::ShaderStage::Compute,
			.storageTexture = {
				.access = wgpu::StorageTextureAccess::WriteOnly,
				.format = lightFormat,
				.viewDimension = wgpu::TextureViewDimension::e2D,
			},
		};

		const wgpu::BindGroupLayoutEntry  normalBindGroupLayoutEntry = {
			.binding = 1,
			.visibility = wgpu::ShaderStage::Compute,
			.storageTexture = {
				.access = wgpu::StorageTextureAccess::ReadOnly,
				.format = normalTextureFormat,
				.viewDimension = wgpu::TextureViewDimension::e2D,
			},
		};

		std::array<wgpu::BindGroupLayoutEntry, 2> bindGroupLayoutEntries = {
			accumulatorBindGroupLayoutEntry,
			normalBindGroupLayoutEntry,
		};

		const wgpu::BindGroupLayoutDescriptor bindGroupLayoutDescriptor = {
			.label = "lighting accumulator bind group layout",
			.entryCount = bindGroupLayoutEntries.size(),
			.entries = bindGroupLayoutEntries.data(),
		};
		_accumulatorBindGroupLayout = _wgpuContext->device.CreateBindGroupLayout(&bindGroupLayoutDescriptor);
	}

	void Lighting::createInputBindGroupLayout() {
		const wgpu::BindGroupLayoutEntry lightBindGroupLayoutEntry = {
			.binding = 0,
			.visibility = wgpu::ShaderStage::Compute,
			.buffer = {
				.type = wgpu::BufferBindingType::Uniform,
				.minBindingSize = sizeof(structs::Light),
			},
		};
		std::array<wgpu::BindGroupLayoutEntry, 1> bindGroupLayoutEntries = {
			lightBindGroupLayoutEntry
		};

		const wgpu::BindGroupLayoutDescriptor bindGroupLayoutDescriptor = {
			.label = "lighting input bind group layout",
			.entryCount = bindGroupLayoutEntries.size(),
			.entries = bindGroupLayoutEntries.data(),
		};
		_inputBindGroupLayout = _wgpuContext->device.CreateBindGroupLayout(&bindGroupLayoutDescriptor);
	}

	void Lighting::createComputePipeline() {
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

	wgpu::PipelineLayout Lighting::getPipelineLayout() {
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
	}

	void Lighting::createAccumulatorBindGroup(
		const wgpu::TextureView& normalTextureView
	) {
		const wgpu::BindGroupEntry accumulatorBindGroupEntry = {
			.binding = 0,
			.textureView = lightTextureView,
		};
		const wgpu::BindGroupEntry normalBindGroupEntry = {
			.binding = 1,
			.textureView = normalTextureView,
		};

		std::array<wgpu::BindGroupEntry, 2> bindGroupEntries = {
			accumulatorBindGroupEntry,
			normalBindGroupEntry,
		};
		const wgpu::BindGroupDescriptor bindGroupDescriptor = {
			.label = "accumulator accumulator bind group",
			.layout = _accumulatorBindGroupLayout,
			.entryCount = bindGroupEntries.size(),
			.entries = bindGroupEntries.data(),
		};
		_accumulatorBindGroup = _wgpuContext->device.CreateBindGroup(&bindGroupDescriptor);
	}

	void Lighting::insertInputBindGroup(
		const wgpu::Buffer& lightBuffer
	) {
		const wgpu::BindGroupEntry lightBindGroupEntry = {
			.binding = 0,
			.buffer = lightBuffer,
		};
		std::array<wgpu::BindGroupEntry, 1> bindGroupEntries = {
			lightBindGroupEntry,
		};
		const wgpu::BindGroupDescriptor bindGroupDescriptor = {
			.label = "accumulator input bind group",
			.layout = _inputBindGroupLayout,
			.entryCount = bindGroupEntries.size(),
			.entries = bindGroupEntries.data(),
		};
		_inputBindGroups.emplace_back(_wgpuContext->device.CreateBindGroup(&bindGroupDescriptor));
	}



}
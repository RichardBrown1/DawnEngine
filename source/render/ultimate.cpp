#pragma once
#include "ultimate.hpp"
#include <array>
#include "../device/device.hpp"
#include "../enums.hpp"
#include "vertexBufferLayout.hpp"
#include "../texture/texture.hpp"

namespace render {
	Ultimate::Ultimate(wgpu::Device* device) {
		_device = device;
		_screenDimensions = wgpu::Extent2D(0, 0); //generateGpuObjects() will handle this

		_computeShaderModule = device::createShaderModule(*_device, ULTIMATE_SHADER_LABEL, ULTIMATE_SHADER_PATH);
	};

	void Ultimate::generateGpuObjects(const render::ultimate::descriptor::GenerateGpuObjects* descriptor) {
		assert(descriptor->screenDimensions.width > 1);
		_screenDimensions = descriptor->screenDimensions;

		createInputBindGroupLayout();
		createOutputBindGroupLayout();
		createPipeline();
		createInputBindGroup(
			descriptor->baseColorTextureView,
			descriptor->shadowMapTextureView
		);
	};

	void Ultimate::doCommands(const render::ultimate::descriptor::DoCommands* descriptor) {
		wgpu::BindGroup outputBindGroup = createOutputBindGroup(descriptor->surfaceTextureView);

		wgpu::ComputePassDescriptor computePassDescriptor = {
			.label = "ultimate compute pass",
		};
		wgpu::ComputePassEncoder computePassEncoder = descriptor->commandEncoder.BeginComputePass(&computePassDescriptor);
		computePassEncoder.SetPipeline(_computePipeline);
		computePassEncoder.SetBindGroup(0, _inputBindGroup);
		computePassEncoder.SetBindGroup(1, outputBindGroup);
		computePassEncoder.DispatchWorkgroups(_screenDimensions.width, _screenDimensions.height);
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
		_computePipeline = _device->CreateComputePipeline(&computePipelineDescriptor);
	}

	void Ultimate::createInputBindGroup(
		wgpu::TextureView& baseColorTextureView,
		wgpu::TextureView& shadowMapTextureView
	) {
		const wgpu::BindGroupEntry baseColorBindGroupEntry = {
			.binding = 0,
			.textureView = baseColorTextureView,
		};
		const wgpu::BindGroupEntry shadowMapBindGroupEntry = {
			.binding = 1,
			.textureView = shadowMapTextureView,
		};
		std::array<wgpu::BindGroupEntry, 2> bindGroupEntries = {
			baseColorBindGroupEntry,
			shadowMapBindGroupEntry,
		};
		const wgpu::BindGroupDescriptor bindGroupDescriptor = {
			.label = "ultimate render group",
			.layout = _inputBindGroupLayout,
			.entryCount = bindGroupEntries.size(),
			.entries = bindGroupEntries.data(),
		};
		_inputBindGroup = _device->CreateBindGroup(&bindGroupDescriptor);
	}

	wgpu::BindGroup Ultimate::createOutputBindGroup(
		wgpu::TextureView& surfaceTextureView
	) {
		const wgpu::BindGroupEntry surfaceBindGroupEntry = {
			.binding = 0,
			.textureView = surfaceTextureView,
		};
		std::array<wgpu::BindGroupEntry, 1> bindGroupEntries = {
			surfaceBindGroupEntry
		};
		const wgpu::BindGroupDescriptor bindGroupDescriptor = {
			.label = "ultimate render group",
			.layout = _outputBindGroupLayout,
			.entryCount = bindGroupEntries.size(),
			.entries = bindGroupEntries.data(),
		};
		return _device->CreateBindGroup(&bindGroupDescriptor);
	}

	void Ultimate::createInputBindGroupLayout() {
		const wgpu::BindGroupLayoutEntry baseColorBindGroupLayoutEntry = {
			.binding = 0,
			.visibility = wgpu::ShaderStage::Compute,
			.texture = {
				.sampleType = wgpu::TextureSampleType::Float,
				.viewDimension = wgpu::TextureViewDimension::e2D,
			},
		};
		const wgpu::BindGroupLayoutEntry shadowMapBindGroupLayoutEntry = {
			.binding = 1,
			.visibility = wgpu::ShaderStage::Compute,
			.texture = {
				.sampleType = wgpu::TextureSampleType::Float,
				.viewDimension = wgpu::TextureViewDimension::e2D,
			},
		};

		std::array<wgpu::BindGroupLayoutEntry, 2> bindGroupLayoutEntries = {
			baseColorBindGroupLayoutEntry,
			shadowMapBindGroupLayoutEntry,
		};

		const wgpu::BindGroupLayoutDescriptor bindGroupLayoutDescriptor = {
			.label = "ultimate input bind group layout",
			.entryCount = bindGroupLayoutEntries.size(),
			.entries = bindGroupLayoutEntries.data(),
		};
		_inputBindGroupLayout = _device->CreateBindGroupLayout(&bindGroupLayoutDescriptor);
	};

	void Ultimate::createOutputBindGroupLayout() {
		const wgpu::BindGroupLayoutEntry surfaceBindGroupLayoutEntry = {
			.binding = 0,
			.visibility = wgpu::ShaderStage::Compute,
			.texture = {
				.sampleType = wgpu::TextureSampleType::Float,
				.viewDimension = wgpu::TextureViewDimension::e2D,
			},
		};
		const wgpu::BindGroupLayoutDescriptor bindGroupLayoutDescriptor = {
			.label = "ultimate output bind group layout",
			.entryCount = 1,
			.entries = &surfaceBindGroupLayoutEntry,
		};
		_outputBindGroupLayout = _device->CreateBindGroupLayout(&bindGroupLayoutDescriptor);
	}

	wgpu::PipelineLayout Ultimate::getPipelineLayout() {
		std::array<wgpu::BindGroupLayout, 2> bindGroupLayout = {
			_inputBindGroupLayout,
			_outputBindGroupLayout,
		};
		const wgpu::PipelineLayoutDescriptor pipelineLayoutDescriptor = {
			.label = "ultimate render pipeline layout",
			.bindGroupLayoutCount = bindGroupLayout.size(),
			.bindGroupLayouts = bindGroupLayout.data(),
		};
		return _device->CreatePipelineLayout(&pipelineLayoutDescriptor);
	};

}

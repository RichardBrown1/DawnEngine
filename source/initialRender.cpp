#pragma once
#include "initialRender.hpp"
#include <array>
#include "utilities.hpp"

namespace DawnEngine {
	InitialRender::InitialRender(const InitialRenderDescriptor* descriptor) {
		_device = descriptor->device;
		_queue = _device.GetQueue();

		_initialRenderVertexShaderModule = Utilities::createShaderModule(
			_device,
			VERTEX_SHADER_LABEL,
			INITIAL_RENDER_SHADER_PATH
		);
	};

	wgpu::RenderPipeline InitialRender::createPipeline(const InitialRenderPipelineDescriptor* descriptor) {
		const wgpu::PipelineLayout pipelineLayout = getPipelineLayout();
		
		wgpu::ColorTargetState textureMasterInfoColorTargetState = {
			.format = wgpu::TextureFormat::RGBA32Uint,
		};
		wgpu::ColorTargetState baseColorColorTargetState = {
			.format = wgpu::TextureFormat::RGBA16Unorm,
		};
		const std::array<wgpu::ColorTargetState, 2> colorTargetStates = {
			textureMasterInfoColorTargetState,
			baseColorColorTargetState,
		};
		const wgpu::FragmentState fragmentState = {
			.targetCount = colorTargetStates.size(),
			.targets = colorTargetStates.data(),
		};
		const wgpu::RenderPipelineDescriptor renderPipelineDescriptor = {
			.label = "input render pipeline",
			.layout = pipelineLayout,
	//		.vertex = vertexState,
			.fragment = &fragmentState,
//			.primitive = primitiveState,
	//		.depthStencilState = depthStencilState,
		};
		_device.CreateRenderPipeline(&renderPipelineDescriptor);
	}

	wgpu::PipelineLayout InitialRender::getPipelineLayout() {
		const wgpu::BindGroupLayoutEntry cameraBindGroupLayoutEntry = {
			.binding = 0,
			.visibility = wgpu::ShaderStage::Vertex,
			.buffer = {
				.type = wgpu::BufferBindingType::Uniform,
				.minBindingSize = sizeof(glm::f32mat4x4),
			},
		};
		const wgpu::BindGroupLayoutEntry transformsBindGroupLayoutEntry = {
			.visibility = wgpu::ShaderStage::Vertex,
			.buffer = {
				.type = wgpu::BufferBindingType::ReadOnlyStorage,
				.minBindingSize = sizeof(glm::f32mat4x4),
			},
		};
		const wgpu::BindGroupLayoutEntry instancePropertyBindGroupLayoutEntry = {
			.visibility = wgpu::ShaderStage::Vertex,
			.buffer = {
				.type = wgpu::BufferBindingType::ReadOnlyStorage,
				.minBindingSize = sizeof(InstanceProperty),
			}
		};
		const wgpu::BindGroupLayoutEntry materialsBindGroupLayoutEntry = {
			.visibility = wgpu::ShaderStage::Vertex,
			.buffer = {
				.type = wgpu::BufferBindingType::ReadOnlyStorage,
				.minBindingSize = sizeof(Material),
			},
		};

		std::array<wgpu::BindGroupLayoutEntry, 4> bindGroupLayoutEntries = {
			cameraBindGroupLayoutEntry,
			transformsBindGroupLayoutEntry,
			instancePropertyBindGroupLayoutEntry,
			materialsBindGroupLayoutEntry,
		};
		const wgpu::BindGroupLayoutDescriptor bindGroupLayoutDescriptor = {
			.entryCount = bindGroupLayoutEntries.size(),
			.entries = bindGroupLayoutEntries.data(),
		};
		const wgpu::BindGroupLayout bindGroupLayout = _device.CreateBindGroupLayout(&bindGroupLayoutDescriptor);

		const wgpu::PipelineLayoutDescriptor pipelineLayoutDescriptor = {
			.label = "initial render pipeline layout",
			.bindGroupLayoutCount = 1,
			.bindGroupLayouts = &bindGroupLayout,
		};
		return _device.CreatePipelineLayout(&pipelineLayoutDescriptor);
	};
}
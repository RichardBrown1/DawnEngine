#pragma once
#include <array>

#include "../include/renderPipelineHelper.hpp"
#include "../include/constants.hpp"
#include "../include/gpuObjectManager.hpp"
#include "../include/utilities.hpp"

namespace {
	wgpu::BindGroupLayout initFixedBindGroupLayout(RenderPipelineHelper::RenderPipelineHelperDescriptor& descriptor) {
		std::vector<DawnEngine::GPU_OBJECT_ID> gpuObjectIds = {
			DawnEngine::GPU_OBJECT_ID::CAMERA,
			DawnEngine::GPU_OBJECT_ID::TRANSFORMS,
			DawnEngine::GPU_OBJECT_ID::INSTANCE_PROPERTIES,
			DawnEngine::GPU_OBJECT_ID::MATERIALS,
			DawnEngine::GPU_OBJECT_ID::LIGHTS,
			DawnEngine::GPU_OBJECT_ID::SHADOW_MAPS,
			DawnEngine::GPU_OBJECT_ID::DEPTH_SAMPLER,
			DawnEngine::GPU_OBJECT_ID::TEXTURES,
		};

		auto p_GpuObjectManager = DawnEngine::GpuObjectManager::instance().get();
		wgpu::BindGroupLayout bindGroupLayout = p_GpuObjectManager->getBindGroupLayout(
			descriptor.device, "output bind group layout", std::span{ gpuObjectIds }
		);

		std::vector<wgpu::BindGroupEntry> bindGroupEntries = {
			{
				.binding = +DawnEngine::GPU_OBJECT_ID::CAMERA,
				.buffer = descriptor.buffers.camera,
				.size = descriptor.buffers.camera.GetSize(),
			},
			{
				.binding = +DawnEngine::GPU_OBJECT_ID::TRANSFORMS,
				.buffer = descriptor.buffers.transform,
				.size = descriptor.buffers.transform.GetSize(),
			},
			{
				.binding = +DawnEngine::GPU_OBJECT_ID::INSTANCE_PROPERTIES,
				.buffer = descriptor.buffers.instanceProperties,
				.size = descriptor.buffers.instanceProperties.GetSize(),
			},
			{
				.binding = +DawnEngine::GPU_OBJECT_ID::MATERIALS,
				.buffer = descriptor.buffers.material,
				.size = descriptor.buffers.material.GetSize(),
			},
			{
				.binding = +DawnEngine::GPU_OBJECT_ID::LIGHTS,
				.buffer = descriptor.buffers.light,
				.size = descriptor.buffers.light.GetSize(),
			},
			{
				.binding = +DawnEngine::GPU_OBJECT_ID::SHADOW_MAPS,
				.textureView = descriptor.textureViews.shadowMaps[0],
			},
			{
				.binding = +DawnEngine::GPU_OBJECT_ID::DEPTH_SAMPLER,
				.sampler = descriptor.depthSampler,
			},
			{
				.binding = +DawnEngine::GPU_OBJECT_ID::TEXTURES,
				.textureView = descriptor.textureViews.textures,
			}
		};

		const wgpu::BindGroupDescriptor bindGroupDescriptor = {
			.label = "output bind group",
			.layout = bindGroupLayout,
			.entryCount = bindGroupEntries.size(),
			.entries = bindGroupEntries.data() ,
		};
		descriptor.bindGroups.fixed = descriptor.device.CreateBindGroup(&bindGroupDescriptor);

		return bindGroupLayout;
	}

	wgpu::BindGroupLayout initLightBindGroupLayout(RenderPipelineHelper::RenderPipelineHelperDescriptor& descriptor) {
		std::vector<DawnEngine::GPU_OBJECT_ID> gpuObjectIds = {
			DawnEngine::GPU_OBJECT_ID::TRANSFORMS,
			DawnEngine::GPU_OBJECT_ID::LIGHTS,
		};

		auto p_GpuObjectManager = DawnEngine::GpuObjectManager::instance().get();
		wgpu::BindGroupLayout bindGroupLayout = p_GpuObjectManager->getBindGroupLayout(
			descriptor.device, "lights bind group layout", std::span{ gpuObjectIds }
		);

		std::vector<wgpu::BindGroupEntry> bindGroupEntries = {
			{
				.binding = +DawnEngine::GPU_OBJECT_ID::TRANSFORMS,
				.buffer = descriptor.buffers.transform,
				.size = descriptor.buffers.transform.GetSize(),
			},
			{
				.binding = +DawnEngine::GPU_OBJECT_ID::LIGHTS,
				.buffer = descriptor.buffers.light,
				.size = descriptor.buffers.light.GetSize(),
			},
		};

		const wgpu::BindGroupDescriptor bindGroupDescriptor = {
			.label = "output bind group",
			.layout = bindGroupLayout,
			.entryCount = bindGroupEntries.size(),
			.entries = bindGroupEntries.data() ,
		};
		descriptor.bindGroups.lights = descriptor.device.CreateBindGroup(&bindGroupDescriptor);

		return bindGroupLayout;
	}

	wgpu::PipelineLayout initOutputPipelineLayout(RenderPipelineHelper::RenderPipelineHelperDescriptor& descriptor) {
		std::array<wgpu::BindGroupLayout, 1> bindGroupLayouts = { initFixedBindGroupLayout(descriptor) };
		wgpu::PipelineLayoutDescriptor pipelineLayoutDescriptor = {
			.label = "Geometry Pipeline Layout",
			.bindGroupLayoutCount = 1,
			.bindGroupLayouts = bindGroupLayouts.data(),
		};
		return descriptor.device.CreatePipelineLayout(&pipelineLayoutDescriptor);
	}

	wgpu::PipelineLayout initShadowPipelineLayout(RenderPipelineHelper::RenderPipelineHelperDescriptor& descriptor) {
		std::array<wgpu::BindGroupLayout, 1> bindGroupLayouts = { initLightBindGroupLayout(descriptor) };
		wgpu::PipelineLayoutDescriptor pipelineLayoutDescriptor = {
			.label = "Shadow Pipeline Layout",
			.bindGroupLayoutCount = 1,
			.bindGroupLayouts = bindGroupLayouts.data(),
		};
		return descriptor.device.CreatePipelineLayout(&pipelineLayoutDescriptor);
	}

}

namespace RenderPipelineHelper {

	wgpu::RenderPipeline RenderPipelineHelper::createOutputRenderPipeline(RenderPipelineHelperDescriptor& descriptor)
	{
		constexpr wgpu::VertexAttribute positionAttribute = {
			.format = wgpu::VertexFormat::Float32x3,
			.offset = 0,
			.shaderLocation = 0,
		};
		constexpr wgpu::VertexAttribute normalAttribute = {
			.format = wgpu::VertexFormat::Float32x3,
			.offset = offsetof(DawnEngine::VBO, normal),
			.shaderLocation = 1,
		};
		const auto vertexAttributes = std::vector<wgpu::VertexAttribute>{ positionAttribute, normalAttribute };
		const	wgpu::VertexBufferLayout vertexBufferLayout = {
			.arrayStride = sizeof(DawnEngine::VBO),
			.attributeCount = vertexAttributes.size(),
			.attributes = vertexAttributes.data(),
		};
		const wgpu::VertexState vertexState = {
			.module = descriptor.vertexShaderModule,
			.entryPoint = "VS_main",
			.bufferCount = 1,
			.buffers = &vertexBufferLayout,
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
		const wgpu::ColorTargetState colorTargetState = {
			.format = descriptor.colorTargetStateFormat,
			.blend = &blendState,
			.writeMask = wgpu::ColorWriteMask::All,
		};
		const wgpu::FragmentState fragmentState = {
			.module = descriptor.fragmentShaderModule,
			.entryPoint = "FS_main",
			.constantCount = 0,
			.constants = nullptr,
			.targetCount = 1,
			.targets = &colorTargetState,
		};

		constexpr wgpu::DepthStencilState depthStencilState = {
			.format = DawnEngine::DEPTH_FORMAT,
			.depthWriteEnabled = true,
			.depthCompare = wgpu::CompareFunction::Less,
		};

		wgpu::RenderPipelineDescriptor renderPipelineDescriptor = {
			.label = "output render pipeline",
			.layout = initOutputPipelineLayout(descriptor),
			.vertex = vertexState,
			.primitive = wgpu::PrimitiveState {
				.topology = wgpu::PrimitiveTopology::TriangleList,
				.cullMode = wgpu::CullMode::Back,
			},
			.depthStencil = &depthStencilState,
			.multisample = wgpu::MultisampleState {
				.count = 1,
				.mask = ~0u,
				.alphaToCoverageEnabled = false,
			},
			.fragment = &fragmentState,
		};

		return descriptor.device.CreateRenderPipeline(&renderPipelineDescriptor);
	}

	wgpu::RenderPipeline RenderPipelineHelper::createShadowRenderPipeline(RenderPipelineHelperDescriptor& descriptor)
	{
		constexpr wgpu::VertexAttribute positionAttribute = {
			.format = wgpu::VertexFormat::Float32x3,
			.offset = 0,
			.shaderLocation = 0,
		};
		constexpr wgpu::VertexAttribute normalAttribute = {
			.format = wgpu::VertexFormat::Float32x3,
			.offset = offsetof(DawnEngine::VBO, normal),
			.shaderLocation = 1,
		};
		const auto vertexAttributes = std::vector<wgpu::VertexAttribute>{ positionAttribute, normalAttribute };
		const wgpu::VertexBufferLayout vertexBufferLayout = {
			.arrayStride = sizeof(DawnEngine::VBO),
			.attributeCount = vertexAttributes.size(),
			.attributes = vertexAttributes.data(),
		};
		const wgpu::VertexState vertexState = {
			.module = descriptor.vertexShaderModule,
			.entryPoint = "VS_main",
			.bufferCount = 1,
			.buffers = &vertexBufferLayout,
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
			.module = descriptor.fragmentShaderModule,
			.entryPoint = "FS_main",
			.constantCount = 0,
			.constants = nullptr,
		};

		constexpr wgpu::DepthStencilState depthStencilState = {
			.format = DawnEngine::DEPTH_FORMAT,
			.depthWriteEnabled = true,
			.depthCompare = wgpu::CompareFunction::Less,
		};

		wgpu::RenderPipelineDescriptor renderPipelineDescriptor = {
			.label = "shadow render pipeline descriptor",
			.layout = initShadowPipelineLayout(descriptor),
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

		return descriptor.device.CreateRenderPipeline(&renderPipelineDescriptor);
	}
}


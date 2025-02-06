#pragma once
#include <array>

#include "../include/renderPipelineHelper.hpp"
#include "../include/constants.hpp"

namespace {
	wgpu::BindGroupLayout initStaticBindGroupLayout(RenderPipelineHelper::RenderPipelineHelperDescriptor &descriptor) {
		wgpu::BindGroupLayoutEntry cameraBindGroupLayoutEntry = {
			.binding = 0,
			.visibility = (wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment),
			.buffer = {
				.type = wgpu::BufferBindingType::Uniform,
				.minBindingSize = sizeof(Camera),
			}
		};
		wgpu::BindGroupLayoutEntry transformsBindGroupLayoutEntry = {
			.binding = 1,
			.visibility = wgpu::ShaderStage::Vertex,
			.buffer = {
				.type = wgpu::BufferBindingType::ReadOnlyStorage,
				.minBindingSize = sizeof(glm::f32mat4x4),
			}
		};
		wgpu::BindGroupLayoutEntry instancePropertiesBindGroupLayoutEntry = {
			.binding = 2,
			.visibility = wgpu::ShaderStage::Vertex,
			.buffer = {
				.type = wgpu::BufferBindingType::ReadOnlyStorage,
				.minBindingSize = sizeof(InstanceProperty),
			}
		};
		std::array<wgpu::BindGroupLayoutEntry, 3> bindGroupLayoutEntries = {
			cameraBindGroupLayoutEntry,
			transformsBindGroupLayoutEntry,
			instancePropertiesBindGroupLayoutEntry,
		};

		wgpu::BindGroupLayoutDescriptor bindGroupLayoutDescriptor = {
			.label = "static bind group",
			.entryCount = bindGroupLayoutEntries.size(),
			.entries = bindGroupLayoutEntries.data(),
		};
		wgpu::BindGroupLayout bindGroupLayout = descriptor.device.CreateBindGroupLayout(&bindGroupLayoutDescriptor);


		wgpu::BindGroupEntry cameraBindGroupEntry = {
			.binding = 0,
			.buffer = descriptor.buffers.camera,
			.size = descriptor.buffers.camera.GetSize(),
		};
		wgpu::BindGroupEntry transformsBindGroupEntry = {
			.binding = 1,
			.buffer = descriptor.buffers.transform,
			.size = descriptor.buffers.transform.GetSize(),
		};
		wgpu::BindGroupEntry instancePropertiesBindGroupEntry = {
			.binding = 2,
			.buffer = descriptor.buffers.instanceProperties,
			.size = descriptor.buffers.instanceProperties.GetSize(),
		};
		std::array<wgpu::BindGroupEntry, 3> bindGroupEntries = {
			cameraBindGroupEntry,
			transformsBindGroupEntry,
			instancePropertiesBindGroupEntry,
		};

		wgpu::BindGroupDescriptor bindGroupDescriptor = {
			.label = "static bind group",
			.layout = bindGroupLayout,
			.entryCount = bindGroupEntries.size(),
			.entries = bindGroupEntries.data() ,
		};
		descriptor.bindGroups.push_back(descriptor.device.CreateBindGroup(&bindGroupDescriptor));

		return bindGroupLayout;
	}

	wgpu::BindGroupLayout initInfrequentBindGroupLayout(RenderPipelineHelper::RenderPipelineHelperDescriptor descriptor) {
		wgpu::BindGroupLayoutEntry materialBindGroupLayoutEntry = {
			.binding = 0,
			.visibility = wgpu::ShaderStage::Vertex,
			.buffer = {
				.type = wgpu::BufferBindingType::ReadOnlyStorage,
				.minBindingSize = sizeof(Material),
			}
		};
		wgpu::BindGroupLayoutEntry lightBindGroupLayoutEntry = {
			.binding = 1,
			.visibility = wgpu::ShaderStage::Fragment,
			.buffer = {
				.type = wgpu::BufferBindingType::ReadOnlyStorage,
				.minBindingSize = sizeof(Light),
			}
		};
		std::array<wgpu::BindGroupLayoutEntry, 2> bindGroupLayoutEntries = {
			materialBindGroupLayoutEntry,
			lightBindGroupLayoutEntry,
		};

		wgpu::BindGroupLayoutDescriptor bindGroupLayoutDescriptor = {
			.label = "infrequent bind group",
			.entryCount = bindGroupLayoutEntries.size(),
			.entries = bindGroupLayoutEntries.data(),
		};
		wgpu::BindGroupLayout bindGroupLayout = descriptor.device.CreateBindGroupLayout(&bindGroupLayoutDescriptor);

		wgpu::BindGroupEntry materialBindGroupEntry = {
			.binding = 0,
			.buffer = descriptor.buffers.material,
			.size = descriptor.buffers.material.GetSize(),
		};
		wgpu::BindGroupEntry lightBindGroupEntry = {
			.binding = 1,
			.buffer = descriptor.buffers.light,
			.size = descriptor.buffers.light.GetSize(),
		};
		std::array<wgpu::BindGroupEntry, 2> bindGroupEntries = {
			materialBindGroupEntry,
			lightBindGroupEntry,
		};

		wgpu::BindGroupDescriptor bindGroupDescriptor = {
			.label = "infrequent bind group",
			.layout = bindGroupLayout,
			.entryCount = bindGroupLayoutDescriptor.entryCount,
			.entries = bindGroupEntries.data(),
		};
		descriptor.bindGroups.push_back(descriptor.device.CreateBindGroup(&bindGroupDescriptor));
		return bindGroupLayout;
	}

	wgpu::PipelineLayout initPipelineLayout(RenderPipelineHelper::RenderPipelineHelperDescriptor &descriptor) {
		std::array<wgpu::BindGroupLayout, 2> bindGroupLayouts = { initStaticBindGroupLayout(descriptor), initInfrequentBindGroupLayout(descriptor) };
		wgpu::PipelineLayoutDescriptor pipelineLayoutDescriptor = {
			.label = "Pipeline Layout",
			.bindGroupLayoutCount = 2,
			.bindGroupLayouts = bindGroupLayouts.data(),
		};
		return descriptor.device.CreatePipelineLayout(&pipelineLayoutDescriptor);
	}
}

namespace RenderPipelineHelper {

	wgpu::RenderPipeline RenderPipelineHelper::createRenderPipeline(RenderPipelineHelperDescriptor& descriptor)
	{
		wgpu::VertexAttribute positionAttribute = {
			.format = wgpu::VertexFormat::Float32x3,
			.offset = 0,
			.shaderLocation = 0,
		};
		wgpu::VertexAttribute normalAttribute = {
			.format = wgpu::VertexFormat::Float32x3,
			.offset = offsetof(VBO, normal),
			.shaderLocation = 1,
		};
		auto vertexAttributes = std::vector<wgpu::VertexAttribute>{ positionAttribute, normalAttribute };
		wgpu::VertexBufferLayout vertexBufferLayout = {
			.arrayStride = sizeof(VBO),
			.attributeCount = vertexAttributes.size(),
			.attributes = vertexAttributes.data(),
		};
		wgpu::VertexState vertexState = {
			.module = descriptor.vertexShaderModule,
			.entryPoint = "VS_main",
			.bufferCount = 1,
			.buffers = &vertexBufferLayout,
		};

		wgpu::BlendState blendState = {
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
		wgpu::ColorTargetState colorTargetState = {
			.format = descriptor.colorTargetStateFormat,
			.blend = &blendState,
			.writeMask = wgpu::ColorWriteMask::All,
		};
		wgpu::FragmentState fragmentState = {
			.module = descriptor.fragmentShaderModule,
			.entryPoint = "FS_main",
			.constantCount = 0,
			.constants = nullptr,
			.targetCount = 1,
			.targets = &colorTargetState,
		};

		wgpu::DepthStencilState depthStencilState = {
			.format = DawnEngine::DEPTH_FORMAT,
			.depthWriteEnabled = true,
			.depthCompare = wgpu::CompareFunction::Less,
		};

		wgpu::RenderPipelineDescriptor renderPipelineDescriptor = {
			.label = "render pipeline descriptor",
			.layout = initPipelineLayout(descriptor),
			.vertex = vertexState,
			.primitive = wgpu::PrimitiveState {
				.topology = wgpu::PrimitiveTopology::TriangleList,
				.cullMode = wgpu::CullMode::None,
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


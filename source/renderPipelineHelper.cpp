#pragma once
#include <array>

#include "../include/renderPipelineHelper.hpp"
#include "../include/constants.hpp"

namespace {
	wgpu::BindGroupLayout initFixedBindGroupLayout(RenderPipelineHelper::RenderPipelineHelperDescriptor &descriptor) {
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
		wgpu::BindGroupLayoutEntry materialBindGroupLayoutEntry = {
			.binding = 3,
			.visibility = wgpu::ShaderStage::Vertex,
			.buffer = {
				.type = wgpu::BufferBindingType::ReadOnlyStorage,
				.minBindingSize = sizeof(Material),
			}
		};
		wgpu::BindGroupLayoutEntry lightBindGroupLayoutEntry = {
			.binding = 4,
			.visibility = wgpu::ShaderStage::Fragment,
			.buffer = {
				.type = wgpu::BufferBindingType::ReadOnlyStorage,
				.minBindingSize = sizeof(Light),
			}
		};

		std::array<wgpu::BindGroupLayoutEntry, 5> bindGroupLayoutEntries = {
			cameraBindGroupLayoutEntry,
			transformsBindGroupLayoutEntry,
			instancePropertiesBindGroupLayoutEntry,
			materialBindGroupLayoutEntry,
			lightBindGroupLayoutEntry,
		};

		wgpu::BindGroupLayoutDescriptor bindGroupLayoutDescriptor = {
			.label = "geometry bind group",
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
		wgpu::BindGroupEntry materialBindGroupEntry = {
			.binding = 3,
			.buffer = descriptor.buffers.material,
			.size = descriptor.buffers.material.GetSize(),
		};
		wgpu::BindGroupEntry lightBindGroupEntry = {
			.binding = 4,
			.buffer = descriptor.buffers.light,
			.size = descriptor.buffers.light.GetSize(),
		};


		std::array<wgpu::BindGroupEntry, 5> bindGroupEntries = {
			cameraBindGroupEntry,
			transformsBindGroupEntry,
			instancePropertiesBindGroupEntry,
			materialBindGroupEntry,
			lightBindGroupEntry,
		};

		wgpu::BindGroupDescriptor bindGroupDescriptor = {
			.label = "geometry bind group",
			.layout = bindGroupLayout,
			.entryCount = bindGroupEntries.size(),
			.entries = bindGroupEntries.data() ,
		};
		descriptor.bindGroups.fixed = descriptor.device.CreateBindGroup(&bindGroupDescriptor);

		return bindGroupLayout;
	}

	wgpu::BindGroupLayout initLightBindGroupLayout(RenderPipelineHelper::RenderPipelineHelperDescriptor &descriptor) {
		wgpu::BindGroupLayoutEntry transformsBindGroupLayoutEntry = {
			.binding = 0,
			.visibility = wgpu::ShaderStage::Vertex,
			.buffer = {
				.type = wgpu::BufferBindingType::ReadOnlyStorage,
				.minBindingSize = sizeof(glm::f32mat4x4),
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
			transformsBindGroupLayoutEntry,
			lightBindGroupLayoutEntry,
		};

		wgpu::BindGroupLayoutDescriptor bindGroupLayoutDescriptor = {
			.label = "lights bind group layout",
			.entryCount = bindGroupLayoutEntries.size(),
			.entries = bindGroupLayoutEntries.data(),
		};
		wgpu::BindGroupLayout bindGroupLayout = descriptor.device.CreateBindGroupLayout(&bindGroupLayoutDescriptor);


		wgpu::BindGroupEntry transformsBindGroupEntry = {
			.binding = 0,
			.buffer = descriptor.buffers.transform,
			.size = descriptor.buffers.transform.GetSize(),
		};
		wgpu::BindGroupEntry lightBindGroupEntry = {
			.binding = 1,
			.buffer = descriptor.buffers.light,
			.size = descriptor.buffers.light.GetSize(),
		};


		std::array<wgpu::BindGroupEntry, 2> bindGroupEntries = {
			transformsBindGroupEntry,
			lightBindGroupEntry,
		};

		wgpu::BindGroupDescriptor bindGroupDescriptor = {
			.label = "lights bind group",
			.layout = bindGroupLayout,
			.entryCount = bindGroupEntries.size(),
			.entries = bindGroupEntries.data() ,
		};
		descriptor.bindGroups.lights = descriptor.device.CreateBindGroup(&bindGroupDescriptor);

		return bindGroupLayout;
	}

	
	wgpu::PipelineLayout initOutputPipelineLayout(RenderPipelineHelper::RenderPipelineHelperDescriptor &descriptor) {
		std::array<wgpu::BindGroupLayout, 1> bindGroupLayouts = { initFixedBindGroupLayout(descriptor)};
		wgpu::PipelineLayoutDescriptor pipelineLayoutDescriptor = {
			.label = "Geometry Pipeline Layout",
			.bindGroupLayoutCount = 1,
			.bindGroupLayouts = bindGroupLayouts.data(),
		};
		return descriptor.device.CreatePipelineLayout(&pipelineLayoutDescriptor);
	}

	wgpu::PipelineLayout initShadowPipelineLayout(RenderPipelineHelper::RenderPipelineHelperDescriptor &descriptor) {
		std::array<wgpu::BindGroupLayout, 1> bindGroupLayouts = { initLightBindGroupLayout(descriptor)};
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
			.label = "output render pipeline",
			.layout = initOutputPipelineLayout(descriptor),
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

	wgpu::RenderPipeline RenderPipelineHelper::createShadowRenderPipeline(RenderPipelineHelperDescriptor& descriptor)
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
			.label = "shadow render pipeline descriptor",
			.layout = initShadowPipelineLayout(descriptor),
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


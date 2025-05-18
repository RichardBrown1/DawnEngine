#pragma once
#include "shadow.hpp"
#include "vertexBufferLayout.hpp"
#include "../device/device.hpp"
#include "../enums.hpp"
#include "../constants.hpp"

namespace render {

	Shadow::Shadow(wgpu::Device* device) {
		_device = device;

		_vertexShaderModule = device::createShaderModule(*_device, VERTEX_SHADER_LABEL, VERTEX_SHADER_PATH);
		_fragmentShaderModule = device::createShaderModule(*_device, FRAGMENT_SHADER_LABEL, FRAGMENT_SHADER_PATH);
	}

	void Shadow::generateGpuObjects(const render::shadow::descriptor::GenerateGpuObjects* descriptor) {
		createPipeline();
	}

	void Shadow::doCommands(const render::shadow::descriptor::DoCommands* descriptor) {

	}

	void Shadow::createPipeline() {
		{
		const wgpu::VertexState vertexState = {
			.module = _vertexShaderModule,
			.entryPoint = "VS_main",
			.bufferCount = 1,
			.buffers = &render::vertexBufferLayout,
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
			.module = _fragmentShaderModule,
			.entryPoint = enums::EntryPoint::FRAGMENT,
			.constantCount = 0,
			.constants = nullptr,
		};

		constexpr wgpu::DepthStencilState depthStencilState = {
			.format = constants::DEPTH_FORMAT,
			.depthWriteEnabled = true,
			.depthCompare = wgpu::CompareFunction::Less,
		};

		wgpu::RenderPipelineDescriptor renderPipelineDescriptor = {
			.label = "shadow render pipeline descriptor",
			.layout = initPipelineLayout(descriptor),
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

	}
}
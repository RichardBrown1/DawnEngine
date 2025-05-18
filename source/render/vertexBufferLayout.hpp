#pragma once
#include <array>
#include <webgpu/webgpu_cpp.h>
#include "../structs/structs.hpp"

namespace {
	constexpr wgpu::VertexAttribute positionAttribute = {
			.format = wgpu::VertexFormat::Float32x3,
			.offset = 0,
			.shaderLocation = 0,
	};
	constexpr wgpu::VertexAttribute normalAttribute = {
		.format = wgpu::VertexFormat::Float32x3,
		.offset = offsetof(structs::VBO, normal),
		.shaderLocation = 1,
	};
	constexpr wgpu::VertexAttribute texcoordAttribute = {
		.format = wgpu::VertexFormat::Float32x2,
		.offset = offsetof(structs::VBO, texcoord),
		.shaderLocation = 2,
	};
	constexpr auto vertexAttributes = std::array<wgpu::VertexAttribute, 3>{
		positionAttribute,
		normalAttribute,
		texcoordAttribute
	};
}

namespace render {
	constexpr	wgpu::VertexBufferLayout vertexBufferLayout = {
		.arrayStride = sizeof(structs::VBO),
		.attributeCount = vertexAttributes.size(),
		.attributes = vertexAttributes.data(),
	};
}

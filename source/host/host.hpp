#pragma once
#include <vector>
#include <dawn/webgpu_cpp.h>
#include "structs.hpp"
#include "../device/structs.hpp"
#include "../engine.hpp"
#include "createBuffer.hpp"


//Objects for the wgpu::Device but in RAM waiting to be processed
//This data should be in a format that can be consumed by the shader if its written into the device as is
namespace {
	wgpu::Buffer createVboBuffer(
		Engine& engine,
		std::vector<host::structs::VBO> h_vbo
	) {
		const wgpu::BufferDescriptor vboBufferDescriptor = {
			.label = "vbo buffer",
			.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Vertex,
			.size = sizeof(host::structs::VBO) * h_vbo.size(),
		};
		wgpu::Buffer d_vbo = engine.device.CreateBuffer(&vboBufferDescriptor);
		engine.queue.WriteBuffer(d_vbo, 0, h_vbo.data(), vboBufferDescriptor.size);
		return d_vbo;
	}

}
namespace host {
	void ConvertHostObjects(Engine& engine, host::structs::Objects& h_objects, device::structs::Objects& d_objects) {
		d_objects.vbo = createVboBuffer(engine, h_objects.vbo);
		d_objects.transforms = createBuffer::storage<glm::f32mat4x4>(engine, h_objects.transforms, std::string("transforms"));
	}

}
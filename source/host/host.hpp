#pragma once
#include <vector>
#include <dawn/webgpu_cpp.h>
#include "../structs/host.hpp"
#include "../structs/device.hpp"
#include "../engine.hpp"
#include "../device/device.hpp"

//Objects for the wgpu::Device but in RAM waiting to be processed
//This data should be in a format that can be consumed by the shader if its written into the device as is

namespace host {
	void ConvertHostObjects(
		Engine& engine, 
		structs::host::Objects& h_objects,
		structs::device::Objects& d_objects
	) {
		d_objects.vbo = device::createBuffer<structs::VBO>(engine, h_objects.vbo, "vbo", wgpu::BufferUsage::Vertex);
		d_objects.indices = device::createBuffer<uint16_t>(
			engine, 
			h_objects.indices, 
			"indices", 
			wgpu::BufferUsage::Index
		);
		d_objects.transforms = device::createBuffer<glm::f32mat4x4>(
			engine,
			h_objects.transforms,
			"transforms",
			wgpu::BufferUsage::Storage
		);
		d_objects.instanceProperties = device::createBuffer<structs::InstanceProperty>(
			engine,
			h_objects.instanceProperties,
			"instance properties",
			wgpu::BufferUsage::Storage
		);
		d_objects.lights = device::createBuffer<structs::Light>(
			engine,
			h_objects.lights,
			"lights",
			wgpu::BufferUsage::Storage
		);
		
		//	d_objects.indices = createIndicesBuffer
	}

}
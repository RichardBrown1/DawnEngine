#pragma once
#include <vector>
#include <dawn/webgpu_cpp.h>
#include <glm/glm.hpp>
#include "../structs/host.hpp"
#include "../structs/device.hpp"
#include "../engine.hpp"
#include "../device/device.hpp"
#pragma once
#include "../constants.hpp"

//Objects for the wgpu::Device but in RAM waiting to be processed
//This data should be in a format that can be consumed by the shader if its written into the device as is
struct Host {
	//Mesh data
	std::vector<structs::VBO> vbo;
	std::vector<uint16_t> indices;
	std::vector<glm::f32mat4x4> transforms;
	std::vector<structs::InstanceProperty> instanceProperties;
	std::vector<structs::host::DrawCall> drawCalls;

	//Other data
	std::vector<structs::Light> lights;
	std::vector<structs::host::H_Camera> cameras;

	//Material related data
	std::vector<structs::Material> materials;
	std::vector<structs::SamplerTexturePair> samplerTexturePairs;
	std::vector<std::string> textureUris;
	std::vector<wgpu::SamplerDescriptor> samplers;

	structs::device::Objects ToDeviceObjects(Engine& engine);
	
};

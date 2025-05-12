#pragma once
#include <vector>
#include <string>
#include <dawn/webgpu_cpp.h>
#include <glm/fwd.hpp>
#include "../structs/host.hpp"
#include "../device/device.hpp"
#include "../engine.hpp"

//Objects for the wgpu::Device but in RAM waiting to be processed
//This data should be in a format that can be consumed by the shader if its written into the device as is
namespace host {
	struct SceneResources {
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

		device::SceneResources ToDevice(Engine& engine);
	};
}

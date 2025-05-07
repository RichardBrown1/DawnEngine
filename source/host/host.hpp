#pragma once
#include <vector>
#include <glm/fwd.hpp>
#include "structs.hpp"

//Objects for the wgpu::Device but in RAM waiting to be processed
//This data should be in a format that can be consumed by the shader if its written into the device as is

namespace host {
	struct Objects {
		//Mesh data
		std::vector<structs::VBO> vbos;
		std::vector<glm::f32mat4x4> transforms;
		std::vector<uint16_t> indices;
		std::vector<structs::InstanceProperty> instanceProperties;
		std::vector<structs::DrawCall> drawCalls;

		//Light data
		std::vector<structs::Light> lights;

		//Camera data
		std::vector<structs::H_Camera> cameras;
	};

}
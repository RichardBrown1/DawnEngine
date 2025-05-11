#pragma once
#include "structs.hpp"
	
namespace structs {
	namespace host {

		//Host Camera - This will be a f32mat4x4 projectionView at device
		struct H_Camera {
			glm::f32mat4x4 projection;
			glm::f32vec3 position;
			glm::f32vec3 forward;
		};

		struct DrawCall {
			uint32_t indexCount;
			uint32_t instanceCount;
			uint32_t firstIndex;
			uint32_t baseVertex;
			uint32_t firstInstance; //requires indirect-first-instance feature
		};

		struct Objects {
			//Mesh data
			std::vector<structs::VBO> vbo;
			std::vector<uint16_t> indices;
			std::vector<glm::f32mat4x4> transforms;
			std::vector<InstanceProperty> instanceProperties;
			std::vector<DrawCall> drawCalls;

			//Other data
			std::vector<Light> lights;
			std::vector<H_Camera> cameras;

			//Material related data
			std::vector<Material> materials;
			std::vector<SamplerTexturePair> samplerTexturePairs;
			std::vector<std::string> textureUris;
			std::vector<Sampler> samplers;
		};

	}
}
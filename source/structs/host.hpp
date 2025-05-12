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


	}
}
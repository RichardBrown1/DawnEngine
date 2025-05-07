#include <vector>
#include <glm/glm.hpp>
#include "structs.hpp"

//Objects for the wgpu::Device but in RAM waiting to be processed
//This data should be in a format that can be consumed by the shader if its written into the device as is

namespace host {
  namespace objects {
		std::vector<VBO> vbos;
	}

}
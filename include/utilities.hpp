#pragma once
#include <vector>
#include <string>
#include <fastgltf/core.hpp>
#include <glm/glm.hpp>

namespace Utilities {
	std::vector<uint32_t> readShader(const std::string& filename);

	void checkFastGltfError(const fastgltf::Error& error, const std::string& additionalMessage = "");

	glm::f32mat4x4 toGlmFormat(fastgltf::math::fmat4x4& matrix);
};

#pragma once
#include <vector>
#include <string>
#include <fastgltf/core.hpp>

namespace Utilities {
	std::vector<uint32_t> readShader(const std::string& filename);

	void checkFastGltfError(const fastgltf::Error& error, const std::string& additionalMessage = "");
};

#pragma once
#include "glm/glm.hpp"
#include "fastgltf/core.hpp"
#include "fastgltf/tools.hpp"
#include "fastgltf/types.hpp"
#include "absl/log/log.h"
#include "host/host.hpp"
#include "host/structs.hpp"

namespace gltf {
	fastgltf::Asset* getAsset(std::string& gltfFilePath);
	host::Objects processAsset(fastgltf::Asset& asset, std::array<uint32_t, 2> screenDimensions);
};
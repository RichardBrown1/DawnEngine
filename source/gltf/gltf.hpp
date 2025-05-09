#pragma once
#include "fastgltf/types.hpp"
#include "../host/host.hpp"

namespace gltf {
	fastgltf::Asset* getAsset(std::string& gltfDirectory, std::string& gltfFilePath);
	host::Objects processAsset(fastgltf::Asset& asset, std::array<uint32_t, 2> screenDimensions);
};
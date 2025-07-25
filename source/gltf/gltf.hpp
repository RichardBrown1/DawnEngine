#pragma once
#include "fastgltf/types.hpp"
#include "../host/host.hpp"

namespace gltf {
	fastgltf::Asset getAsset(const std::string& gltfDirectory, const std::string& gltfFilePath);
	void processAsset(HostSceneResources& sceneResources, fastgltf::Asset& asset, std::array<uint32_t, 2> screenDimensions, const std::string gltfDirectory);
};
#pragma once
#include "host.hpp"
#include "../device/device.hpp"
#include "../gltf/gltf.hpp"
#include <glm/ext/matrix_clip_space.hpp>

HostSceneResources::HostSceneResources(
	const std::string& gltfDirectory,
	const std::string& gltfFileName,
	const std::array<uint32_t, 2> screenDimensions) {
	fastgltf::Asset asset = gltf::getAsset(gltfDirectory, gltfFileName);
	gltf::processAsset(*this, asset, screenDimensions, gltfDirectory);
	addDefaults(screenDimensions);
	postProcessData();
};

//defaults if none found
void HostSceneResources::addDefaults(const std::array<uint32_t, 2> screenDimensions) {
	if (cameras.size() == 0) {
		cameras.push_back(structs::host::H_Camera{
			.projection = glm::perspectiveRH_ZO(45.0f, screenDimensions[0] / (float)screenDimensions[1], 0.00001f, 1024.0f),
			.position = { 0.0f, 0.0f, -0.1f },
			.forward = { 0.0f, 0.0f, 0.1f },
			});
	}
	if (lights.size() == 0) {
		lights.push_back(structs::Light{
			.rotation = glm::f32vec3{2.755f, -0.286f, -1.269f}, //Points downwards and slightly in +X and +Z
			.color = {1.0f, 1.0f, 0.9f},
			.type = 0, //Directional
			.intensity = 128.0f,
			});
	}
}

void HostSceneResources::postProcessData() {
	for (auto& m : materials) {
		const uint32_t baseColorStpId = m.pbrMetallicRoughness.baseColorTextureInfo.index;
		if (baseColorStpId != UINT32_MAX) {
			baseColorStpIds.emplace_back(baseColorStpId);
		}
		const uint32_t metallicRoughnessStpId = m.pbrMetallicRoughness.baseColorTextureInfo.index;
		if (metallicRoughnessStpId != UINT32_MAX) {
			metallicRoughnessStpIds.emplace_back(metallicRoughnessStpId);
		}
		const uint32_t normalStpId = m.normalTextureInfo.index;
		if (normalStpId != UINT32_MAX) {
			normalStpIds.emplace_back(normalStpId);
		}
	}
}

#pragma once
#include "host.hpp"
#include "../enums.hpp"	
#include "../device/device.hpp"
#include "../gltf/gltf.hpp"
#include <glm/glm.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include "../constants.hpp"

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
		const auto rotation = glm::f32vec3{ 1.26f, 0.0f, 1.269f }; //Points downwards and slightly in +X and +Z
		const float yaw = rotation.y;
		const float pitch = rotation.x;

		 // Calculate the front vector from the Euler angles
    glm::f32vec3 front;
    front.x = cos(yaw) * cos(pitch);
    front.y = sin(pitch);
    front.z = sin(yaw) * cos(pitch);
    front = glm::normalize(front);

    // Create the view matrix
		const auto position = glm::f32vec3{ 0.0f, 1.0f, 0.0f };
    const glm::vec3 target = position + front;
    const auto lightSpaceMatrix = glm::lookAt(position, target, constants::UP);

		lights.push_back(structs::Light{
			.lightSpaceMatrix = lightSpaceMatrix,
			.position = position,
			.rotation = rotation,
			.color = {1.0f, 1.0f, 0.9f},
			.type = enums::LightType::DIRECTIONAL,
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

#include "gltf.hpp"
#include "glm/glm.hpp"
#include "fastgltf/core.hpp"
#include "fastgltf/tools.hpp"
#include "fastgltf/types.hpp"
#include "absl/log/log.h"

namespace {
	void processNodes(fastgltf::Asset& asset) {
		const size_t sceneIndex = asset.defaultScene.value_or(0);
		fastgltf::iterateSceneNodes(asset, sceneIndex, fastgltf::math::fmat4x4(),
			[&](fastgltf::Node& node, fastgltf::math::fmat4x4 m) {
				constexpr glm::f32mat4x4 zMirror = {
					1.0f, 0.0f, 0.0f, 0.0f,
					0.0f, 1.0f, 0.0f, 0.0f,
					0.0f, 0.0f, -1.0f, 0.0f,
					0.0f, 0.0f, 0.0f, 1.0f,
				};
				glm::f32mat4x4 matrix = reinterpret_cast<glm::f32mat4x4&>(m);
				matrix *= zMirror;


				if (node.meshIndex.has_value()) {
					addMeshData(asset, matrix, static_cast<uint32_t>(node.meshIndex.value()));
					return;
				}
				if (node.lightIndex.has_value()) {
					addLightData(asset, matrix, static_cast<uint32_t>(node.lightIndex.value()));
					return;
				}
				if (node.cameraIndex.has_value()) {
					addCameraData(asset, matrix, static_cast<uint32_t>(node.cameraIndex.value()));
					return;
				}
				std::cout << "warning: unknown node type: " << node.name << std::endl;
			});
	};
}

namespace gltf {
	fastgltf::Asset* gltf::getAsset(std::string gltfFilePath) {
		fastgltf::Parser parser = fastgltf::Parser::Parser(fastgltf::Extensions::KHR_lights_punctual);

		auto gltfFile = fastgltf::GltfDataBuffer::FromPath(gltfFilePath);
		if (gltfFile.error() != fastgltf::Error::None) {// "cube databuffer fromPath");
			LOG(ERROR) << "can't load gltf file";
		}

		auto wholeGltf = parser.loadGltf(gltfFile.get(), "models/cornellBox", fastgltf::Options::LoadExternalBuffers);
		if (wholeGltf.error() != fastgltf::Error::None) {
			LOG(ERROR) << "can't load whole gltf";
		}
		
		return &wholeGltf.get();
	}

	host::Objects gltf::processAsset(fastgltf::Asset* asset) {
		host::Objects objects;

		return objects;
	}
}

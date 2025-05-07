#pragma once
#include "gltf.hpp"
#include "glm/glm.hpp"
#include "glm/ext.hpp"
#include "fastgltf/core.hpp"
#include "fastgltf/tools.hpp"
#include "fastgltf/types.hpp"
#include "absl/log/log.h"
#include <glm/gtx/matrix_decompose.hpp>

namespace {
	void addMeshData(host::Objects& objects, fastgltf::Asset& asset, glm::f32mat4x4& transform, uint32_t meshIndex) {
		//		if (_meshIndexToDrawInfoMap.count(meshIndex)) {
		//			++_meshIndexToDrawInfoMap[meshIndex]->instanceCount;
		//			return;
		//		};

		auto& mesh = asset.meshes[meshIndex];

		for (auto& primitive : mesh.primitives) {
			const size_t vbosOffset = objects.vbos.size();

			objects.transforms.push_back(transform);

			//vertice
			fastgltf::Attribute& positionAttribute = *primitive.findAttribute("POSITION");
			fastgltf::Accessor& positionAccessor = asset.accessors[positionAttribute.accessorIndex];
			objects.vbos.resize(objects.vbos.size() + positionAccessor.count);
			fastgltf::iterateAccessorWithIndex<fastgltf::math::f32vec3>(
				asset, positionAccessor, [&](fastgltf::math::f32vec3 vertex, size_t i) {
					memcpy(&objects.vbos[i + vbosOffset].vertex, &vertex, sizeof(glm::f32vec3));
				}
			);

			//normal
			fastgltf::Attribute& normalAttribute = *primitive.findAttribute("NORMAL");
			fastgltf::Accessor& normalAccessor = asset.accessors[normalAttribute.accessorIndex];
			fastgltf::iterateAccessorWithIndex<fastgltf::math::f32vec3>(
				asset, normalAccessor, [&](fastgltf::math::f32vec3 normal, size_t i) {
					memcpy(&objects.vbos[i + vbosOffset].normal, &normal, sizeof(glm::f32vec3));
				}
			);

			//texcoord_0
			fastgltf::Attribute& texcoordAttribute = *primitive.findAttribute("TEXCOORD_0");
			fastgltf::Accessor& texcoordAccessor = asset.accessors[texcoordAttribute.accessorIndex];
			fastgltf::iterateAccessorWithIndex<fastgltf::math::f32vec2>(
				asset, texcoordAccessor, [&](fastgltf::math::f32vec2 texcoord, size_t i) {
					memcpy(&objects.vbos[i + vbosOffset].texcoord, &texcoord, sizeof(glm::f32vec2));
				}
			);

			//indice
			if (!primitive.indicesAccessor.has_value()) {
				LOG(FATAL) << "no indices accessor value";
			}
			auto& accessor = asset.accessors[primitive.indicesAccessor.value()];
			size_t indicesOffset = objects.indices.size();
			objects.indices.resize(objects.indices.size() + accessor.count);
			fastgltf::iterateAccessorWithIndex<uint16_t>(
				asset, accessor, [&](uint16_t index, size_t i) {
					objects.indices[i + indicesOffset] = static_cast<uint16_t>(vbosOffset) + index;
				}
			);

			//instanceProperty
			const host::structs::InstanceProperty instanceProperty = {
				.materialIndex = static_cast<uint32_t>(primitive.materialIndex.value_or(asset.materials.size())),
			};
			objects.instanceProperties.push_back(instanceProperty);

			//drawCall
			const host::structs::DrawCall drawCall = {
				.indexCount = static_cast<uint32_t>(accessor.count),
				.instanceCount = 1, //TODO handle multiple instances
				.firstIndex = static_cast<uint32_t>(indicesOffset),
				.firstInstance = static_cast<uint32_t>(objects.drawCalls.size()),
			};
			objects.drawCalls.emplace_back(drawCall);
			//_meshIndexToDrawInfoMap.insert(std::make_pair(meshIndex, &objects.drawCalls.emplace_back(drawCall)));
		}
	}

	void addLightData(host::Objects& objects, fastgltf::Asset& asset, glm::f32mat4x4& transform, uint32_t lightIndex) {
		host::structs::Light l = {};
		glm::f32quat quaterion;
		glm::f32vec3 scale, skew;
		glm::f32vec4 perspective;
		bool success = glm::decompose(transform, scale, quaterion, l.position, skew, perspective);
		if (!success) {
			throw std::runtime_error("could not decompose matrix");
		}

		quaterion = glm::normalize(quaterion);
		l.rotation = glm::eulerAngles(quaterion);

		memcpy(&l.color, &asset.lights[lightIndex].color, sizeof(glm::f32vec3));
		l.type = static_cast<uint32_t>(asset.lights[lightIndex].type);
		memcpy(&l.intensity, &asset.lights[lightIndex].intensity, sizeof(glm::f32) * 4);

		const glm::mat4x4 lightView = glm::inverse(transform);
		const glm::mat4x4 lightProjection = glm::perspectiveRH_ZO(l.outerConeAngle, 1.0f, 0.1f, l.range);
		l.lightSpaceMatrix = lightProjection * lightView;

		objects.lights.push_back(l);
	}



	void processNodes(host::Objects& object, fastgltf::Asset& asset) {
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
					addMeshData(object, asset, matrix, static_cast<uint32_t>(node.meshIndex.value()));
					return;
				}
				else if (node.lightIndex.has_value()) {
					addLightData(object, asset, matrix, static_cast<uint32_t>(node.lightIndex.value()));
					return;
				}
				else if (node.cameraIndex.has_value()) {
					addCameraData(object, asset, matrix, static_cast<uint32_t>(node.cameraIndex.value()));
					return;
				}
				LOG(WARNING) << "unknown node type: " << node.name << std::endl;
			});
	};
}

namespace gltf {
	fastgltf::Asset* gltf::getAsset(std::string& gltfFilePath) {
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

	host::Objects gltf::processAsset(fastgltf::Asset& asset) {
		host::Objects hostObjects;
		processNodes(hostObjects, asset);

		return hostObjects;
	}
}

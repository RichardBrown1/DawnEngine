#pragma once
#include "gltf.hpp"
#include "glm/glm.hpp"
#include "glm/ext.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>

#include "fastgltf/core.hpp"
#include "fastgltf/tools.hpp"
#include "fastgltf/types.hpp"
#include "absl/log/log.h"
#include "../host/structs.hpp"
#include "../host/host.hpp"
#include "convert.hpp"

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

	void addCameraData(
		host::Objects& objects,
		fastgltf::Asset& asset,
		glm::f32mat4x4& transform,
		uint32_t cameraIndex,
		const std::array<uint32_t, 2> screenDimensions) {
		//	if (_buffers.camera.GetSize() > 0) {
		//TODO what if there is 0 cameras or more than 1 cameras
//		return;
//	}
		const glm::f32mat4x4 view = glm::inverse(transform);
		fastgltf::Camera::Perspective* perspectiveCamera = std::get_if<fastgltf::Camera::Perspective>(&asset.cameras[cameraIndex].camera);
		fastgltf::Camera::Orthographic* orthographicCamera = std::get_if<fastgltf::Camera::Orthographic>(&asset.cameras[cameraIndex].camera);
		if (orthographicCamera != nullptr) {
			throw std::runtime_error("orthographic camera not supported");
		}

		host::structs::H_Camera h_camera = {
			.projection = glm::perspectiveRH_ZO(perspectiveCamera->yfov, screenDimensions[0] / (float)screenDimensions[1], perspectiveCamera->znear, perspectiveCamera->zfar.value_or(1024.0f)),
			.position = glm::f32vec3(transform[3]),
			.forward = -glm::normalize(glm::f32vec3(view[2])),
		};

		objects.cameras.push_back(h_camera);
	}

	void processNodes(host::Objects& object, fastgltf::Asset& asset, const std::array<uint32_t, 2> screenDimensions) {
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
					addCameraData(object, asset, matrix, static_cast<uint32_t>(node.cameraIndex.value()), screenDimensions);
					return;
				}
				LOG(WARNING) << "unknown node type: " << node.name << std::endl;
			});
	};

	void addMaterial(const fastgltf::Material& inputMaterial, host::structs::Material& outputMaterial) {
		memcpy(&outputMaterial.pbrMetallicRoughness, &inputMaterial.pbrData, sizeof(glm::f32vec4) + sizeof(float) * 2);
		outputMaterial.pbrMetallicRoughness.baseColorTextureInfo = gltf::convert::textureInfo(inputMaterial.pbrData.baseColorTexture.value());

		//TODO _stpMaterialIndex[outputMaterial.pbrMetallicRoughness.baseColorTextureInfo.index];

	}

	//texture is gltf name - stp will be DawnEngine name.
	void addSamplerTexturePair(const fastgltf::Texture& inputTexture, host::structs::SamplerTexturePair& outputStp) {
		if (!inputTexture.basisuImageIndex.has_value()) {
			throw std::runtime_error("only KTX files are able to be used as textures");
		}
		const host::structs::SamplerTexturePair samplerTexturePair = {
			.samplerIndex = static_cast<uint32_t>(inputTexture.samplerIndex.has_value()),
			.textureIndex = static_cast<uint32_t>(inputTexture.basisuImageIndex.has_value()),
		};
		outputStp = samplerTexturePair;
	}

	void addTextureUri(fastgltf::DataSource& dataSource, const std::string& gltfDirectory, std::string& outputFilePath)
	{
		if (!std::holds_alternative<fastgltf::sources::URI>(dataSource)) {
			LOG(ERROR) << "Cannot get fastgltf::DataSource Texture, unsupported type";
		}
		fastgltf::sources::URI* p_uri = std::get_if<fastgltf::sources::URI>(&dataSource);
		if (p_uri->mimeType != fastgltf::MimeType::KTX2) {
			LOG(ERROR) << "Only KTX2 Textures are supported";
		}
		outputFilePath = gltfDirectory + outputFilePath;
	}

	void addSampler(const fastgltf::Sampler& inputSampler, host::structs::Sampler& outputSampler) {
		const host::structs::Sampler samplerDescriptor = {
		 .addressModeU = gltf::convert::convertType(inputSampler.wrapS),
		 .addressModeV = gltf::convert::convertType(inputSampler.wrapT),
		 .magFilter = gltf::convert::convertFilter(inputSampler.magFilter.value_or(fastgltf::Filter::Linear)),
		 .minFilter = gltf::convert::convertFilter(inputSampler.minFilter.value_or(fastgltf::Filter::Linear)),
		 .mipmapFilter = gltf::convert::convertMipMapFilter(inputSampler.minFilter.value_or(fastgltf::Filter::Linear)),
		};
		outputSampler = samplerDescriptor;
	}
}

namespace gltf {
	fastgltf::Asset* getAsset(const std::string& gltfDirectory, const std::string& gltfFileName) {
		fastgltf::Parser parser = fastgltf::Parser::Parser(fastgltf::Extensions::KHR_lights_punctual);

		std::string gltfFilePath = gltfDirectory + gltfFileName;
		auto gltfFile = fastgltf::GltfDataBuffer::FromPath(gltfFilePath);
		if (gltfFile.error() != fastgltf::Error::None) {// "cube databuffer fromPath");
			LOG(ERROR) << "can't load gltf file";
		}

		auto wholeGltf = parser.loadGltf(gltfFile.get(), gltfDirectory, fastgltf::Options::LoadExternalBuffers);
		if (wholeGltf.error() != fastgltf::Error::None) {
			LOG(ERROR) << "can't load whole gltf";
		}
		
		return &wholeGltf.get();
	}

	host::Objects processAsset(fastgltf::Asset& asset, std::array<uint32_t, 2> screenDimensions, const std::string gltfDirectory) {
		host::Objects hostObjects;

		processNodes(hostObjects, asset, screenDimensions);

		hostObjects.materials.resize(asset.materials.size());
		for (uint32_t i = 0; i < hostObjects.materials.size(); ++i) {
			addMaterial(asset.materials[i], hostObjects.materials[i]);
		}
		hostObjects.samplerTexturePairs.resize(asset.textures.size());
		for (uint32_t i = 0; i < hostObjects.samplerTexturePairs.size(); ++i) {
			addSamplerTexturePair(asset.textures[i], hostObjects.samplerTexturePairs[i]);
		}
		hostObjects.textureUris.resize(asset.images.size());
		for (uint32_t i = 0; i < hostObjects.textureUris.size(); ++i) {
			addTextureUri(asset.images[i].data, gltfDirectory, hostObjects.textureUris[i]);
		}
		hostObjects.samplers.resize(asset.samplers.size());
		for (uint32_t i = 0; i < hostObjects.samplers.size(); ++i) {
			addSampler(asset.samplers[i], hostObjects.samplers[i]);
		}
		return hostObjects;
	}
}

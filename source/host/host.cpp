#pragma once
#include "host.hpp"
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include "../device/device.hpp"
#include "../texture/texture.hpp"
#include "../constants.hpp"
#include "../gltf/gltf.hpp"
#include <glm/ext/matrix_clip_space.hpp>
#include <unordered_map>
#include <dawn/webgpu_cpp.h>

namespace host {
	SceneResources::SceneResources(
	const std::string& gltfDirectory,
	const std::string& gltfFileName,
  const std::array<uint32_t, 2> screenDimensions) {
		fastgltf::Asset asset = gltf::getAsset(gltfDirectory, gltfFileName);
		gltf::processAsset(*this, asset, screenDimensions, gltfDirectory);
		addDefaults(screenDimensions);
		postProcessData();
	};
	
	//defaults if none found
	void SceneResources::addDefaults(const std::array<uint32_t, 2> screenDimensions) {
		if (cameras.size() == 0) {
			cameras.push_back(structs::host::H_Camera{
				.projection = glm::perspectiveRH_ZO(45.0f, screenDimensions[0] / (float)screenDimensions[1], 0.00001f, 1024.0f),
				.position = { 0.0f, 0.0f, -0.1f },
				.forward = { 0.0f, 0.0f, 1.0f },
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

	void SceneResources::postProcessData() {
		for (auto& m : materials) {
			baseColorStpIds.emplace_back(m.pbrMetallicRoughness.baseColorTextureInfo.index);
			metallicRoughnessStpIds.emplace_back(m.pbrMetallicRoughness.metallicRoughnessTextureInfo.index);
			normalStpIds.emplace_back(m.normalTextureInfo.index);
		}
	}

	device::SceneResources SceneResources::ToDevice(
		WGPUContext& wgpuContext
	) {
		device::SceneResources d_objects;
		d_objects.vbo = device::createBuffer<structs::VBO>(
			wgpuContext,
			this->vbo,
			"vbo",
			wgpu::BufferUsage::Vertex
		);
		d_objects.indices = device::createBuffer<uint16_t>(
			wgpuContext,
			this->indices,
			"indices",
			wgpu::BufferUsage::Index
		);
		d_objects.transforms = device::createBuffer<glm::f32mat4x4>(
			wgpuContext,
			this->transforms,
			"transforms",
			wgpu::BufferUsage::Storage
		);
		d_objects.instanceProperties = device::createBuffer<structs::InstanceProperty>(
			wgpuContext,
			this->instanceProperties,
			"instance properties",
			wgpu::BufferUsage::Storage
		);
		d_objects.lights = device::createBuffer<structs::Light>(
			wgpuContext,
			this->lights,
			"lights",
			wgpu::BufferUsage::Storage
		);
		d_objects.materials = device::createBuffer<structs::Material>(
			wgpuContext,
			this->materials,
			"materials",
			wgpu::BufferUsage::Storage
		);
		d_objects.samplerTexturePairs = device::createBuffer<structs::SamplerTexturePair>(
			wgpuContext,
			this->samplerTexturePairs,
			"sampler texture pairs",
			wgpu::BufferUsage::Storage
		);


		std::vector<glm::f32mat4x4> projectionViews;
		for (uint32_t i = 0; i < this->cameras.size(); i++) {
			const glm::f32mat4x4 view = glm::lookAt(
				this->cameras[i].position,
				this->cameras[i].position + this->cameras[i].forward,
				constants::UP
			);
			projectionViews.push_back(this->cameras[0].projection * view);
		}
		d_objects.cameras = device::createBuffer<glm::f32mat4x4>(
			wgpuContext,
			projectionViews,
			"cameras",
			wgpu::BufferUsage::Uniform
		);

		for (uint32_t i = 0; i < d_objects.samplers.size(); ++i) {
			d_objects.samplers[i] = wgpuContext.device.CreateSampler(&this->samplers[i]);
		}
		const wgpu::SamplerDescriptor defaultSamplerDescriptor = {
			.label = "default label",
			.addressModeU = wgpu::AddressMode::Repeat,
			.addressModeV = wgpu::AddressMode::Repeat,
			.magFilter = wgpu::FilterMode::Linear,
			.minFilter = wgpu::FilterMode::Linear,
			.mipmapFilter = wgpu::MipmapFilterMode::Nearest,
		};
		d_objects.samplers[UINT32_MAX] = wgpuContext.device.CreateSampler(&defaultSamplerDescriptor);

		d_objects.textures.resize(this->textureUris.size());
		d_objects.textureViews.resize(this->textureUris.size());
		for (uint32_t i = 0; i < this->textureUris.size(); ++i) {
			texture::getTexture(wgpuContext, this->textureUris[i], d_objects.textures[i], d_objects.textureViews[i]);
		}

		return d_objects;
	}

}

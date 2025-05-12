#pragma once
#include "host.hpp"
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include "../device/device.hpp"
#include "../texture/texture.hpp"
#include "../constants.hpp"

structs::device::Objects host::SceneResources::ToDeviceObjects(
	Engine& engine
) {
	structs::device::Objects d_objects;
	d_objects.vbo = device::createBuffer<structs::VBO>(
		engine,
		this->vbo,
		"vbo",
		wgpu::BufferUsage::Vertex
	);
	d_objects.indices = device::createBuffer<uint16_t>(
		engine,
		this->indices,
		"indices",
		wgpu::BufferUsage::Index
	);
	d_objects.transforms = device::createBuffer<glm::f32mat4x4>(
		engine,
		this->transforms,
		"transforms",
		wgpu::BufferUsage::Storage
	);
	d_objects.instanceProperties = device::createBuffer<structs::InstanceProperty>(
		engine,
		this->instanceProperties,
		"instance properties",
		wgpu::BufferUsage::Storage
	);
	d_objects.lights = device::createBuffer<structs::Light>(
		engine,
		this->lights,
		"lights",
		wgpu::BufferUsage::Storage
	);
	d_objects.materials = device::createBuffer<structs::Material>(
		engine,
		this->materials,
		"materials",
		wgpu::BufferUsage::Storage
	);
	d_objects.samplerTexturePairs = device::createBuffer<structs::SamplerTexturePair>(
		engine,
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
		engine,
		projectionViews,
		"cameras",
		wgpu::BufferUsage::Storage
	);

	d_objects.samplers.resize(this->samplers.size());
	for (uint32_t i = 0; i < d_objects.samplers.size(); ++i) {
		d_objects.samplers[i] = engine.device.CreateSampler(&this->samplers[i]);
	}

	d_objects.textureViews.resize(this->textureUris.size());
	for (uint32_t i = 0; i < d_objects.samplers.size(); ++i) {
		texture::getTexture(engine, this->textureUris[i], d_objects.textures[i], d_objects.textureViews[i]);
	}

	return d_objects;
}

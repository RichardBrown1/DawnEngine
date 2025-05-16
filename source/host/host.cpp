#pragma once
#include "host.hpp"
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include "../device/device.hpp"
#include "../texture/texture.hpp"
#include "../constants.hpp"

device::SceneResources host::SceneResources::ToDevice(
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

	d_objects.samplers.resize(this->samplers.size());
	for (uint32_t i = 0; i < d_objects.samplers.size(); ++i) {
		d_objects.samplers[i] = wgpuContext.device.CreateSampler(&this->samplers[i]);
	}

	d_objects.textures.resize(this->textureUris.size());
	d_objects.textureViews.resize(d_objects.textures.size());
	for (uint32_t i = 0; i < d_objects.samplers.size(); ++i) {
		texture::getTexture(wgpuContext, this->textureUris[i], d_objects.textures[i], d_objects.textureViews[i]);
	}

	return d_objects;
}

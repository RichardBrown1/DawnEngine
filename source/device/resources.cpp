#pragma once
#include "../texture/texture.hpp"
#include "device.hpp"
#include "resources.hpp"
#include <glm/ext/matrix_transform.hpp>
#include <cstdint>
#include <format>
#include <string>
#include <vector>
#include <glm/fwd.hpp>
#include "../constants.hpp"
#include "../host/host.hpp"
#include "../structs/structs.hpp"
#include "../wgpuContext/wgpuContext.hpp"
#include <dawn/webgpu_cpp.h>

const std::string worldPositionLabel = "world position info";
const std::string baseColorLabel = "base color";
const std::string normalLabel = "normals";
const std::string texCoordLabel = "texcoord";
const std::string depthTextureLabel = "depth texture";
const std::string baseColorIdLabel = "base color id";
const std::string normalIdLabel = "normal id";
const std::string lightingLabel = "lighting";
const std::string shadowLabel = "shadow";
const std::string ultimateLabel = "ultimate";

const wgpu::TextureUsage worldPositionTextureUsage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::StorageBinding;
const wgpu::TextureUsage baseColorTextureUsage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::StorageBinding;
const wgpu::TextureUsage normalTextureUsage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::StorageBinding;
const wgpu::TextureUsage texCoordTextureUsage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::StorageBinding;
const wgpu::TextureUsage baseColorIdTextureUsage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::StorageBinding;
const wgpu::TextureUsage normalIdTextureUsage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::StorageBinding;
const wgpu::TextureUsage depthTextureUsage = wgpu::TextureUsage::RenderAttachment;
const wgpu::TextureUsage lightingTextureUsage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::StorageBinding;
const wgpu::TextureUsage shadowTextureUsage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::StorageBinding;
const wgpu::TextureUsage ultimateTextureUsage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::StorageBinding;

wgpu::Extent2D shadowDimensions = wgpu::Extent2D{ 2048, 2048 };

RenderResources::RenderResources(WGPUContext* wgpuContext) {
	const texture::descriptor::CreateTextureView worldPositionTextureViewDescriptor = {
		.label = worldPositionLabel,
		.device = &wgpuContext->device,
		.textureUsage = worldPositionTextureUsage,
		.textureDimensions = wgpuContext->getScreenDimensions(),
		.textureFormat = worldPositionTextureFormat,
		.outputTextureView = worldPositionTextureView,
	};
	texture::createTextureView(&worldPositionTextureViewDescriptor);

	const texture::descriptor::CreateTextureView baseColorTextureViewDescriptor = {
		.label = baseColorLabel,
		.device = &wgpuContext->device,
		.textureUsage = baseColorTextureUsage,
		.textureDimensions = wgpuContext->getScreenDimensions(),
		.textureFormat = baseColorTextureFormat,
		.outputTextureView = baseColorTextureView,
	};
	texture::createTextureView(&baseColorTextureViewDescriptor);

	const texture::descriptor::CreateTextureView normalTextureViewDescriptor = {
		.label = normalLabel,
		.device = &wgpuContext->device,
		.textureUsage = normalTextureUsage,
		.textureDimensions = wgpuContext->getScreenDimensions(),
		.textureFormat = normalTextureFormat,
		.outputTextureView = normalTextureView,
	};
	texture::createTextureView(&normalTextureViewDescriptor);

	const texture::descriptor::CreateTextureView texCoordTextureViewDescriptor = {
		.label = texCoordLabel,
		.device = &wgpuContext->device,
		.textureUsage = texCoordTextureUsage,
		.textureDimensions = wgpuContext->getScreenDimensions(),
		.textureFormat = texCoordTextureFormat,
		.outputTextureView = texCoordTextureView,
	};
	texture::createTextureView(&texCoordTextureViewDescriptor);

	const texture::descriptor::CreateTextureView baseColorIdTextureViewDescriptor = {
		.label = baseColorIdLabel,
		.device = &wgpuContext->device,
		.textureUsage = baseColorIdTextureUsage,
		.textureDimensions = wgpuContext->getScreenDimensions(),
		.textureFormat = baseColorIdTextureFormat,
		.outputTextureView = baseColorIdTextureView,
	};
	texture::createTextureView(&baseColorIdTextureViewDescriptor);

	const texture::descriptor::CreateTextureView normalIdTextureViewDescriptor = {
		.label = normalIdLabel,
		.device = &wgpuContext->device,
		.textureUsage = normalIdTextureUsage,
		.textureDimensions = wgpuContext->getScreenDimensions(),
		.textureFormat = normalIdTextureFormat,
		.outputTextureView = normalIdTextureView,
	};
	texture::createTextureView(&normalIdTextureViewDescriptor);

	const texture::descriptor::CreateTextureView depthTextureViewDescriptor = {
		.label = depthTextureLabel,
		.device = &wgpuContext->device,
		.textureUsage = depthTextureUsage,
		.textureDimensions = wgpuContext->getScreenDimensions(),
		.textureFormat = depthTextureFormat,
		.outputTextureView = depthTextureView,
	};
	texture::createTextureView(&depthTextureViewDescriptor);

	const texture::descriptor::CreateTextureView lightingTextureViewDescriptor = {
		.label = lightingLabel,
		.device = &wgpuContext->device,
		.textureUsage = lightingTextureUsage,
		.textureDimensions = wgpuContext->getScreenDimensions(),
		.textureFormat = lightingTextureFormat,
		.outputTextureView = lightingTextureView,
	};
	texture::createTextureView(&lightingTextureViewDescriptor);

	const texture::descriptor::CreateTextureView shadowTextureViewDescriptor = {
		.label = shadowLabel,
		.device = &wgpuContext->device,
		.textureUsage = shadowTextureUsage,
		.textureDimensions = shadowDimensions,
		.textureFormat = constants::DEPTH_FORMAT,
		.outputTextureView = shadowTextureView,
	};
	texture::createTextureView(&shadowTextureViewDescriptor);

	const texture::descriptor::CreateTextureView ultimateTextureViewDescriptor = {
		.label = ultimateLabel,
		.device = &wgpuContext->device,
		.textureUsage = ultimateTextureUsage,
		.textureDimensions = wgpuContext->getScreenDimensions(),
		.textureFormat = ultimateTextureFormat,
		.outputTextureView = ultimateTextureView,
	};
	texture::createTextureView(&ultimateTextureViewDescriptor);

}

SceneResources::SceneResources(WGPUContext* wgpuContext, HostSceneResources& host) {
	this->vbo = device::createBuffer<structs::VBO>(
		*wgpuContext,
		host.vbo,
		"vbo",
		wgpu::BufferUsage::Vertex
	);
	this->indices = device::createBuffer<uint16_t>(
		*wgpuContext,
		host.indices,
		"indices",
		wgpu::BufferUsage::Index
	);
	this->transforms = device::createBuffer<glm::f32mat4x4>(
		*wgpuContext,
		host.transforms,
		"transforms",
		wgpu::BufferUsage::Storage
	);
	this->materialIndices = device::createBuffer<uint32_t>(
		*wgpuContext,
		host.materialIndices,
		"materialIndices",
		wgpu::BufferUsage::Storage
	);
	for (uint32_t i = 0; auto & light : host.lights) {
		const std::string lightLabel = std::format("light {0}", i);
		this->lights.emplace_back(
			device::createBuffer(
				*wgpuContext,
				light,
				lightLabel,
				wgpu::BufferUsage::Uniform)
		);
		++i;
	}
	this->materials = device::createBuffer<structs::Material>(
		*wgpuContext,
		host.materials,
		"materials",
		wgpu::BufferUsage::Storage
	);
	this->samplerTexturePairs = device::createBuffer<structs::SamplerTexturePair>(
		*wgpuContext,
		host.samplerTexturePairs,
		"sampler texture pairs",
		wgpu::BufferUsage::Storage
	);

	std::vector<glm::f32mat4x4> projectionViews;
	for (uint32_t i = 0; i < host.cameras.size(); i++) {
		const glm::f32mat4x4 view = glm::lookAt(
			host.cameras[i].position,
			host.cameras[i].position + host.cameras[i].forward,
			constants::UP
		);
		projectionViews.push_back(host.cameras[i].projection * view);
	}
	this->cameras = device::createBuffer<glm::f32mat4x4>(
		*wgpuContext,
		projectionViews,
		"cameras",
		wgpu::BufferUsage::Uniform
	);

	for (uint32_t i = 0; i < this->samplers.size(); ++i) {
		this->samplers[i] = wgpuContext->device.CreateSampler(&host.samplers[i]);
	}
	const wgpu::SamplerDescriptor defaultSamplerDescriptor = {
		.label = "default label",
		.addressModeU = wgpu::AddressMode::Repeat,
		.addressModeV = wgpu::AddressMode::Repeat,
		.magFilter = wgpu::FilterMode::Linear,
		.minFilter = wgpu::FilterMode::Linear,
		.mipmapFilter = wgpu::MipmapFilterMode::Nearest,
	};
	this->samplers[UINT32_MAX] = wgpuContext->device.CreateSampler(&defaultSamplerDescriptor);

	this->textures.resize(host.textureUris.size());
	this->textureViews.resize(host.textureUris.size());
	for (uint32_t i = 0; i < host.textureUris.size(); ++i) {
		texture::getTexture(*wgpuContext, host.textureUris[i], this->textures[i], this->textureViews[i]);
	}
}
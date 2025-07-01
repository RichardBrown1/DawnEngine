#pragma once
#include "../texture/texture.hpp"
#include "resources.hpp"

const std::string worldPositionLabel = "world position info";
const std::string baseColorLabel = "base color";
const std::string normalLabel = "normals";
const std::string texCoordLabel = "texcoord";
const std::string depthTextureLabel = "depth texture";
const std::string baseColorIdLabel = "base color id";
const std::string normalIdLabel = "normal id";

const wgpu::TextureUsage worldPositionTextureUsage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::StorageBinding;
const wgpu::TextureUsage baseColorTextureUsage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::StorageBinding;
const wgpu::TextureUsage normalTextureUsage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::StorageBinding;
const wgpu::TextureUsage texCoordTextureUsage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::StorageBinding;
const wgpu::TextureUsage baseColorIdTextureUsage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::StorageBinding;
const wgpu::TextureUsage normalIdTextureUsage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::StorageBinding;
const wgpu::TextureUsage depthTextureUsage = wgpu::TextureUsage::RenderAttachment;

RenderResources::RenderResources(WGPUContext* wgpuContext) {
	texture::descriptor::CreateTextureView worldPositionTextureViewDescriptor = {
		.label = worldPositionLabel,
		.device = &wgpuContext->device,
		.textureUsage = worldPositionTextureUsage,
		.textureDimensions = wgpuContext->getScreenDimensions(),
		.textureFormat = worldPositionTextureFormat,
		.outputTextureView = worldPositionTextureView,
	};
	texture::createTextureView(&worldPositionTextureViewDescriptor);

	texture::descriptor::CreateTextureView baseColorTextureViewDescriptor = {
		.label = baseColorLabel,
		.device = &wgpuContext->device,
		.textureUsage = baseColorTextureUsage,
		.textureDimensions = wgpuContext->getScreenDimensions(),
		.textureFormat = baseColorTextureFormat,
		.outputTextureView = baseColorTextureView,
	};
	texture::createTextureView(&baseColorTextureViewDescriptor);

	texture::descriptor::CreateTextureView normalTextureViewDescriptor = {
		.label = normalLabel,
		.device = &wgpuContext->device,
		.textureUsage = normalTextureUsage,
		.textureDimensions = wgpuContext->getScreenDimensions(),
		.textureFormat = normalTextureFormat,
		.outputTextureView = normalTextureView,
	};
	texture::createTextureView(&normalTextureViewDescriptor);

	texture::descriptor::CreateTextureView texCoordTextureViewDescriptor = {
		.label = texCoordLabel,
		.device = &wgpuContext->device,
		.textureUsage = texCoordTextureUsage,
		.textureDimensions = wgpuContext->getScreenDimensions(),
		.textureFormat = texCoordTextureFormat,
		.outputTextureView = texCoordTextureView,
	};
	texture::createTextureView(&texCoordTextureViewDescriptor);

	texture::descriptor::CreateTextureView baseColorIdTextureViewDescriptor = {
		.label = baseColorIdLabel,
		.device = &wgpuContext->device,
		.textureUsage = baseColorIdTextureUsage,
		.textureDimensions = wgpuContext->getScreenDimensions(),
		.textureFormat = baseColorIdTextureFormat,
		.outputTextureView = baseColorIdTextureView,
	};
	texture::createTextureView(&baseColorIdTextureViewDescriptor);

	texture::descriptor::CreateTextureView normalIdTextureViewDescriptor = {
		.label = normalIdLabel,
		.device = &wgpuContext->device,
		.textureUsage = normalIdTextureUsage,
		.textureDimensions = wgpuContext->getScreenDimensions(),
		.textureFormat = normalIdTextureFormat,
		.outputTextureView = normalIdTextureView,
	};
	texture::createTextureView(&normalIdTextureViewDescriptor);

	texture::descriptor::CreateTextureView depthTextureViewDescriptor = {
		.label = depthTextureLabel,
		.device = &wgpuContext->device,
		.textureUsage = depthTextureUsage,
		.textureDimensions = wgpuContext->getScreenDimensions(),
		.textureFormat = depthTextureFormat,
		.outputTextureView = depthTextureView,
	};
	texture::createTextureView(&depthTextureViewDescriptor);
}
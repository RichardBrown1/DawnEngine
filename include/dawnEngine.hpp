#pragma once
#include <iostream>
#include <glm/glm.hpp>
#include <webgpu/webgpu_cpp.h>
#include <fastgltf/core.hpp>

struct UBO {
	alignas(sizeof(glm::mat4x4)) glm::mat4x4 projection;
	alignas(sizeof(glm::mat4x4)) glm::mat4x4 model;
	alignas(sizeof(glm::mat4x4)) glm::mat4x4 view;
};

struct DrawInfo {
	uint32_t indexCount;
	uint32_t instanceCount;
	uint32_t firstIndex;
	uint32_t baseVertex;
	uint32_t firstInstance; //requires indirect-first-instance feature
};


class DawnEngine {

public:
	DawnEngine();
	void run();
	void destroy();

private:
	fastgltf::Parser _gltfParser;

	wgpu::Instance _instance;
	wgpu::Device _device;
	wgpu::Surface _surface;
	wgpu::SurfaceConfiguration _surfaceConfiguration;
	wgpu::Queue _queue;
	wgpu::RenderPipeline _renderPipeline;
	wgpu::Buffer _vertexBuffer;
	wgpu::Buffer _uniformBuffer;
	wgpu::Buffer _indexBuffer;
	wgpu::Buffer _materialBuffer; //todo storage buffer
	std::vector<wgpu::BindGroup> _bindGroups;
	wgpu::TextureView _depthTextureView;
	wgpu::Sampler _depthSampler;
	std::vector<DrawInfo> _drawCalls;

	void initGltf();
	void initMeshBuffers(fastgltf::Asset& asset);
	void initMaterialBuffer(fastgltf::Asset& asset);
	void initDepthTexture();
	void initRenderPipeline();
	wgpu::PipelineLayout initPipelineLayout();
	wgpu::BindGroupLayout initUniformBindGroupLayout();
	wgpu::BindGroupLayout initMaterialBindGroupLayout();

	void draw();
	void updateUniformBuffers();
	wgpu::TextureView getNextSurfaceTextureView();
};
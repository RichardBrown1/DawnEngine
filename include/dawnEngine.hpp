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

struct InstanceProperty {
		uint32_t materialIndex;
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
	std::vector<fastgltf::math::f32vec3> _vertices;
	std::vector<uint16_t> _indices;
	std::vector<DrawInfo> _drawCalls;
	std::vector<glm::f32mat4x4> _transforms;
	std::vector<InstanceProperty> _instanceProperties;
	std::unordered_map<uint32_t, DrawInfo*> _meshIndexToDrawInfoMap;
	wgpu::Buffer _uniformBuffer;
	wgpu::Buffer _vertexBuffer;
	wgpu::Buffer _indexBuffer;
	wgpu::Buffer _instancePropertiesBuffer;
	wgpu::Buffer _transformBuffer;
	wgpu::Buffer _materialBuffer;
	std::vector<wgpu::BindGroup> _bindGroups;
	wgpu::TextureView _depthTextureView;
	wgpu::Sampler _depthSampler;

	void initGltf();
	void initNodes(fastgltf::Asset& asset);
	void addMeshData(fastgltf::Asset& asset, glm::f32mat4x4 transform, uint32_t meshIndex);
	void initMeshBuffers();
	void initMaterialBuffer(fastgltf::Asset& asset);
	void initDepthTexture();
	void initRenderPipeline();
	wgpu::PipelineLayout initPipelineLayout();
	wgpu::BindGroupLayout initStaticBindGroupLayout();
	wgpu::BindGroupLayout initInfrequentBindGroupLayout();
	//wgpu::BindGroupLayout initFrequentBindGroupLayout();
	//wgpu::BindGroupLayout initPerFrameBindGroupLayout():

	void draw();
	void updateUniformBuffers();
	wgpu::TextureView getNextSurfaceTextureView();
};
#pragma once
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <webgpu/webgpu_cpp.h>
#include <fastgltf/core.hpp>
#include "structs.hpp"

class Engine {

public:
	Engine();
	void run();
	void destroy();

private:
	fastgltf::Parser _gltfParser;

	wgpu::Instance _instance;
	wgpu::Device _device;
	wgpu::Surface _surface;
	wgpu::SurfaceConfiguration _surfaceConfiguration;
	wgpu::Queue _queue;
	RenderPipelines _renderPipelines;
	std::vector<VBO> _vbos;
	std::vector<uint16_t> _indices;
	std::vector<DrawInfo> _drawCalls;
	std::vector<glm::f32mat4x4> _transforms;
	std::vector<InstanceProperty> _instanceProperties;
	std::unordered_map<uint32_t, DrawInfo*> _meshIndexToDrawInfoMap;
	std::vector<Light> _lights;
	std::vector<Camera> _cameras;
	Buffers _buffers;
	BindGroups _bindGroups;
	wgpu::TextureView _depthTextureView;
	wgpu::Sampler _depthSampler;

	void initGltf();
	void initNodes(fastgltf::Asset& asset);
	void addMeshData(fastgltf::Asset& asset, glm::f32mat4x4& transform, uint32_t meshIndex);
	void addLightData(fastgltf::Asset& asset, glm::f32mat4x4& transform, uint32_t lightIndex);
	void addCameraData(fastgltf::Asset& asset, glm::f32mat4x4& transform, uint32_t cameraIndex);
	void initSceneBuffers();
	void initMaterialBuffer(fastgltf::Asset& asset);
	void initDepthTexture();
	void initRenderPipeline();

	void draw();
	wgpu::TextureView getNextSurfaceTextureView();
};
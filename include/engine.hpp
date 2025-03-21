#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_LEFT_HANDED
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
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
	DawnEngine::RenderPipelines _renderPipelines;
	std::vector<DawnEngine::VBO> _vbos;
	std::vector<uint16_t> _indices;
	std::vector<DawnEngine::DrawInfo> _drawCalls;
	std::vector<glm::f32mat4x4> _transforms;
	std::vector<DawnEngine::InstanceProperty> _instanceProperties;
	std::unordered_map<uint32_t, DawnEngine::DrawInfo*> _meshIndexToDrawInfoMap;
	std::vector<DawnEngine::Light> _lights;
	std::vector<DawnEngine::Camera> _cameras;
	DawnEngine::Buffers _buffers;
	DawnEngine::TextureViews _textureViews;
	DawnEngine::Samplers _samplers;
	DawnEngine::BindGroups _bindGroups;

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
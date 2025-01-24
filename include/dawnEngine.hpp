#pragma once
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <webgpu/webgpu_cpp.h>
#include <fastgltf/core.hpp>

struct Camera {
	alignas(sizeof(glm::mat4x4)) glm::mat4x4 projection;
	alignas(sizeof(glm::mat4x4)) glm::mat4x4 view;
};

struct VBO {
	glm::f32vec3 vertex;
	glm::f32vec3 normal;
};

struct DrawInfo {
	uint32_t indexCount;
	uint32_t instanceCount;
	uint32_t firstIndex;
	uint32_t baseVertex;
	uint32_t firstInstance; //requires indirect-first-instance feature
};

struct Light { //glm version of fastgltf::Light
  glm::f32vec3 position;
	uint32_t PAD0;
	glm::f32vec3 rotation;
	uint32_t PAD1;
	glm::f32vec3 color;
	glm::u32 type; //this is u32 instead of u8 for shader compatibility
	glm::f32 intensity;
	glm::f32 range;
	glm::f32 innerConeAngle;
	glm::f32 outerConeAngle;
};

struct InstanceProperty {
		uint32_t materialIndex;
		uint32_t PAD0;
		uint32_t PAD1;
		uint32_t PAD2;
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
	std::vector<VBO> _vbos;
	std::vector<uint16_t> _indices;
	std::vector<DrawInfo> _drawCalls;
	std::vector<glm::f32mat4x4> _transforms;
	std::vector<InstanceProperty> _instanceProperties;
	std::unordered_map<uint32_t, DrawInfo*> _meshIndexToDrawInfoMap;
	std::vector<Light> _lights;
	std::vector<Camera> _cameras;
	wgpu::Buffer _cameraBuffer;
	wgpu::Buffer _vboBuffer;
	wgpu::Buffer _indexBuffer;
	wgpu::Buffer _instancePropertiesBuffer;
	wgpu::Buffer _transformBuffer;
	wgpu::Buffer _materialBuffer;
	wgpu::Buffer _lightBuffer;
	std::vector<wgpu::BindGroup> _bindGroups;
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
	wgpu::PipelineLayout initPipelineLayout();
	wgpu::BindGroupLayout initStaticBindGroupLayout();
	wgpu::BindGroupLayout initInfrequentBindGroupLayout();
	//wgpu::BindGroupLayout initFrequentBindGroupLayout();
	//wgpu::BindGroupLayout initPerFrameBindGroupLayout():

	void draw();
	wgpu::TextureView getNextSurfaceTextureView();
};
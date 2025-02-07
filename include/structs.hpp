#pragma once
#include <glm/glm.hpp>

struct Buffers {
	wgpu::Buffer camera;
	wgpu::Buffer vbo;
	wgpu::Buffer index;
	wgpu::Buffer instanceProperties;
	wgpu::Buffer transform;
	wgpu::Buffer material;
	wgpu::Buffer light;
};

struct BindGroups {
	wgpu::BindGroup fixed;
	wgpu::BindGroup lights;
};

struct RenderPipelines {
	wgpu::RenderPipeline shadow;
	wgpu::RenderPipeline geometry;
};

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

struct Material {
	glm::vec4 baseColor;
};

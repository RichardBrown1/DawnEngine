#include "../include/gpuMemoryManager.hpp"

namespace DawnEngine {
	static GpuObjectManager* instance;

	GpuObjectManager::GpuObjectManager()
	{
		assert(instance == nullptr); //limit to one
		instance = this;
	}

	wgpu::BindGroupLayoutEntry GpuObjectManager::getBindGroupLayoutEntry(GPU_OBJECT_ID id) {
		wgpu::BindGroupLayoutEntry bindGroupLayoutEntry = gpuObjectsRegistry.at(id);
		bindGroupLayoutEntry.binding = (uint32_t)id;
		return bindGroupLayoutEntry;
	};

  std::map < GPU_OBJECT_ID, wgpu::BindGroupLayoutEntry > GpuObjectManager::gpuObjectsRegistry = {
		{
			GPU_OBJECT_ID::CAMERA,
			{
				.visibility = (wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment),
				.buffer = {
					.type = wgpu::BufferBindingType::Uniform,
					.minBindingSize = sizeof(Camera),
				}
			},
		},
		{
			GPU_OBJECT_ID::TRANSFORMS,
			{
				.visibility = wgpu::ShaderStage::Vertex,
				.buffer = {
					.type = wgpu::BufferBindingType::ReadOnlyStorage,
					.minBindingSize = sizeof(glm::f32mat4x4),
				}
			}
		},
		{
			GPU_OBJECT_ID::INSTANCE_PROPERTIES,
			{
				.visibility = wgpu::ShaderStage::Vertex,
				.buffer = {
					.type = wgpu::BufferBindingType::ReadOnlyStorage,
					.minBindingSize = sizeof(InstanceProperty),
				}
			}
		},
		{
			GPU_OBJECT_ID::MATERIALS,
			{
				.visibility = wgpu::ShaderStage::Vertex,
				.buffer = {
					.type = wgpu::BufferBindingType::ReadOnlyStorage,
					.minBindingSize = sizeof(Material),
				}
			}
		},
		{
			GPU_OBJECT_ID::LIGHTS,
			{
				.visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment,
				.buffer = {
					.type = wgpu::BufferBindingType::ReadOnlyStorage,
					.minBindingSize = sizeof(Light),
				}
			}
		},
		{
			GPU_OBJECT_ID::SHADOW_MAPS,
			{
				.visibility = wgpu::ShaderStage::Fragment,
				.texture = {
					.sampleType = wgpu::TextureSampleType::Depth,
					.viewDimension = wgpu::TextureViewDimension::e2D,
				}
			}
		},
		{
			GPU_OBJECT_ID::DEPTH_SAMPLERS,
			{
				.visibility = wgpu::ShaderStage::Fragment,
				.sampler = {
					.type = wgpu::SamplerBindingType::Comparison
				}
			}
		}
	};
};


#include "gpuObjectManager.hpp"

namespace DawnEngine {
	static GpuObjectManager* instance;

	wgpu::BindGroupLayoutEntry GpuObjectManager::getBindGroupLayoutEntry(GPU_OBJECT_ID id) {
		wgpu::BindGroupLayoutEntry bindGroupLayoutEntry = gpuObjectsRegistry.at(id);
		bindGroupLayoutEntry.binding = (uint32_t)id;
		return bindGroupLayoutEntry;
	};

	std::vector<wgpu::BindGroupLayoutEntry> GpuObjectManager::getBindGroupLayoutEntries(std::span<GPU_OBJECT_ID> span) {
		std::vector<wgpu::BindGroupLayoutEntry> bindGroupLayoutEntries;
		for (GPU_OBJECT_ID id : span) {
			bindGroupLayoutEntries.push_back(getBindGroupLayoutEntry(id));
		}
		return bindGroupLayoutEntries;
	};

	wgpu::BindGroupLayout GpuObjectManager::getBindGroupLayout(
		wgpu::Device &device, wgpu::StringView label, std::span<GPU_OBJECT_ID> gpuObjectIds) {
		auto bindGroupLayoutEntries = getBindGroupLayoutEntries(std::span{ gpuObjectIds });

		const wgpu::BindGroupLayoutDescriptor bindGroupLayoutDescriptor = {
			.label = label,
			.entryCount = bindGroupLayoutEntries.size(),
			.entries = bindGroupLayoutEntries.data(),
		};
		return device.CreateBindGroupLayout(&bindGroupLayoutDescriptor);
	}

	const std::unordered_map < GPU_OBJECT_ID, wgpu::BindGroupLayoutEntry > GpuObjectManager::gpuObjectsRegistry = {
		{
			GPU_OBJECT_ID::CAMERA,
			{
				.visibility = (wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment),
				.buffer = {
					.type = wgpu::BufferBindingType::Uniform,
					.minBindingSize = sizeof(glm::f32mat4x4),
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
				.visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment,
				.buffer = {
					.type = wgpu::BufferBindingType::ReadOnlyStorage,
					.minBindingSize = sizeof(InstanceProperty),
				}
			}
		},
		{
			GPU_OBJECT_ID::MATERIALS,
			{
				.visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment,
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
			GPU_OBJECT_ID::DEPTH_SAMPLER,
			{
				.visibility = wgpu::ShaderStage::Fragment,
				.sampler = {
					.type = wgpu::SamplerBindingType::Comparison
				}
			}
		},
		{
			GPU_OBJECT_ID::SAMPLER_TEXTURE_PAIR,
			{
				.visibility = wgpu::ShaderStage::Fragment,
				.buffer = {
					.type = wgpu::BufferBindingType::ReadOnlyStorage,
					.minBindingSize = sizeof(DawnEngine::SamplerTexturePair),
				}
			}
		},
		{
			GPU_OBJECT_ID::TEXTURES,
			{
				.visibility = wgpu::ShaderStage::Fragment,
				.texture = {
					.sampleType = wgpu::TextureSampleType::Float,
					.viewDimension = wgpu::TextureViewDimension::e2DArray,
				},
			},
		},
		{
			GPU_OBJECT_ID::SAMPLER,
			{
				.visibility = wgpu::ShaderStage::Fragment,
				.sampler = {
					.type = wgpu::SamplerBindingType::Filtering
				},
			},
		},
	};
};


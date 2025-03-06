#pragma once
#include <vector>
#include <map>
#include <span>
#include <variant>
#include <webgpu/webgpu_cpp.h>
#include "structs.hpp"

namespace DawnEngine {

	enum class GPU_OBJECT_ID : uint32_t {
		CAMERA,
		TRANSFORMS,
		INSTANCE_PROPERTIES,
		MATERIALS,
		LIGHTS,
		SHADOW_MAPS,
		DEPTH_SAMPLERS,
	};

	class GpuObjectManager {
	public:
		GpuObjectManager(GpuObjectManager const&) = delete;
		GpuObjectManager& operator=(GpuObjectManager const&) = delete;

		static std::shared_ptr<GpuObjectManager> instance() {
			static std::shared_ptr<GpuObjectManager> sp_GpuObjectManager{ new GpuObjectManager };
			return sp_GpuObjectManager;
		}

		wgpu::BindGroupLayoutEntry getBindGroupLayoutEntry(GPU_OBJECT_ID id);
		std::vector<wgpu::BindGroupLayoutEntry> getBindGroupLayoutEntries(std::span<GPU_OBJECT_ID> span);

	private:
		GpuObjectManager() {};
		static std::map <GPU_OBJECT_ID, wgpu::BindGroupLayoutEntry> gpuObjectsRegistry;
		//std::map <GPU_OBJECT_ID, wgpu::BindGroupLayoutEntry> gpuObjectsRegistry;

	};

};

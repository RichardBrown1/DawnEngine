#pragma once
#include <vector>
#include <map>
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
		GpuObjectManager();
		wgpu::BindGroupLayoutEntry getBindGroupLayoutEntry(GPU_OBJECT_ID id);
	private:
		static std::map <GPU_OBJECT_ID, wgpu::BindGroupLayoutEntry> gpuObjectsRegistry;
		//std::map <GPU_OBJECT_ID, wgpu::BindGroupLayoutEntry> gpuObjectsRegistry;

	};

};

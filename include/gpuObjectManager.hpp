#pragma once
#include <vector>
#include <map>
#include <span>
#include <variant>
#include <webgpu/webgpu_cpp.h>
#include "structs.hpp"

namespace DawnEngine {

	//https://stackoverflow.com/questions/8357240/how-to-automatically-convert-strongly-typed-enum-into-int
	enum class GPU_OBJECT_ID : uint32_t {
		CAMERA,
		TRANSFORMS,
		INSTANCE_PROPERTIES,
		MATERIALS,
		LIGHTS,
		SHADOW_MAPS,
		DEPTH_SAMPLERS,
	};

	//To be replaced with std::to_underlying in c++23
	template<typename T>
	inline constexpr auto operator+(T e) noexcept -> std::enable_if_t<std::is_enum<T>::value, std::underlying_type_t<T>> {
		return static_cast<std::underlying_type_t<T>>(e);
	};

	class GpuObjectManager {
	public:
		GpuObjectManager(GpuObjectManager const&) = delete;
		GpuObjectManager& operator=(GpuObjectManager const&) = delete;

		static std::shared_ptr<GpuObjectManager> instance() {
			static std::shared_ptr<GpuObjectManager> sp_GpuObjectManager{ new GpuObjectManager };
			return sp_GpuObjectManager;
		}

		wgpu::BindGroupLayout getBindGroupLayout(wgpu::Device& device, wgpu::StringView label, std::span<GPU_OBJECT_ID> gpuObjectIds);

	private:
		GpuObjectManager() {};
		static std::map <GPU_OBJECT_ID, wgpu::BindGroupLayoutEntry> gpuObjectsRegistry;
		wgpu::BindGroupLayoutEntry getBindGroupLayoutEntry(GPU_OBJECT_ID id);
		std::vector<wgpu::BindGroupLayoutEntry> getBindGroupLayoutEntries(std::span<GPU_OBJECT_ID> span);
		//std::map <GPU_OBJECT_ID, wgpu::BindGroupLayoutEntry> gpuObjectsRegistry;
	};
};

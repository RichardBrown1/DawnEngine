#pragma once
#include <vector>
#include <string>
#include <dawn/webgpu_cpp.h>
#include <glm/fwd.hpp>
#include "../structs/host.hpp"
#include "../device/device.hpp"

//Objects for the wgpu::Device but in RAM waiting to be processed
//This data should be in a format that can be consumed by the shader if its written into the device as is
namespace host {
	class SceneResources {
	public:
		//Mesh data
		std::vector<structs::VBO> vbo;
		std::vector<uint16_t> indices;
		std::vector<glm::f32mat4x4> transforms;
		std::vector<uint32_t> materialIndices;
		std::vector<structs::host::DrawCall> drawCalls;

		//Other data
		std::vector<structs::Light> lights;
		std::vector<structs::host::H_Camera> cameras;

		//Material related data
		std::vector<structs::Material> materials;
		std::vector<structs::SamplerTexturePair> samplerTexturePairs;
		std::vector<std::string> textureUris;
		std::vector<wgpu::SamplerDescriptor> samplers;

		//Post Process Data
		std::vector<uint32_t> baseColorStpIds;
		std::vector<uint32_t> normalStpIds;
		std::vector<uint32_t> metallicRoughnessStpIds;

		SceneResources() = delete;
		SceneResources(
			const std::string& gltfDirectory,
			const std::string& gltfFileName,
			const std::array<uint32_t, 2> screenDimensions
		);
		device::SceneResources ToDevice(WGPUContext& wgpuContext);

	private:
		void addDefaults(std::array<uint32_t, 2> screenDimensions);
		void postProcessData();
	};
}

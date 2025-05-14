#pragma once
#include <string>
#include <iostream>
#include "engine.hpp"
#include "gltf/gltf.hpp"
#include <fastgltf/types.hpp>
#include "host/host.hpp"
#include "render/initialRender.hpp"

namespace {
	std::string gltfDirectory = "models/cornellBox/"; //must end with "/"
	std::string gltfFileName = "cornellbox.gltf";
}

int main()
{
	try {
		Engine engine = Engine();
		fastgltf::Asset asset = gltf::getAsset(gltfDirectory, gltfFileName);
		host::SceneResources h_objects = gltf::processAsset(
			asset,
			std::array<uint32_t, 2>{engine.screenDimensions.width, engine.screenDimensions.height},
			gltfDirectory
		);
		device::SceneResources d_objects = h_objects.ToDevice(engine);
		render::Initial initialRender = render::Initial(&engine.device);
		const render::initial::descriptor::GenerateGpuObjects generateGpuObjectsDescriptor = {
			.buffers = {
				.cameraBuffer = d_objects.cameras,
				.transformBuffer = d_objects.transforms,
				.instancePropertiesBuffer = d_objects.instanceProperties,
				.materialBuffer = d_objects.materials,
			},
			.screenDimensions = engine.screenDimensions,
		};
		initialRender.generateGpuObjects(&generateGpuObjectsDescriptor);
	}
	catch (std::exception& err) {
		
		std::cout << "std::Exception: " << err.what() << std::endl;
	}
	catch (...) {
		std::cout << "unknown error" << std::endl;
	}

	return 0;
}

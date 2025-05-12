#pragma once
#include <string>
#include <iostream>
#include "engine.hpp"
#include "gltf/gltf.hpp"
#include <fastgltf/types.hpp>
#include "host/host.hpp"

namespace {
	std::string gltfDirectory = "models/cornellBox/"; //must end with "/"
	std::string gltfFileName = "cornellbox.gltf";
}

int main()
{
	try {
		Engine engine = Engine();
		fastgltf::Asset asset = gltf::getAsset(gltfDirectory, gltfFileName);
		SceneResources h_objects = gltf::processAsset(
			asset,
			std::array<uint32_t, 2>{engine.screenDimensions.width, engine.screenDimensions.height},
			gltfDirectory
		);
		structs::device::Objects d_objects = h_objects.ToDeviceObjects(engine);
	}
	catch (std::exception& err) {
		
		std::cout << "std::Exception: " << err.what() << std::endl;
	}
	catch (...) {
		std::cout << "unknown error" << std::endl;
	}

	return 0;
}

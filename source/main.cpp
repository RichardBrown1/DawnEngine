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
		auto up_asset = std::unique_ptr<fastgltf::Asset>(gltf::getAsset(gltfDirectory, gltfFileName));
		structs::host::Objects h_objects = gltf::processAsset(
			*up_asset.get(),
			std::array<uint32_t, 2>{engine.screenDimensions.width, engine.screenDimensions.height},
			gltfDirectory
		);
		structs::device::Objects d_objects;
		host::ConvertHostObjects(engine, h_objects, d_objects);
	}
	catch (std::exception& err) {
		
		std::cout << "std::Exception: " << err.what() << std::endl;
	}
	catch (...) {
		std::cout << "unknown error" << std::endl;
	}

	return 0;
}

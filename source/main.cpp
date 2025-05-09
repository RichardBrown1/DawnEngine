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
		Engine dawnEngine = Engine();
		auto up_asset = std::unique_ptr<fastgltf::Asset>(gltf::getAsset(gltfDirectory, gltfFileName));
		host::Objects objects = gltf::processAsset(
			*up_asset.get(),
			std::array<uint32_t, 2>{dawnEngine.screenDimensions.width, dawnEngine.screenDimensions.height},
			gltfDirectory
		);
	}
	catch (std::exception& err) {
		
		std::cout << "std::Exception: " << err.what() << std::endl;
	}
	catch (...) {
		std::cout << "unknown error" << std::endl;
	}

	return 0;
}

#pragma once
#include <string>
#include <iostream>
#include "engine.hpp"
#include "gltf.hpp"
#include <fastgltf/types.hpp>
#include "host/host.hpp"

namespace {
	std::string gltfFilePath = "models/cornellBox/cornellbox.gltf";
}

int main()
{
	try {
		Engine dawnEngine = Engine();
		auto up_asset = std::unique_ptr<fastgltf::Asset>(gltf::getAsset(gltfFilePath));
	//	const screenDimension = std::array<uint32_t,
		host::Objects objects = gltf::processAsset(
			*up_asset.get(),
			std::array<uint32_t, 2>{dawnEngine.screenDimensions.width, dawnEngine.screenDimensions.height});
	}
	catch (std::exception& err) {
		std::cout << "std::Exception: " << err.what() << std::endl;
	}
	catch (...) {
		std::cout << "unknown error" << std::endl;
	}

	return 0;
}

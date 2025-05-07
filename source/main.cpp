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
		auto up_objects = std::unique_ptr<host::Objects>(gltf::processAsset(up_asset.get()));
		
	}
	catch (std::exception& err) {
		std::cout << "std::Exception: " << err.what() << std::endl;
	}
	catch (...) {
		std::cout << "unknown error" << std::endl;
	}

	return 0;
}

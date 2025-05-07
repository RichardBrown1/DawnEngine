#pragma once
#include <string>
#include <iostream>
#include "engine.hpp"
#include "gltf.hpp"
#include <fastgltf/types.hpp>

namespace {
	std::string gltfFilePath = "models/cornellBox/cornellbox.gltf";
}

int main()
{
	try {
		Engine dawnEngine = Engine();
		fastgltf::Asset* asset = gltf::getAsset(gltfFilePath);
		
		//gltf::processSceneNodes(asset);
		
	}
	catch (std::exception& err) {
		std::cout << "std::Exception: " << err.what() << std::endl;
	}
	catch (...) {
		std::cout << "unknown error" << std::endl;
	}

	return 0;
}

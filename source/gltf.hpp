#include "glm/glm.hpp"
#include "fastgltf/core.hpp"
#include "fastgltf/tools.hpp"
#include "fastgltf/types.hpp"
#include "absl/log/log.h"

namespace gltf {
	fastgltf::Asset* getAsset(std::string gltfFilePath) {
		fastgltf::Parser parser = fastgltf::Parser::Parser(fastgltf::Extensions::KHR_lights_punctual);

		auto gltfFile = fastgltf::GltfDataBuffer::FromPath(gltfFilePath);
		if (gltfFile.error() != fastgltf::Error::None) {// "cube databuffer fromPath");
			LOG(ERROR) << "can't load gltf file";
		}

		auto wholeGltf = parser.loadGltf(gltfFile.get(), "models/cornellBox", fastgltf::Options::LoadExternalBuffers);
		if (wholeGltf.error() != fastgltf::Error::None) {
			LOG(ERROR) << "can't load whole gltf";
		}
		
		return &wholeGltf.get();
	}
};
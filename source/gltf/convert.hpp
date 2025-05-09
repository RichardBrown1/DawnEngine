#include "fastgltf/types.hpp"
#include "../host/structs.hpp"
#include <webgpu/webgpu_cpp.h>

namespace gltf {
	//Used to convert any fastgltf types
	//Not used for processing 
	namespace convert {

		host::structs::TextureInfo textureInfo(const fastgltf::TextureInfo &textureInfo)
		{
			host::structs::TextureInfo dawnEngineTextureInfo = {
				.index = static_cast<uint32_t>(textureInfo.textureIndex),
				.texCoord = static_cast<uint32_t>(textureInfo.texCoordIndex),
			};
			return dawnEngineTextureInfo;
		}

		wgpu::AddressMode convertType(fastgltf::Wrap wrap) {
		switch (wrap) {
		case fastgltf::Wrap::ClampToEdge:
			return wgpu::AddressMode::ClampToEdge;
		case fastgltf::Wrap::MirroredRepeat:
			return wgpu::AddressMode::MirrorRepeat;
		case fastgltf::Wrap::Repeat:
			return wgpu::AddressMode::Repeat;
		default:
			throw std::runtime_error("Invalid wrap conversion");
		}
	};

	wgpu::FilterMode convertFilter(fastgltf::Filter filter) {
		switch (filter) {
		case fastgltf::Filter::Linear:
		case fastgltf::Filter::LinearMipMapLinear:
		case fastgltf::Filter::LinearMipMapNearest:
			return wgpu::FilterMode::Linear;
		case fastgltf::Filter::Nearest:
		case fastgltf::Filter::NearestMipMapLinear:
		case fastgltf::Filter::NearestMipMapNearest:
			return wgpu::FilterMode::Nearest;
		default:
			throw std::runtime_error("Invalid filter conversion");
		}
	};

	wgpu::MipmapFilterMode convertMipMapFilter(fastgltf::Filter filter) {
		switch (filter) {
		case fastgltf::Filter::Linear:
		case fastgltf::Filter::Nearest:
			return wgpu::MipmapFilterMode::Undefined;
		case fastgltf::Filter::LinearMipMapLinear:
		case fastgltf::Filter::NearestMipMapLinear:
			return wgpu::MipmapFilterMode::Linear;
		case fastgltf::Filter::LinearMipMapNearest:
		case fastgltf::Filter::NearestMipMapNearest:
			return wgpu::MipmapFilterMode::Nearest;
		default:
			throw std::runtime_error("Invalid mipmap filter conversion");
		}
	};
	}
}
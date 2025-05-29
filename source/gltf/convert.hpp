#pragma once
#include "fastgltf/types.hpp"
#include "../structs/host.hpp"
#include <webgpu/webgpu_cpp.h>

namespace gltf {
	//Used to convert any fastgltf types
	//Not used for processing 
	namespace convert {

		void textureInfo(
			const std::optional<fastgltf::TextureInfo>& texInfo, 
			structs::TextureInfo& outTextureInfo
		) {
			if (texInfo.has_value()) {
				const fastgltf::TextureInfo& textureInfo = texInfo.value();
				outTextureInfo = {
					.index = static_cast<uint32_t>(textureInfo.textureIndex),
					.texCoord = static_cast<uint32_t>(textureInfo.texCoordIndex),
				};
			}
			else {
				outTextureInfo = {
					.index = UINT32_MAX,
					.texCoord = UINT32_MAX,
				};
			}
		}

		void normalTextureInfo(
			const std::optional<fastgltf::NormalTextureInfo>& texInfo, 
			structs::TextureInfo& outTextureInfo, 
			float& outScale 
		)	{
			if (texInfo.has_value()) {
				const fastgltf::NormalTextureInfo& textureInfo = texInfo.value();
				outTextureInfo = {
					.index = static_cast<uint32_t>(textureInfo.textureIndex),
					.texCoord = static_cast<uint32_t>(textureInfo.texCoordIndex),
				};
				outScale = textureInfo.scale;
			}
			else {
				outTextureInfo = {
					.index = UINT32_MAX,
					.texCoord = UINT32_MAX,
				};
				outScale = UINT32_MAX;
			}
		}

		wgpu::AddressMode convertType(const fastgltf::Wrap wrap) {
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

	wgpu::FilterMode convertFilter(const fastgltf::Filter filter) {
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

	wgpu::MipmapFilterMode convertMipMapFilter(const fastgltf::Filter filter) {
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
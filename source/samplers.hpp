#pragma once
#include <webgpu/webgpu_cpp.h>
#include <fastgltf/types.hpp>
#include "structs.hpp"
#include "utilities.hpp"

namespace DawnEngine {
	void getSamplers(wgpu::Device _device, std::vector<fastgltf::Sampler>& inSamplers, DawnEngine::Samplers& outSamplers) {
		const auto convertAddressMode = [](fastgltf::Wrap wrap) {
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

		const auto convertFilter = [](fastgltf::Optional<fastgltf::Filter> filter) {
			if (!filter.has_value()) {
				return wgpu::FilterMode::Nearest; //default value in wgpu::SamplerDescriptor
			}
			switch (filter.value()) {
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

		const auto convertMipMapFilter = [](fastgltf::Optional<fastgltf::Filter> filter) {
			if (!filter.has_value()) {
				return wgpu::MipmapFilterMode::Nearest; //default value in wgpu::SamplerDescriptor
			}
			switch (filter.value()) {
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

		wgpu::SamplerDescriptor samplerDescriptor;
		if(inSamplers.size() > 0) {
			const auto s = inSamplers[0];
			samplerDescriptor = {
				.label = s.name.c_str(),
				.addressModeU = convertAddressMode(s.wrapS),
				.addressModeV = convertAddressMode(s.wrapT),
				.magFilter = convertFilter(s.magFilter),
				.minFilter = convertFilter(s.minFilter),
				.mipmapFilter = convertMipMapFilter(s.minFilter),
			};
		}
		else {
			 samplerDescriptor = {
					.label = "default sampler",
					.addressModeU = wgpu::AddressMode::ClampToEdge,
					.addressModeV = wgpu::AddressMode::ClampToEdge,
					.magFilter = wgpu::FilterMode::Linear,
					.minFilter = wgpu::FilterMode::Linear,
					.mipmapFilter = wgpu::MipmapFilterMode::Undefined
			};
		}
		outSamplers.texture = _device.CreateSampler(&samplerDescriptor);



	}
}

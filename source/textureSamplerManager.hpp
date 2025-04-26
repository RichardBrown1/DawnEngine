#include <vector>
#include <webgpu/webgpu_cpp.h>
#include <fastgltf/types.hpp>

#include "structs.hpp"

class TextureSamplerManager {
	public:
		TextureSamplerManager(wgpu::Device device);
		void addAsset(fastgltf::Asset& asset, std::string gltfDirectory);

	private:
		wgpu::Device _device;
		std::vector<DawnEngine::SamplerTexturePair> _samplerTexturePair;
		std::vector<wgpu::Texture> _textures;
		std::vector<wgpu::TextureView> _textureViews;
		std::vector<wgpu::Sampler> _samplers;

		void addTextureSamplerPair(fastgltf::Texture texture);
		void addTexture(fastgltf::DataSource dataSource, std::string gltfDirectory);
		void addSampler(fastgltf::Sampler sampler);
};

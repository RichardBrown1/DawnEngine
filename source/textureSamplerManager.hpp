#include <vector>
#include <webgpu/webgpu_cpp.h>
#include <fastgltf/types.hpp>

#include "structs.hpp"

struct GenerateTexturePipelineDescriptor {
	wgpu::TextureView colorTextureView;
	wgpu::TextureFormat colorTextureFormat;
};

struct textureBindGroupLayoutDescriptor {
	wgpu::TextureFormat accumulatorTextureFormat;
	wgpu::SamplerBindingType inputSamplerBindingType;
};

class TextureSamplerManager {
	public:
		TextureSamplerManager(wgpu::Device device);
		void addAsset(fastgltf::Asset& asset, std::string gltfDirectory);
		void doTextureCommands(wgpu::CommandEncoder& commandEncoder);
		wgpu::ComputePipeline generateTexturePipeline(GenerateTexturePipelineDescriptor descriptor);

	private:
		wgpu::Device _device;
		wgpu::ComputePipeline _computePipeline;
		wgpu::Extent2D _workgroupSize;
		std::vector<DawnEngine::SamplerTexturePair> _samplerTexturePair;
		std::vector<wgpu::Texture> _textures;
		std::vector<wgpu::TextureView> _textureViews;
		std::vector<wgpu::Sampler> _samplers;

		wgpu::BindGroupLayout getBindGroupLayout(wgpu::TextureFormat accumulatorTextureFormat);
		void addTextureSamplerPair(fastgltf::Texture texture);
		void addTexture(fastgltf::DataSource dataSource, std::string gltfDirectory);
		void addSampler(fastgltf::Sampler sampler);
};

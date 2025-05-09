#pragma once
#include "glm/glm.hpp"
#include <webgpu/webgpu_cpp.h>
	
namespace host {
	namespace structs {
//		struct TextureViews {
//			std::vector<wgpu::TextureView> shadowMaps;
//			wgpu::TextureView cameraDepth;
//			wgpu::TextureView textures;
//		};
//
//		struct Samplers {
//			wgpu::Sampler depth;
//			wgpu::Sampler texture;
//		};
//
//		struct BindGroups {
//			wgpu::BindGroup fixed;
//			wgpu::BindGroup lights;
//		};
//
//		struct RenderPipelines {
//			wgpu::RenderPipeline shadow;
//			wgpu::RenderPipeline geometry;
//		};

		//Host Camera - This will be a f32mat4x4 projectionView at device
		struct H_Camera {
			glm::f32mat4x4 projection;
			glm::f32vec3 position;
			glm::f32vec3 forward;
		};

		struct VBO {
			glm::f32vec3 vertex;
			glm::f32vec3 normal;
			glm::f32vec2 texcoord;
		};

		struct DrawCall {
			uint32_t indexCount;
			uint32_t instanceCount;
			uint32_t firstIndex;
			uint32_t baseVertex;
			uint32_t firstInstance; //requires indirect-first-instance feature
		};

		struct Light { //glm version of fastgltf::Light
			glm::f32mat4x4 lightSpaceMatrix;
			glm::f32vec3 position;
			uint32_t PAD0;
			glm::f32vec3 rotation; //TODO: check to see if alignas will do the trick
			uint32_t PAD1;
			glm::f32vec3 color;
			glm::u32 type; //this is u32 instead of u8 for webgpu shader compatibility
			glm::f32 intensity;
			glm::f32 range;
			glm::f32 innerConeAngle;
			glm::f32 outerConeAngle;
		};

		struct InstanceProperty { //TODO: is this padding necessary?
			uint32_t materialIndex;
			uint32_t PAD0;
			uint32_t PAD1;
			uint32_t PAD2;
		};

		struct SamplerTexturePair {
			uint32_t samplerIndex;
			uint32_t textureIndex;
		//	DawnEngine::TextureType textureType;
		};

		struct TextureInfo {
			uint32_t index;
			uint32_t texCoord;
		};

		struct PBRMetallicRoughness {
			glm::f32vec4 baseColorFactor;
			float metallicFactor;
			float roughnessFactor;
			TextureInfo baseColorTextureInfo;
			TextureInfo metallicRoughnessTextureInfo;
			uint32_t PAD0;
			uint32_t PAD1;
		};

		struct Material {
			PBRMetallicRoughness pbrMetallicRoughness;
		};

		struct Sampler {
			wgpu::AddressMode addressModeU;
			wgpu::AddressMode addressModeV;
			wgpu::FilterMode magFilter;
			wgpu::FilterMode minFilter;
			wgpu::MipmapFilterMode mipmapFilter;
		};
	}
}
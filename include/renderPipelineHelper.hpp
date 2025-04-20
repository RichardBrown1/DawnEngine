#pragma once
#include <vector>
#include <iostream>
#include <webgpu/webgpu_cpp.h>
#include "../include/structs.hpp"


//Consider RenderBundles
namespace RenderPipelineHelper {

	struct RenderPipelineHelperDescriptor {
		wgpu::Device& device;
		DawnEngine::Buffers& buffers;
		DawnEngine::TextureViews& textureViews;
		DawnEngine::Samplers& samplers;
		DawnEngine::BindGroups& bindGroups;
		wgpu::ShaderModule& vertexShaderModule;
		wgpu::ShaderModule& fragmentShaderModule;
		wgpu::TextureFormat& colorTargetStateFormat;
	};

	wgpu::RenderPipeline createOutputRenderPipeline(RenderPipelineHelperDescriptor& descriptor);
	wgpu::RenderPipeline createShadowRenderPipeline(RenderPipelineHelperDescriptor& descriptor);
};

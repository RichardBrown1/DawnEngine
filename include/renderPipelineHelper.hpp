#pragma once
#include <vector>
#include <webgpu/webgpu_cpp.h>
#include "../include/structs.hpp"


//Consider RenderBundles
namespace RenderPipelineHelper {

	struct RenderPipelineHelperDescriptor {
		wgpu::Device& device;
		Buffers& buffers;
		std::optional<TextureViews> textureViews;
		wgpu::BindGroup* p_bindGroup;
		uint32_t bindGroupCount; //to be implemented
		wgpu::ShaderModule& vertexShaderModule;
		wgpu::ShaderModule& fragmentShaderModule;
		wgpu::TextureFormat& colorTargetStateFormat;
	};

	wgpu::RenderPipeline createGeometryRenderPipeline(RenderPipelineHelperDescriptor& descriptor);
	
	wgpu::RenderPipeline createOutputRenderPipeline(RenderPipelineHelperDescriptor& descriptor);
};

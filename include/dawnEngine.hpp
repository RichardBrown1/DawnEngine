#pragma once
#include <iostream>

#include <webgpu/webgpu_cpp.h>

class DawnEngine {

public:
	DawnEngine();
	void run();
	void destroy();

private:
	wgpu::Instance _instance;
	wgpu::Device _device;
	wgpu::Surface _surface;
	wgpu::SurfaceConfiguration _surfaceConfiguration;
	wgpu::Queue _queue;
	wgpu::RenderPipeline _renderPipeline;
	wgpu::Buffer _vertexBuffer;
	wgpu::Buffer _uniformBuffer;
	wgpu::Buffer _indexBuffer;
	wgpu::BindGroup _bindGroup;

	void initBuffers();
	void initRenderPipeline();

	void draw();
	void updateUniformBuffers();
	wgpu::TextureView getNextSurfaceTextureView();
};
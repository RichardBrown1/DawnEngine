#pragma once
#include <iostream>

#include <webgpu/webgpu_cpp.h>

class DawnEngine {

public:
	DawnEngine();
	void run();
	void destroy();

private:
	wgpu::Device _device;
	wgpu::Surface _surface;
	wgpu::Queue _queue;
	wgpu::RenderPipeline _renderPipeline;

	void draw();
	wgpu::TextureView getNextSurfaceTextureView();
};
#pragma once
#include <absl/base/log_severity.h>
#include <dawn/webgpu_cpp.h>

namespace {
	constexpr absl::LogSeverityAtLeast LOG_LEVEL = absl::LogSeverityAtLeast::kInfo;
	const std::string WINDOW_TITLE = "Dawn WebGPU Engine";
}

class WGPUContext {
public:
	wgpu::Instance instance;
	wgpu::Adapter adapter;
	wgpu::Device device;
	wgpu::Queue queue;

	wgpu::Surface surface;
	wgpu::TextureFormat surfaceFormat = wgpu::TextureFormat::BGRA8Unorm;

	WGPUContext();
	wgpu::Extent2D getScreenDimensions();
	wgpu::Buffer& getScreenDimensionsBuffer();
	void setScreenDimensions(wgpu::Extent2D ScreenDimensions);

private:
	wgpu::Extent2D _screenDimensions = { 1280, 720 };
	wgpu::Buffer _screenDimensionsBuffer;

};


#pragma once
#include <absl/base/log_severity.h>
#include <webgpu/webgpu_cpp.h>

namespace {
	constexpr absl::LogSeverityAtLeast LOG_LEVEL = absl::LogSeverityAtLeast::kInfo;
	const std::string WINDOW_TITLE = "Dawn WebGPU Engine";
}

struct WGPUContext {
	wgpu::Device device;
	wgpu::Queue queue;

	wgpu::Instance instance;
	wgpu::Adapter adapter;
	wgpu::Surface surface;
	
	wgpu::Extent2D screenDimensions = { 1280, 720 };
	wgpu::TextureFormat surfaceFormat = wgpu::TextureFormat::BGRA8Unorm;

	WGPUContext();
};


#pragma once
#include <absl/base/log_severity.h>
#include <webgpu/webgpu_cpp.h>

namespace {
	constexpr absl::LogSeverityAtLeast LOG_LEVEL = absl::LogSeverityAtLeast::kInfo;
	const std::string WINDOW_TITLE = "Dawn WebGPU Engine";
}

class Engine {
public:
	wgpu::Instance instance;
	wgpu::Adapter adapter;
	wgpu::Device device;
	wgpu::Queue queue;
	wgpu::Surface surface;
	
	wgpu::Extent2D screenDimensions = { 1280, 720 };

	Engine();
};


#include "absl/log/globals.h"
#include <webgpu/webgpu_cpp.h>

namespace {
	constexpr absl::LogSeverityAtLeast LOG_LEVEL = absl::LogSeverityAtLeast::kInfo;
}

struct Engine {
	wgpu::Instance instance;
	wgpu::Adapter adapter;
	wgpu::Device device;

	wgpu::Surface surface;
	wgpu::Extent2D screenDimensions = { 1280, 720 };
	
	void initEngine();
};


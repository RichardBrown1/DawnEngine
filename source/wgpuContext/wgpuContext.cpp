#pragma once
#include <string>
#define SDL_MAIN_HANDLED
#include "../sdl3webgpu.hpp"
#include "SDL3/SDL.h"
#include "absl/log/check.h"
#include "absl/log/initialize.h"
#include "absl/log/globals.h"
#include "../print.hpp"
#include "../device/callback.hpp"
#include "wgpuContext.hpp"

WGPUContext::WGPUContext() {
	absl::SetStderrThreshold(LOG_LEVEL);
	absl::InitializeLog();

	SDL_Init(SDL_INIT_VIDEO);
	SDL_Window* p_sdl_window = SDL_CreateWindow(WINDOW_TITLE.c_str(), static_cast<int>(this->screenDimensions.width), static_cast<int>(this->screenDimensions.height), 0);

	CHECK(p_sdl_window);

	constexpr wgpu::InstanceDescriptor instanceDescriptor = {
		.capabilities = {
			.timedWaitAnyEnable = true,
		},
	};
	this->instance = wgpu::CreateInstance(&instanceDescriptor);
	CHECK(instance);

	this->surface = wgpu::Surface(SDL_GetWGPUSurface(this->instance, p_sdl_window));
	CHECK(surface);

	const wgpu::RequestAdapterOptions requestAdapterOptions = {
		.powerPreference = wgpu::PowerPreference::HighPerformance,
	};
	this->instance.WaitAny(this->instance.RequestAdapter(
		&requestAdapterOptions,
		wgpu::CallbackMode::WaitAnyOnly,
		[&](wgpu::RequestAdapterStatus status,
			wgpu::Adapter a,
			wgpu::StringView message) {
				if (status != wgpu::RequestAdapterStatus::Success) {
					LOG(FATAL) << message;
				};
				this->adapter = std::move(a);
		}),
		INT64_MAX);
	CHECK(adapter);

	print::adapter::GetInfo(this->adapter);
	print::adapter::GetLimits(this->adapter);

	const std::array<wgpu::FeatureName, 4> requiredFeatures = {
		wgpu::FeatureName::IndirectFirstInstance,
		wgpu::FeatureName::TextureCompressionBC,
		wgpu::FeatureName::BGRA8UnormStorage,
		wgpu::FeatureName::Unorm16TextureFormats,
	};
	
	constexpr wgpu::Limits requiredLimits = {
		.maxColorAttachmentBytesPerSample = 64,
	};

	wgpu::DeviceDescriptor deviceDescriptor = {};
	deviceDescriptor.label = "device";
	deviceDescriptor.requiredFeatures = requiredFeatures.data();
	deviceDescriptor.requiredFeatureCount = requiredFeatures.size();
	deviceDescriptor.requiredLimits = &requiredLimits;
	deviceDescriptor.SetUncapturedErrorCallback(device::callback::uncapturedError);
	deviceDescriptor.SetDeviceLostCallback(wgpu::CallbackMode::AllowSpontaneous, device::callback::deviceLost);

	device = adapter.CreateDevice(&deviceDescriptor);
	device.SetLoggingCallback(device::callback::logging);
	queue = device.GetQueue();

	const wgpu::SurfaceConfiguration surfaceConfiguration = {
		.device = device,
		.format = this->surfaceFormat,
		.usage = wgpu::TextureUsage::RenderAttachment,
		.width = screenDimensions.width,
		.height = screenDimensions.height,
		.alphaMode = wgpu::CompositeAlphaMode::Auto,
		.presentMode = wgpu::PresentMode::Immediate,
		};

	surface.Configure(&surfaceConfiguration);
}

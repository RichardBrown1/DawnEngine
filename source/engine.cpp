#pragma once
#include "engine.hpp"
#include <iostream>
#include <stdexcept>
#include <string>
#define SDL_MAIN_HANDLED
#include "sdl3webgpu.hpp"
#include "SDL3/SDL.h"
#include "absl/log/check.h"
#include "absl/log/initialize.h"
#include "absl/log/flags.h"
#include "absl/log/globals.h"
#include "print.hpp"
#include "surfaceConfiguration.hpp"


void Engine::initEngine() {
	absl::SetStderrThreshold(LOG_LEVEL);
	absl::InitializeLog();

	SDL_Init(SDL_INIT_VIDEO);
	SDL_Window* p_sdl_window = SDL_CreateWindow("DAWN WebGPU Engine", static_cast<int>(this->screenDimensions.width), static_cast<int>(this->screenDimensions.height), 0);

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
		.powerPreference = wgpu::PowerPreference::HighPerformance
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

	std::array<wgpu::FeatureName, 2> requiredFeatures = {
		wgpu::FeatureName::IndirectFirstInstance,
		wgpu::FeatureName::TextureCompressionBC,
	};
	//wgpu::FeatureName requiredFeatures = wgpu::FeatureName::IndirectFirstInstance;
	wgpu::DeviceDescriptor deviceDescriptor = {};
	deviceDescriptor.label = "device";
	deviceDescriptor.requiredFeatures = requiredFeatures.data();
	deviceDescriptor.requiredFeatureCount = requiredFeatures.size();
	deviceDescriptor.SetUncapturedErrorCallback(
		[](const wgpu::Device&, wgpu::ErrorType type, wgpu::StringView message) {
			const char* errorTypeName = "";
			switch (type) {
			case wgpu::ErrorType::Validation:
				errorTypeName = "Validation";
				break;
			case wgpu::ErrorType::OutOfMemory:
				errorTypeName = "Out of memory";
				break;
			case wgpu::ErrorType::Internal:
				errorTypeName = "Internal";
				break;
			case wgpu::ErrorType::Unknown:
				errorTypeName = "Unknown";
				break;
			default:
				LOG(FATAL) << "Unknown wgpu::Error Type";
			}
			LOG(ERROR) << errorTypeName << " error: " << message;
		}
	);

	deviceDescriptor.SetDeviceLostCallback(
		wgpu::CallbackMode::AllowSpontaneous,
		[](const wgpu::Device&, wgpu::DeviceLostReason reason, wgpu::StringView message) {
			std::string reasonName = "";
			switch (reason) {
			case wgpu::DeviceLostReason::Unknown:
				reasonName = "Unknown";
				break;
			case wgpu::DeviceLostReason::Destroyed:
				reasonName = "Destroyed";
				break;
			case wgpu::DeviceLostReason::CallbackCancelled:
				reasonName = "CallbackCancelled";
				break;
			case wgpu::DeviceLostReason::FailedCreation:
				reasonName = "FailedCreation";
				break;
			default:
				LOG(FATAL) << "Unknown wgpu::DeviceLostReason";
			}
			LOG(ERROR) << "Device lost because of " << reasonName << ": " << message;
		}
	);
	device = adapter.CreateDevice(&deviceDescriptor);

	device.SetLoggingCallback(
		[](wgpu::LoggingType type, wgpu::StringView message) {
			std::string loggingType;
			switch (type) {
			case wgpu::LoggingType::Verbose:
				loggingType = "Verbose";
				break;
			case wgpu::LoggingType::Info:
				loggingType = "Info";
				break;
			case wgpu::LoggingType::Warning:
				loggingType = "Warning";
				break;
			case wgpu::LoggingType::Error:
				loggingType = "Error";
				break;
			default:
				LOG(FATAL) << "Unknown wgpu::LoggingType";
			};
			LOG(ERROR) << loggingType << message << std::endl;
		});

	wgpu::SurfaceCapabilities surfaceCapabilities;
	wgpu::ConvertibleStatus getCapabilitiesStatus = this->surface.GetCapabilities(adapter, &surfaceCapabilities);
	if (getCapabilitiesStatus == wgpu::Status::Error) {
		throw std::runtime_error("failed to get surface capabilities");
	}
	const wgpu::SurfaceConfiguration surfaceConfiguration = surfaceConfiguration::get(this->device, this->screenDimensions);
	surface.Configure(&surfaceConfiguration);
}
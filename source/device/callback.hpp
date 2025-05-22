#pragma once
#include <webgpu/webgpu_cpp.h>
#include "absl/log/log.h"

namespace device {
	namespace callback {
		const auto uncapturedError = [](const wgpu::Device&, wgpu::ErrorType type, wgpu::StringView message) {
			std::string errorTypeName = "";
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
			__debugbreak();
			};

		const auto deviceLost = [](const wgpu::Device&, wgpu::DeviceLostReason reason, wgpu::StringView message) {
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
			};

		const auto logging = [](wgpu::LoggingType type, wgpu::StringView message) {
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
			};

	};
}
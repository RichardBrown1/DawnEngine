#include <webgpu/webgpu_cpp_print.h>
#include "absl/log/log.h"

namespace print {
	namespace adapter {
		void GetInfo(const wgpu::Adapter& adapter) {
				wgpu::AdapterInfo info{};
				adapter.GetInfo(&info);
				LOG(INFO) << "---Adapter Info---";
				LOG(INFO) << "VendorID: " << std::hex << info.vendorID << std::dec;
				LOG(INFO) << "Vendor: " << info.vendor;
				LOG(INFO) << "Architecture: " << info.architecture;
				LOG(INFO) << "DeviceID: " << std::hex << info.deviceID << std::dec;
				LOG(INFO) << "Name: " << info.device;
				LOG(INFO) << "Driver description: " << info.description;
				LOG(INFO) << std::endl;
		}

		void GetLimits(const wgpu::Adapter& adapter) {
			wgpu::Limits limits{};
			adapter.GetLimits(&limits);
			LOG(INFO) << "---Adapter Limits---";
			LOG(INFO) << "Vertex Attribute Limit: " << limits.maxVertexAttributes;
			LOG(INFO) << "Vertex Buffer Limit: " << limits.maxVertexBuffers;
			LOG(INFO) << "BindGroup Limit: " << limits.maxBindGroups;
			LOG(INFO) << "Max Bindings/BindGroup Limit: " << limits.maxBindingsPerBindGroup;
			LOG(INFO) << "Max Texture Dimension 2D Limit: " << limits.maxTextureDimension2D;
			LOG(INFO) << "Max Texture Array Layers Limit: " << limits.maxTextureArrayLayers;
			LOG(INFO) << "Max Compute Workgroups Per Dimension: " << limits.maxComputeWorkgroupsPerDimension;
			LOG(INFO) << std::endl;
		}
	}
}
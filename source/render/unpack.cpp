#include "unpack.hpp"
#include <array>
#include "../device/device.hpp"
#include "../enums.hpp"
#include "../texture/texture.hpp"

namespace render {
	Unpack::Unpack(WGPUContext* wgpuContext) : _wgpuContext(wgpuContext) {
		_computeShaderModule = device::createWGSLShaderModule(wgpuContext->device, UNPACK_SHADER_LABEL, UNPACK_SHADER_PATH);
	};

	void Unpack::generateGpuObjects(const DeviceResources* p_deviceResources) {
		createBindGroupLayout(p_deviceResources);
		createComputePipeline();
		createBindGroup(p_deviceResources);
	}

	void Unpack::doCommands(const render::unpack::descriptor::DoCommands* descriptor) {
		wgpu::ComputePassDescriptor computePassDescriptor = {
			.label = "unpack compute pass",
		};
		wgpu::ComputePassEncoder computePassEncoder = descriptor->commandEncoder.BeginComputePass(&computePassDescriptor);
		computePassEncoder.SetPipeline(_computePipeline);
		computePassEncoder.SetBindGroup(0, _bindGroup);
		computePassEncoder.DispatchWorkgroups(_wgpuContext->getScreenDimensions().width, _wgpuContext->getScreenDimensions().height);
		computePassEncoder.End();
	}

	void Unpack::createBindGroupLayout(const DeviceResources* p_deviceResources) {
		const wgpu::BindGroupLayoutEntry packedInfoBindGroupLayoutEntry = {
			.binding = 0,
			.visibility = wgpu::ShaderStage::Compute,
			.storageTexture = {
				.access = wgpu::StorageTextureAccess::ReadOnly,
				.format = p_deviceResources->render->packedInfoTextureFormat,
				.viewDimension = wgpu::TextureViewDimension::e2D,
			},
		};
		const wgpu::BindGroupLayoutEntry materialBindGroupLayoutEntry = {
			.binding = 1,
			.visibility = wgpu::ShaderStage::Compute,
			.buffer = {
				.type = wgpu::BufferBindingType::ReadOnlyStorage,
				.minBindingSize	= sizeof(structs::Material), 
			},
		};
		const wgpu::BindGroupLayoutEntry normalBindGroupLayoutEntry = {
			.binding = 2,
			.visibility = wgpu::ShaderStage::Compute,
			.storageTexture = {
				.access = wgpu::StorageTextureAccess::WriteOnly,
				.format = p_deviceResources->render->normalTextureFormat,
				.viewDimension = wgpu::TextureViewDimension::e2D,
			},
		};
		const wgpu::BindGroupLayoutEntry texCoordBindGroupLayoutEntry = {
			.binding = 3,
			.visibility = wgpu::ShaderStage::Compute,
			.storageTexture = {
				.access = wgpu::StorageTextureAccess::WriteOnly,
				.format = p_deviceResources->render->texCoordTextureFormat,
				.viewDimension = wgpu::TextureViewDimension::e2D,
			},
		};
		const wgpu::BindGroupLayoutEntry baseColorBindGroupLayoutEntry = {
			.binding = 4,
			.visibility = wgpu::ShaderStage::Compute,
			.storageTexture = {
				.access = wgpu::StorageTextureAccess::WriteOnly,
				.format = p_deviceResources->render->baseColorTextureFormat,
				.viewDimension = wgpu::TextureViewDimension::e2D,
			},
		};
		const wgpu::BindGroupLayoutEntry baseColorIdBindGroupLayoutEntry = {
			.binding = 5,
			.visibility = wgpu::ShaderStage::Compute,
			.storageTexture = {
				.access = wgpu::StorageTextureAccess::WriteOnly,
				.format = p_deviceResources->render->baseColorIdTextureFormat,
				.viewDimension = wgpu::TextureViewDimension::e2D,
			},
		};
		const wgpu::BindGroupLayoutEntry normalIdBindGroupLayoutEntry = {
			.binding = 6,
			.visibility = wgpu::ShaderStage::Compute,
			.storageTexture = {
				.access = wgpu::StorageTextureAccess::WriteOnly,
				.format = p_deviceResources->render->normalIdTextureFormat,
				.viewDimension = wgpu::TextureViewDimension::e2D,
			},
		};

		std::array<wgpu::BindGroupLayoutEntry, 7> bindGroupLayoutEntries = {
			packedInfoBindGroupLayoutEntry,
			materialBindGroupLayoutEntry,
			normalBindGroupLayoutEntry,
			texCoordBindGroupLayoutEntry,
			baseColorBindGroupLayoutEntry,
			baseColorIdBindGroupLayoutEntry,
			normalIdBindGroupLayoutEntry,
		};

		const wgpu::BindGroupLayoutDescriptor bindGroupLayoutDescriptor = {
			.label = "unpack bind group layout",
			.entryCount = bindGroupLayoutEntries.size(),
			.entries = bindGroupLayoutEntries.data(),
		};
		_bindGroupLayout = _wgpuContext->device.CreateBindGroupLayout(&bindGroupLayoutDescriptor);
	}

	void Unpack::createComputePipeline() {
		const wgpu::PipelineLayout pipelineLayout = getPipelineLayout();
		wgpu::ComputeState computeState = {
			.module = _computeShaderModule,
			.entryPoint = enums::EntryPoint::COMPUTE,
		};

		const wgpu::ComputePipelineDescriptor computePipelineDescriptor = {
			.label = "unpack compute pipeline",
			.layout = pipelineLayout,
			.compute = computeState,
		};
		_computePipeline = _wgpuContext->device.CreateComputePipeline(&computePipelineDescriptor);
	}

	wgpu::PipelineLayout Unpack::getPipelineLayout() {
		std::array<wgpu::BindGroupLayout, 1> bindGroupLayout = {
			_bindGroupLayout
		};
		const wgpu::PipelineLayoutDescriptor pipelineLayoutDescriptor = {
			.label = "unpack render pipeline layout",
			.bindGroupLayoutCount = bindGroupLayout.size(),
			.bindGroupLayouts = bindGroupLayout.data(),
		};
		return _wgpuContext->device.CreatePipelineLayout(&pipelineLayoutDescriptor);
	}

	void Unpack::createBindGroup(const DeviceResources* p_deviceResources) {
		const wgpu::BindGroupEntry packedInfoBindGroupEntry = {
			.binding = 0,
			.textureView = p_deviceResources->render->packedInfoTextureView,
		};
		const wgpu::BindGroupEntry materialBindGroupEntry = {
			.binding = 1,
			.buffer = p_deviceResources->scene->materials,
		};
		const wgpu::BindGroupEntry normalBindGroupEntry = {
			.binding = 2,
			.textureView = p_deviceResources->render->normalTextureView,
		};
		const wgpu::BindGroupEntry texCoordBindGroupEntry = {
			.binding = 3,
			.textureView = p_deviceResources->render->texCoordTextureView,
		};
		const wgpu::BindGroupEntry baseColorBindGroupEntry = {
			.binding = 4,
			.textureView = p_deviceResources->render->baseColorTextureView,
		};
		const wgpu::BindGroupEntry baseColorIdBindGroupEntry = {
			.binding = 5,
			.textureView = p_deviceResources->render->baseColorIdTextureView,
		};
		const wgpu::BindGroupEntry normalIdBindGroupEntry = {
			.binding = 6,
			.textureView = p_deviceResources->render->normalIdTextureView,
		};
		std::array<wgpu::BindGroupEntry, 7> bindGroupEntries = {
			packedInfoBindGroupEntry,
			materialBindGroupEntry,
			normalBindGroupEntry,
			texCoordBindGroupEntry,
			baseColorBindGroupEntry,
			baseColorIdBindGroupEntry,
			normalIdBindGroupEntry,
		};
		const wgpu::BindGroupDescriptor bindGroupDescriptor = {
			.label = "unpack bind group",
			.layout = _bindGroupLayout,
			.entryCount = bindGroupEntries.size(),
			.entries = bindGroupEntries.data(),
		};
		_bindGroup = _wgpuContext->device.CreateBindGroup(&bindGroupDescriptor);
	}

}

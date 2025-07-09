#include "shadowToCamera.hpp"
#include <array>
#include "../device/device.hpp"
#include "../enums.hpp"

namespace render {

	ShadowToCamera::ShadowToCamera(WGPUContext* wgpuContext)
		: _wgpuContext(wgpuContext) {
		// Load compute shader module
		_computeShaderModule = device::createShaderModule(
			_wgpuContext->device,
			SHADOWTOCAMERA_SHADER_LABEL,
			SHADOWTOCAMERA_SHADER_PATH
		);
	}

	void ShadowToCamera::generateGpuObjects(const DeviceResources* deviceResources) {
		// Create bind group layouts for input and accumulator
		createInputBindGroupLayout();
		createAccumulatorBindGroupLayout(
			deviceResources->render->shadowMapTextureFormat,
			deviceResources->render->worldPositionTextureFormat,
			deviceResources->render->normalTextureFormat
		);

		// Create compute pipeline
		createPipeline();

		// Create bind groups for input and accumulator
		for (auto& shadowMap : deviceResources->render->shadowMapTextureViews) {
			insertInputBindGroup(shadowMap);
		}
		createAccumulatorBindGroup(
			deviceResources->scene->samplers.at(UINT32_MAX),
			deviceResources->render->shadowTextureView,
			deviceResources->render->worldPositionTextureView,
			deviceResources->render->normalTextureView
		);
	}

	void ShadowToCamera::doCommands(const render::shadowToCamera::descriptor::DoCommands* descriptor) {
		// Begin compute pass
		wgpu::ComputePassDescriptor computePassDesc = {
		 .label = "shadowToCamera compute pass",
		};
	  wgpu::ComputePassEncoder computePass = descriptor->commandEncoder.BeginComputePass(&computePassDesc);

		computePass.SetPipeline(_computePipeline);
		computePass.SetBindGroup(0, _accumulatorBindGroup);

		for (auto& bg : _inputBindGroups) {
			computePass.SetBindGroup(1, bg);
			computePass.DispatchWorkgroups(
				_wgpuContext->getScreenDimensions().width,
		  	_wgpuContext->getScreenDimensions().height,
				1
			);
		}
		computePass.End();
	}

	wgpu::PipelineLayout ShadowToCamera::getPipelineLayout() {
		std::array<wgpu::BindGroupLayout, 2> bindGroupLayouts = {
			_inputBindGroupLayout,
			_accumulatorBindGroupLayout
		};
		const wgpu::PipelineLayoutDescriptor pipelineLayoutDescriptor = {
			.label = "shadowToCamera pipeline layout",
			.bindGroupLayoutCount = bindGroupLayouts.size(),
			.bindGroupLayouts = bindGroupLayouts.data(),
		};
		return _wgpuContext->device.CreatePipelineLayout(&pipelineLayoutDescriptor);
	}

	void ShadowToCamera::createAccumulatorBindGroupLayout(
		wgpu::TextureFormat shadowMapTextureFormat,
		wgpu::TextureFormat worldPositionTextureFormat,
		wgpu::TextureFormat normalTextureFormat
	) {
		std::array<wgpu::BindGroupLayoutEntry, 4> entries = {
			wgpu::BindGroupLayoutEntry{
				.binding = 0,
				.visibility = wgpu::ShaderStage::Compute,
				.sampler = {
					.type = wgpu::SamplerBindingType::Filtering,
				}
			},
			wgpu::BindGroupLayoutEntry{
				.binding = 1,
				.visibility = wgpu::ShaderStage::Compute,
				.storageTexture = { 
					.format = shadowMapTextureFormat,
					.viewDimension = wgpu::TextureViewDimension::e2D
				}
			},
			wgpu::BindGroupLayoutEntry{
				.binding = 2,
				.visibility = wgpu::ShaderStage::Compute,
				.storageTexture = { 
					.format = worldPositionTextureFormat,
					.viewDimension = wgpu::TextureViewDimension::e2D
				}
			},
			wgpu::BindGroupLayoutEntry{
				.binding = 3,
				.visibility = wgpu::ShaderStage::Compute,
				.storageTexture = {
					.format = normalTextureFormat,
					.viewDimension = wgpu::TextureViewDimension::e2D
				}
			}
		};
		const wgpu::BindGroupLayoutDescriptor descriptor = {
			.label = "shadowToCamera accumulator bind group layout",
			.entryCount = entries.size(),
			.entries = entries.data()
		};
		_accumulatorBindGroupLayout = _wgpuContext->device.CreateBindGroupLayout(&descriptor);
	}

	void ShadowToCamera::createInputBindGroupLayout() {
		std::array<wgpu::BindGroupLayoutEntry, 1> entries = {
			wgpu::BindGroupLayoutEntry{
				.binding = 0,
				.visibility = wgpu::ShaderStage::Compute,
				.texture = { 
					.sampleType = wgpu::TextureSampleType::Depth, 
					.viewDimension = wgpu::TextureViewDimension::e2D
				}
			}
		};
		const wgpu::BindGroupLayoutDescriptor descriptor = {
			.label = "shadowToCamera input bind group layout",
			.entryCount = entries.size(),
			.entries = entries.data()
		};
		_inputBindGroupLayout = _wgpuContext->device.CreateBindGroupLayout(&descriptor);
	}

	void ShadowToCamera::createPipeline() {
		const wgpu::ComputePipelineDescriptor descriptor = {
			.label = SHADOWTOCAMERA_SHADER_LABEL,
			.layout = getPipelineLayout(),
			.compute = {
				.module = _computeShaderModule,
				.entryPoint = enums::EntryPoint::COMPUTE
			}
		};
		_computePipeline = _wgpuContext->device.CreateComputePipeline(&descriptor);
	}

	void ShadowToCamera::createAccumulatorBindGroup(
		wgpu::Sampler& shadowMapSampler,
		wgpu::TextureView& shadowTextureView,
		wgpu::TextureView& worldPositionTextureView,
		wgpu::TextureView& normalTextureView
	) {
		std::array<wgpu::BindGroupEntry, 4> bindGroupEntries = {
			wgpu::BindGroupEntry{
				.binding = 0,
				.sampler = shadowMapSampler
			},
			wgpu::BindGroupEntry{
				.binding = 1,
				.textureView = shadowTextureView,
			},
			wgpu::BindGroupEntry{
				.binding = 2,
				.textureView = worldPositionTextureView,
			},
			wgpu::BindGroupEntry{
				.binding = 3,
				.textureView = normalTextureView,
			},
		};
		const wgpu::BindGroupDescriptor descriptor = {
			.label = "shadowToCamera accumulator bind group",
			.layout = _accumulatorBindGroupLayout,
			.entryCount = bindGroupEntries.size(),
			.entries = bindGroupEntries.data()
		};
		_accumulatorBindGroup = _wgpuContext->device.CreateBindGroup(&descriptor);
	}

	void ShadowToCamera::insertInputBindGroup(
		wgpu::TextureView& shadowMapTextureView
	) {
		// Assumes default sampler is available at UINT32_MAX
		wgpu::Sampler sampler = _wgpuContext->device.CreateSampler();
		std::array<wgpu::BindGroupEntry, 1> bindGroupEntries = {
			wgpu::BindGroupEntry{
				.binding = 0,
				.textureView = shadowMapTextureView,
			},
		};
		const wgpu::BindGroupDescriptor descriptor = {
			.label = "shadowToCamera input bind group",
			.layout = _inputBindGroupLayout,
			.entryCount = bindGroupEntries.size(),
			.entries = bindGroupEntries.data()
		};
		_inputBindGroups.emplace_back( _wgpuContext->device.CreateBindGroup(&descriptor));
	}

} // namespace render

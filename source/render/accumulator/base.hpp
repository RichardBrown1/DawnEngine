#pragma once
#include <vector>
#include <unordered_map>
#include "../../structs/structs.hpp"
#include <string>
#include "../../wgpuContext/wgpuContext.hpp"
#include <dawn/webgpu_cpp.h>
#include "../../device/device.hpp" // Make sure this is included
#include "../../enums.hpp"        // Make sure this is included
#include <array>
#include <cstdint>

namespace render {
	namespace accumulator {
		namespace descriptor {
			struct GenerateGpuObjects {
				WGPUContext& wgpuContext;

				wgpu::TextureView& accumulatorTextureView;
				wgpu::TextureFormat accumulatorTextureFormat;
				wgpu::TextureView& texCoordTextureView;
				wgpu::TextureFormat texCoordTextureFormat;
				wgpu::TextureView& textureIdTextureView;
				wgpu::TextureFormat textureIdTextureFormat;

				//for input bind group
				std::vector<uint32_t>& stpIds;
				std::vector<structs::SamplerTexturePair>& inputSTPs;
				std::vector<wgpu::TextureView>& allTextureViews;
				std::unordered_map<uint32_t, wgpu::Sampler>& allSamplers;
			};

			struct DoCommands {
				wgpu::CommandEncoder& commandEncoder;
				wgpu::Extent2D& screenDimensions;
			};
		}
	}

	template <typename Derived>
	class BaseAccumulator {
	protected:
		BaseAccumulator(WGPUContext* wgpuContext) : _wgpuContext(wgpuContext) {};

	public:
		void generateGpuObjects(const render::accumulator::descriptor::GenerateGpuObjects* descriptor) {
			_computeShaderModule = device::createWGSLShaderModule(
				_wgpuContext->device,
				static_cast<Derived*>(this)->getAccumulatorShaderLabel(),
				static_cast<Derived*>(this)->getAccumulatorShaderPath()
			);

			createAccumulatorBindGroupLayout(
				descriptor->accumulatorTextureFormat,
				descriptor->texCoordTextureFormat,
				descriptor->textureIdTextureFormat
			);
			createInputBindGroupLayout();
			createComputePipeline();

			createAccumulatorBindGroup(
				descriptor->accumulatorTextureView,
				descriptor->texCoordTextureView,
				descriptor->textureIdTextureView
			);

			std::vector<wgpu::Buffer> inputInfoBuffers;
			for (uint32_t i = 0; i < descriptor->inputSTPs.size(); i++) {
				structs::InputInfo inputInfo = {
					.stpIndex = i
				};
				inputInfoBuffers.emplace_back(
					device::createBuffer(descriptor->wgpuContext, inputInfo, "input info", wgpu::BufferUsage::Uniform)
				);
			};
			for (auto& stpId : descriptor->stpIds) {
				structs::SamplerTexturePair stp = descriptor->inputSTPs.at(stpId);
				insertInputBindGroup(
					inputInfoBuffers[stpId],
					descriptor->allTextureViews[stp.textureIndex],
					descriptor->allSamplers[stp.samplerIndex]
				);
			}
		}

		void doCommands(const render::accumulator::descriptor::DoCommands* descriptor) {
			wgpu::ComputePassDescriptor computePassDescriptor = {
				.label = "accumulator compute pass",
			};
			wgpu::ComputePassEncoder computePassEncoder = descriptor->commandEncoder.BeginComputePass(&computePassDescriptor);
			computePassEncoder.SetPipeline(_computePipeline);
			computePassEncoder.SetBindGroup(0, _accumulatorBindGroup);
			for (auto& bg : _inputBindGroups) {
				computePassEncoder.SetBindGroup(1, bg);
				computePassEncoder.DispatchWorkgroups(descriptor->screenDimensions.width, descriptor->screenDimensions.height);
			}
			computePassEncoder.End();
		}

	private:
		wgpu::ShaderModule _computeShaderModule;

		WGPUContext* _wgpuContext;

		std::vector<wgpu::Buffer> _infoBuffer;

		wgpu::BindGroupLayout _accumulatorBindGroupLayout;
		wgpu::BindGroupLayout _inputBindGroupLayout;
		wgpu::BindGroup _accumulatorBindGroup;
		std::vector<wgpu::BindGroup> _inputBindGroups;
		wgpu::ComputePipeline _computePipeline;

		void createAccumulatorBindGroupLayout(
			const wgpu::TextureFormat accumulatorTextureFormat,
			const wgpu::TextureFormat texCoordTextureFormat,
			const wgpu::TextureFormat textureIdTextureFormat
		) {
			const wgpu::BindGroupLayoutEntry accumulatorBindGroupLayoutEntry = {
			.binding = 0,
			.visibility = wgpu::ShaderStage::Compute,
			.storageTexture = {
				.access = wgpu::StorageTextureAccess::WriteOnly,
				.format = accumulatorTextureFormat,
				.viewDimension = wgpu::TextureViewDimension::e2D,
			},
			};

			const wgpu::BindGroupLayoutEntry texCoordBindGroupLayoutEntry = {
				.binding = 1,
				.visibility = wgpu::ShaderStage::Compute,
				.storageTexture = {
					.access = wgpu::StorageTextureAccess::ReadOnly,
					.format = texCoordTextureFormat,
					.viewDimension = wgpu::TextureViewDimension::e2D,
				},
			};

			const wgpu::BindGroupLayoutEntry textureIdBindGroupLayoutEntry = {
				.binding = 2,
				.visibility = wgpu::ShaderStage::Compute,
				.storageTexture = {
					.access = wgpu::StorageTextureAccess::ReadOnly,
					.format = textureIdTextureFormat,
					.viewDimension = wgpu::TextureViewDimension::e2D,
				},
			};

			std::array<wgpu::BindGroupLayoutEntry, 3> bindGroupLayoutEntries = {
				accumulatorBindGroupLayoutEntry,
				texCoordBindGroupLayoutEntry,
				textureIdBindGroupLayoutEntry
			};

			const wgpu::BindGroupLayoutDescriptor bindGroupLayoutDescriptor = {
				.label = "accumulator accumulator bind group layout",
				.entryCount = bindGroupLayoutEntries.size(),
				.entries = bindGroupLayoutEntries.data(),
			};
			_accumulatorBindGroupLayout = _wgpuContext->device.CreateBindGroupLayout(&bindGroupLayoutDescriptor);

		}

		void createInputBindGroupLayout() {
			const wgpu::BindGroupLayoutEntry inputInfoBindGroupLayoutEntry = {
				.binding = 0,
				.visibility = wgpu::ShaderStage::Compute,
				.buffer = {
					.type = wgpu::BufferBindingType::Uniform,
					.minBindingSize = 16,
				},
			};
			const wgpu::BindGroupLayoutEntry textureBindGroupLayoutEntry = {
				.binding = 1,
				.visibility = wgpu::ShaderStage::Compute,
				.texture = {
					.sampleType = wgpu::TextureSampleType::Float,
					.viewDimension = wgpu::TextureViewDimension::e2D,
				},
			};
			const wgpu::BindGroupLayoutEntry samplerBindGroupLayoutEntry = {
				.binding = 2,
				.visibility = wgpu::ShaderStage::Compute,
				.sampler = {
					.type = wgpu::SamplerBindingType::Filtering,
				},
			};
			std::array<wgpu::BindGroupLayoutEntry, 3> bindGroupLayoutEntries = {
				inputInfoBindGroupLayoutEntry,
				textureBindGroupLayoutEntry,
				samplerBindGroupLayoutEntry,
			};

			const wgpu::BindGroupLayoutDescriptor bindGroupLayoutDescriptor = {
				.label = "input bind group layout",
				.entryCount = bindGroupLayoutEntries.size(),
				.entries = bindGroupLayoutEntries.data(),
			};
			_inputBindGroupLayout = _wgpuContext->device.CreateBindGroupLayout(&bindGroupLayoutDescriptor);
		}


		wgpu::PipelineLayout getPipelineLayout() {
			std::array<wgpu::BindGroupLayout, 2> bindGroupLayout = {
				_accumulatorBindGroupLayout,
				_inputBindGroupLayout,
			};
			const wgpu::PipelineLayoutDescriptor pipelineLayoutDescriptor = {
				.label = "accumulator render pipeline layout",
				.bindGroupLayoutCount = bindGroupLayout.size(),
				.bindGroupLayouts = bindGroupLayout.data(),
			};
			return _wgpuContext->device.CreatePipelineLayout(&pipelineLayoutDescriptor);
		};

		void createComputePipeline() {
			const wgpu::PipelineLayout pipelineLayout = getPipelineLayout();
			wgpu::ComputeState computeState = {
				.module = _computeShaderModule,
				.entryPoint = enums::EntryPoint::COMPUTE,
			};

			const wgpu::ComputePipelineDescriptor computePipelineDescriptor = {
				.label = "accumulator compute pipeline",
				.layout = pipelineLayout,
				.compute = computeState,
			};
			_computePipeline = _wgpuContext->device.CreateComputePipeline(&computePipelineDescriptor);
		}

		void createAccumulatorBindGroup(
			const wgpu::TextureView& accumulatorTextureView,
			const wgpu::TextureView& texCoordTextureView,
			const wgpu::TextureView& textureIdTextureView
		) {
			const wgpu::BindGroupEntry accumulatorBindGroupEntry = {
				.binding = 0,
				.textureView = accumulatorTextureView,
			};
			const wgpu::BindGroupEntry texCoordBindGroupEntry = {
				.binding = 1,
				.textureView = texCoordTextureView,
			};
			const wgpu::BindGroupEntry textureIdBindGroupEntry = {
				.binding = 2,
				.textureView = textureIdTextureView,
			};

			std::array<wgpu::BindGroupEntry, 3> bindGroupEntries = {
				accumulatorBindGroupEntry,
				texCoordBindGroupEntry,
				textureIdBindGroupEntry
			};
			const wgpu::BindGroupDescriptor bindGroupDescriptor = {
				.label = "accumulator accumulator bind group",
				.layout = _accumulatorBindGroupLayout,
				.entryCount = bindGroupEntries.size(),
				.entries = bindGroupEntries.data(),
			};
			_accumulatorBindGroup = _wgpuContext->device.CreateBindGroup(&bindGroupDescriptor);
		}

		void insertInputBindGroup(
			const wgpu::Buffer& inputInfoBuffer,
			const wgpu::TextureView& inputTextureView,
			const wgpu::Sampler& sampler
		) {
			const wgpu::BindGroupEntry inputInfoBindGroupEntry = {
				.binding = 0,
				.buffer = inputInfoBuffer,
			};
			const wgpu::BindGroupEntry textureBindGroupEntry = {
				.binding = 1,
				.textureView = inputTextureView,
			};
			const wgpu::BindGroupEntry samplerBindGroupEntry = {
				.binding = 2,
				.sampler = sampler,
			};
			std::array<wgpu::BindGroupEntry, 3> bindGroupEntries = {
				inputInfoBindGroupEntry,
				textureBindGroupEntry,
				samplerBindGroupEntry,
			};
			const wgpu::BindGroupDescriptor bindGroupDescriptor = {
				.label = "accumulator input bind group",
				.layout = _inputBindGroupLayout,
				.entryCount = bindGroupEntries.size(),
				.entries = bindGroupEntries.data(),
			};
			_inputBindGroups.emplace_back(_wgpuContext->device.CreateBindGroup(&bindGroupDescriptor));
		}
	};
}
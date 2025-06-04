#pragma once
#define SDL_MAIN_HANDLED
#include "../sdl3webgpu.hpp"
#include "SDL3/SDL.h"
#include "../gltf/gltf.hpp"
#include <fastgltf/types.hpp>
#include "../host/host.hpp"
#include "absl/log/log.h"
#include "engine.hpp"
#include "../wgpuContext/wgpuContext.hpp"

namespace {
//	std::string gltfDirectory = "models/cornellBox/"; //must end with "/"
//	std::string gltfFileName = "cornellbox.gltf";

	std::string gltfDirectory = "models/avocado/"; //must end with "/"
	std::string gltfFileName = "Avocado2.gltf";

	wgpu::TextureView getNextSurfaceTextureView(wgpu::Surface surface) {
		wgpu::SurfaceTexture surfaceTexture;
		surface.GetCurrentTexture(&surfaceTexture);
		if (surfaceTexture.status != wgpu::SurfaceGetCurrentTextureStatus::SuccessOptimal) {
			throw std::runtime_error("failed to wgpu::SurfaceTexture from getCurrentTexture()");
		}

		wgpu::Texture texture = wgpu::Texture(surfaceTexture.texture);

		wgpu::TextureViewDescriptor textureViewDescriptor = {
			.label = "surface TextureView",
			.format = texture.GetFormat(),
			.dimension = wgpu::TextureViewDimension::e2D,
			.mipLevelCount = 1,
			.arrayLayerCount = 1,
			.aspect = wgpu::TextureAspect::All,
			.usage = wgpu::TextureUsage::RenderAttachment,
		};

		wgpu::TextureView textureView = texture.CreateView(&textureViewDescriptor);

		return textureView;
	}

}

Engine::Engine() {
	fastgltf::Asset asset = gltf::getAsset(gltfDirectory, gltfFileName);
	host::SceneResources h_objects = gltf::processAsset(
		asset,
		std::array<uint32_t, 2>{_wgpuContext.screenDimensions.width, _wgpuContext.screenDimensions.height},
		gltfDirectory
	);
	_drawCalls = h_objects.drawCalls;
	_deviceSceneResources = h_objects.ToDevice(_wgpuContext);

	render::Initial* initialRender = new render::Initial(&_wgpuContext);
	_initialRender = initialRender;
	const render::initial::descriptor::GenerateGpuObjects initialGenerateGpuObjectsDescriptor = {
		.cameraBuffer = _deviceSceneResources.cameras,
		.transformBuffer = _deviceSceneResources.transforms,
		.instancePropertiesBuffer = _deviceSceneResources.instanceProperties,
		.materialBuffer = _deviceSceneResources.materials,
	};
	_initialRender->generateGpuObjects(&initialGenerateGpuObjectsDescriptor);

	render::Shadow* shadowRender = new render::Shadow(&_wgpuContext);
	_shadowRender = shadowRender;
	const render::shadow::descriptor::GenerateGpuObjects shadowGenerateGpuObjectsDescriptor = {
		.screenDimensions = _wgpuContext.screenDimensions,
		.transformBuffer = _deviceSceneResources.transforms,
		.lightBuffer = _deviceSceneResources.lights,
	};
	_shadowRender->generateGpuObjects(&shadowGenerateGpuObjectsDescriptor);

	render::Accumulator* baseColorAccumulatorRender = new render::Accumulator(&_wgpuContext);
	_baseColorAccumulatorRender = baseColorAccumulatorRender;
	const render::accumulator::descriptor::GenerateGpuObjects baseColorGenerateGpuObjectsDescriptor = {
		.wgpuContext = _wgpuContext,
		.accumulatorTextureView = _initialRender->baseColorTextureView,
		.accumulatorTextureFormat = _initialRender->baseColorTextureFormat,
		.texCoordTextureView = _initialRender->texCoordTextureView,
		.texCoordTextureFormat = _initialRender->texCoordTextureFormat,
		.textureIdTextureView = _initialRender->baseColorTextureIdTextureView,
		.textureIdTextureFormat = _initialRender->baseColorTextureIdTextureFormat, 
		.inputSTPs = h_objects.samplerTexturePairs,
		.allTextureViews = _deviceSceneResources.textureViews,
		.allSamplers = _deviceSceneResources.samplers,
		};
	_baseColorAccumulatorRender->generateGpuObjects(&baseColorGenerateGpuObjectsDescriptor);
	
	render::Ultimate* ultimateRender = new render::Ultimate(&_wgpuContext.device);
	_ultimateRender = ultimateRender;
	const render::ultimate::descriptor::GenerateGpuObjects ultimateGenerateGpuObjectsDescriptor = {
		.screenDimensions = _wgpuContext.screenDimensions,
		.baseColorTextureFormat = initialRender->baseColorTextureFormat,
		.baseColorTextureView = initialRender->baseColorTextureView,
		.shadowMapTextureView = shadowRender->shadowMapTextureView,
	};
	_ultimateRender->generateGpuObjects(&ultimateGenerateGpuObjectsDescriptor);

	render::ToSurface* toSurfaceRender = new render::ToSurface(&_wgpuContext);
	_toSurfaceRender = toSurfaceRender;
	const render::toSurface::descriptor::GenerateGpuObjects toSurfaceGenerateGpuObjectsDescriptor = {
		.ultimateTextureView = _ultimateRender->ultimateTextureView,
		.surfaceTextureFormat = _wgpuContext.surfaceFormat,
	};
	_toSurfaceRender->generateGpuObjects(&toSurfaceGenerateGpuObjectsDescriptor);
}

void Engine::run() {
	SDL_Event e;
	bool bQuit = false;
	bool stopRendering = false;

	// main loop
	while (!bQuit) {
		// Handle events on queue
		while (SDL_PollEvent(&e) != 0) {
			// close the window when user alt-f4s or clicks the X button
			if (e.type == SDL_EVENT_QUIT) {
				bQuit = true;
			}

			if (e.window.type == SDL_EVENT_WINDOW_MINIMIZED) {
				stopRendering = true;
			}
			if (e.window.type == SDL_EVENT_WINDOW_RESTORED) {
				stopRendering = false;
			}
		}

		// do not draw if we are minimized
		//if (stopRendering) {
		//    // throttle the speed to avoid the endless spinning
		//    std::this_thread::sleep_for(std::chrono::milliseconds(100));
		//    continue;
		//}

		this->draw();
	}
	this->destroy();
}

void Engine::draw() {

	//Get next surface texture view
	wgpu::TextureView surfaceTextureView = getNextSurfaceTextureView(_wgpuContext.surface);

	constexpr wgpu::CommandEncoderDescriptor commandEncoderDescriptor = {
		.label = "My command encoder"
	};
	wgpu::CommandEncoder commandEncoder = _wgpuContext.device.CreateCommandEncoder(&commandEncoderDescriptor);

	const render::initial::descriptor::DoCommands doInitialRenderCommandsDescriptor = {
		.commandEncoder = commandEncoder,
		.vertexBuffer = _deviceSceneResources.vbo,
		.indexBuffer = _deviceSceneResources.indices,
		.drawCalls = _drawCalls,
	};
	_initialRender->doCommands(&doInitialRenderCommandsDescriptor);

	const render::shadow::descriptor::DoCommands doShadowRenderCommandsDescriptor = {
		.commandEncoder = commandEncoder,
		.vertexBuffer = _deviceSceneResources.vbo,
		.indexBuffer = _deviceSceneResources.indices,
		.drawCalls = _drawCalls,
	};
	_shadowRender->doCommands(&doShadowRenderCommandsDescriptor);

	const render::accumulator::descriptor::DoCommands doBaseColorAccumulatorRenderCommandsDescriptor = {
		.commandEncoder = commandEncoder,
		.screenDimensions = _wgpuContext.screenDimensions,
	};
	_baseColorAccumulatorRender->doCommands(&doBaseColorAccumulatorRenderCommandsDescriptor);

	const render::ultimate::descriptor::DoCommands doUltimateRenderCommandsDescriptor = {
		.commandEncoder = commandEncoder,
	};
	_ultimateRender->doCommands(&doUltimateRenderCommandsDescriptor);

	const render::toSurface::descriptor::DoCommands doToSurfaceRenderCommandsDescriptor = {
		.commandEncoder = commandEncoder,
		.surfaceTextureView = surfaceTextureView,
	};
	_toSurfaceRender->doCommands(&doToSurfaceRenderCommandsDescriptor);

	constexpr wgpu::CommandBufferDescriptor commandBufferDescriptor = {
		.label = "Command Buffer",
	};
	wgpu::CommandBuffer commandBuffer = commandEncoder.Finish(&commandBufferDescriptor);

	_wgpuContext.queue.Submit(1, &commandBuffer);

	_wgpuContext.device.Tick();

	_wgpuContext.device.PopErrorScope(
		wgpu::CallbackMode::AllowSpontaneous,
		[](wgpu::PopErrorScopeStatus status, wgpu::ErrorType, wgpu::StringView message) {
			if (wgpu::PopErrorScopeStatus(status) != wgpu::PopErrorScopeStatus::Success) {
				return;
			}
			std::cerr << std::format("Error: {} \r\n", message.data);
		});

	_wgpuContext.surface.Present();

}
	
void Engine::destroy() {
	//device and gpu object destruction is done by dawn destructor
	_wgpuContext.surface.Unconfigure();
	SDL_Quit();
}

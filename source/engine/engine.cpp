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
	HostSceneResources h_objects = HostSceneResources(
		gltfDirectory,
		gltfFileName,
		std::array<uint32_t, 2>{_wgpuContext.getScreenDimensions().width, _wgpuContext.getScreenDimensions().height}
	);
	_drawCalls = h_objects.drawCalls;

	_deviceResources->render = new RenderResources(&_wgpuContext);
	_deviceResources->scene =  h_objects.ToDevice(_wgpuContext);

	render::Initial* initialRender = new render::Initial(&_wgpuContext);
	_initialRender = initialRender;
	_initialRender->generateGpuObjects(_deviceResources);

	_baseColorAccumulatorRender = new render::FourChannel(&_wgpuContext);
	const render::accumulator::descriptor::GenerateGpuObjects baseColorGenerateGpuObjectsDescriptor = {
		.accumulatorTextureView = _deviceResources->render->baseColorTextureView,
		.accumulatorTextureFormat = _deviceResources->render->baseColorTextureFormat,
		.texCoordTextureView = _deviceResources->render->texCoordTextureView,
		.texCoordTextureFormat = _deviceResources->render->texCoordTextureFormat,
		.textureIdTextureView = _deviceResources->render->baseColorIdTextureView,
		.textureIdTextureFormat = _deviceResources->render->baseColorIdTextureFormat,
		.stpIds = h_objects.baseColorStpIds,
		.inputSTPs = h_objects.samplerTexturePairs,
		.allTextureViews = _deviceResources->scene->textureViews,
		.allSamplers = _deviceResources->scene->samplers,
	};
	_baseColorAccumulatorRender->generateGpuObjects(&baseColorGenerateGpuObjectsDescriptor);
	
	_normalAccumulatorRender = new render::FourChannel(&_wgpuContext);
	const render::accumulator::descriptor::GenerateGpuObjects normalGenerateGpuObjectsDescriptor = {
		.accumulatorTextureView = _deviceResources->render->normalTextureView,
		.accumulatorTextureFormat = _deviceResources->render->normalTextureFormat,
		.texCoordTextureView = _deviceResources->render->texCoordTextureView,
		.texCoordTextureFormat = _deviceResources->render->texCoordTextureFormat,
		.textureIdTextureView = _deviceResources->render->normalIdTextureView,
		.textureIdTextureFormat = _deviceResources->render->normalIdTextureFormat,
		.stpIds = h_objects.normalStpIds,
		.inputSTPs = h_objects.samplerTexturePairs,
		.allTextureViews = _deviceResources->scene->textureViews,
		.allSamplers = _deviceResources->scene->samplers,
	};
	_normalAccumulatorRender->generateGpuObjects(&normalGenerateGpuObjectsDescriptor);

	_lightingRender = new render::Lighting(&_wgpuContext);
	_lightingRender->generateGpuObjects(_deviceResources);

	_shadowRender = new render::Shadow(&_wgpuContext);
	_shadowRender->generateGpuObjects(_deviceResources);

	_ultimateRender = new render::Ultimate(&_wgpuContext);
	_ultimateRender->generateGpuObjects(_deviceResources);

	render::ToSurface* toSurfaceRender = new render::ToSurface(&_wgpuContext);
	_toSurfaceRender = toSurfaceRender;
	const render::toSurface::descriptor::GenerateGpuObjects toSurfaceGenerateGpuObjectsDescriptor = {
		.ultimateTextureView = _deviceResources->render->ultimateTextureView,
		.surfaceTextureFormat = _wgpuContext.surfaceFormat,
	};
	_toSurfaceRender->generateGpuObjects(&toSurfaceGenerateGpuObjectsDescriptor);

	render::Clear* clearLightingRender = new render::Clear(&_wgpuContext);
	_clearLightingRender = clearLightingRender;
	const render::clear::descriptor::GenerateGpuObjects clearLightingRenderDescriptor = {
		.textureFormat = _deviceResources->render->lightingTextureFormat,
		.textureView = _deviceResources->render->lightingTextureView,
	};
	_clearLightingRender->generateGpuObjects(&clearLightingRenderDescriptor);
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

	const render::clear::descriptor::DoCommands doLightingClearRenderCommandsDescriptor = {
		.commandEncoder = commandEncoder,
	};
	_clearLightingRender->doCommands(&doLightingClearRenderCommandsDescriptor);

	const render::initial::descriptor::DoCommands doInitialRenderCommandsDescriptor = {
		.commandEncoder = commandEncoder,
		.vertexBuffer = _deviceResources->scene->vbo,
		.indexBuffer = _deviceResources->scene->indices,
		.drawCalls = _drawCalls,
		.depthTextureView = _deviceResources->render->depthTextureView,
	};
	_initialRender->doCommands(&doInitialRenderCommandsDescriptor);

	const render::accumulator::descriptor::DoCommands doBaseColorAccumulatorRenderCommandsDescriptor = {
		.commandEncoder = commandEncoder,
	};
	_baseColorAccumulatorRender->doCommands(&doBaseColorAccumulatorRenderCommandsDescriptor);

	const render::accumulator::descriptor::DoCommands doNormalAccumulatorRenderCommandsDescriptor = {
		.commandEncoder = commandEncoder,
	};
	_normalAccumulatorRender->doCommands(&doNormalAccumulatorRenderCommandsDescriptor);

	const render::lighting::descriptor::DoCommands doLightingAccumulatorRenderCommandsDescriptor = {
		.commandEncoder = commandEncoder,
	};
	_lightingRender->doCommands(&doLightingAccumulatorRenderCommandsDescriptor);

	const render::shadow::descriptor::DoCommands doShadowRenderCommandsDescriptor = {
		.commandEncoder = commandEncoder,
		.vertexBuffer = _deviceResources->scene->vbo,
		.indexBuffer = _deviceResources->scene->indices,
		.drawCalls = _drawCalls,
	};
	_shadowRender->doCommands(&doShadowRenderCommandsDescriptor);

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

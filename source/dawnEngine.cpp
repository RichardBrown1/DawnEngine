#include <cstdlib>
#include <iostream>
#include <format>

#define SDL_MAIN_HANDLED

#include "../include/sdl3webgpu.hpp"
#include "SDL3/SDL.h"

#include "../include/DawnEngine.hpp"
#include <dawn/webgpu_cpp_print.h>

static DawnEngine* loadedEngine = nullptr;

const uint32_t WIDTH = 640;
const uint32_t HEIGHT = 480;

const char* shaderSource = R"(
    @vertex
	fn vs_main(@builtin(vertex_index) in_vertex_index: u32) -> @builtin(position) vec4f {
		var p = vec2f(0.0, 0.0);
		if (in_vertex_index == 0u) {
			p = vec2f(-0.5, -0.5);
		} else if (in_vertex_index == 1u) {
			p = vec2f(0.5, -0.5);
		} else {
			p = vec2f(0.0, 0.5);
		}
		return vec4f(p, 0.0, 1.0);
	}

	@fragment
	fn fs_main() -> @location(0) vec4f {
		return vec4f(0.0, 0.4, 1.0, 1.0);
	}
)";

DawnEngine::DawnEngine() {
    //Only 1 engine allowed
    assert(loadedEngine == nullptr);
    loadedEngine = this;

    // Initialise SDL
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* p_sdl_window = SDL_CreateWindow("Learn WebGPU", static_cast<int>(WIDTH), static_cast<int>(HEIGHT), 0);

    if (!p_sdl_window) {
        SDL_Quit();
        throw std::runtime_error("SDL_CreateWindow Error: " + std::string(SDL_GetError()));
    }

    wgpu::InstanceDescriptor instanceDescriptor{};
    instanceDescriptor.features.timedWaitAnyEnable = true;
    wgpu::Instance instance = wgpu::CreateInstance(&instanceDescriptor);
    if (instance == nullptr) {
        throw std::runtime_error("Instance creation failed!\n");
    }

    _surface = wgpu::Surface(SDL_GetWGPUSurface(instance, p_sdl_window));
    if (!_surface) {
        throw std::runtime_error("Failed to get SDL Surface");
    }

    // Synchronously request the adapter.
    wgpu::RequestAdapterOptions requestAdapterOptions = {};
    //requestAdapterOptions.backendType = wgpu::BackendType::Vulkan;

    wgpu::Adapter adapter;
    wgpu::RequestAdapterCallbackInfo callbackInfo = {};
    callbackInfo.nextInChain = nullptr;
    callbackInfo.mode = wgpu::CallbackMode::WaitAnyOnly;
    callbackInfo.callback =
        [](WGPURequestAdapterStatus status, WGPUAdapter adapter,
            WGPUStringView message, void* userdata) {
                if (status != WGPURequestAdapterStatus_Success) {
                    std::cerr << "Failed to get an adapter:" << message.data;
                    return;
                }
                *static_cast<wgpu::Adapter*>(userdata) = wgpu::Adapter::Acquire(adapter);
        };
    callbackInfo.userdata = &adapter;
    instance.WaitAny(instance.RequestAdapter(&requestAdapterOptions, callbackInfo), UINT64_MAX);
    if (adapter == nullptr) {
        throw std::runtime_error("RequestAdapter failed!\n");
    }

    wgpu::DawnAdapterPropertiesPowerPreference power_props{};

    wgpu::AdapterInfo info{};
    info.nextInChain = &power_props;

    adapter.GetInfo(&info);
    std::cout << "VendorID: " << std::hex << info.vendorID << std::dec << "\n";
    std::cout << "Vendor: " << info.vendor << "\n";
    std::cout << "Architecture: " << info.architecture << "\n";
    std::cout << "DeviceID: " << std::hex << info.deviceID << std::dec << "\n";
    std::cout << "Name: " << info.device << "\n";
    std::cout << "Driver description: " << info.description << "\n";

    wgpu::DeviceDescriptor deviceDescriptor{};
    deviceDescriptor.SetUncapturedErrorCallback([](const wgpu::Device&, wgpu::ErrorType type, const char* message) {
        std::cout << "Uncaptured device error type: " << type;
        std::cout << std::format("Uncaptured Error Message: {} \r\n", message);
    });
    deviceDescriptor.SetDeviceLostCallback(
	wgpu::CallbackMode::AllowSpontaneous,
	[](const wgpu::Device&, wgpu::DeviceLostReason reason, const char* message) {
		std::cout << "DeviceLostReason: " << reason;
		std::cout << std::format(" Message: {}", message) << std::endl;
	});

    _device = adapter.CreateDevice(&deviceDescriptor);
    _device.PushErrorScope(wgpu::ErrorFilter::Validation);
    
    wgpu::SurfaceConfiguration surfaceConfiguration{};
    surfaceConfiguration.width = WIDTH;
    surfaceConfiguration.height = HEIGHT;
    surfaceConfiguration.device = _device;
    surfaceConfiguration.alphaMode = wgpu::CompositeAlphaMode::Auto;
    surfaceConfiguration.presentMode = wgpu::PresentMode::Immediate;
    surfaceConfiguration.usage = wgpu::TextureUsage::RenderAttachment;
    
    wgpu::SurfaceCapabilities surfaceCapabilites;    
    wgpu::ConvertibleStatus getCapabilitiesStatus = _surface.GetCapabilities(adapter, &surfaceCapabilites);
    if (getCapabilitiesStatus == wgpu::Status::Error) {
        throw std::runtime_error("failed to get surface capabilities");
    }
    surfaceConfiguration.format = surfaceCapabilites.formats[0]; 
    _surface.Configure(&surfaceConfiguration);
    _queue = _device.GetQueue();

    //Render Pipeline
    wgpu::ShaderSourceWGSL shaderSourceWGSL;
    shaderSourceWGSL.code = shaderSource;
    wgpu::ShaderModuleDescriptor shaderModuleDescriptor = {
        .nextInChain = &shaderSourceWGSL,
    };
    wgpu::ShaderModule shaderModule = _device.CreateShaderModule(&shaderModuleDescriptor);
    
    wgpu::VertexState vertexState = {
        .module = shaderModule,
        .entryPoint = "vs_main",
    };

    wgpu::BlendState blendState = {
        .color = wgpu::BlendComponent{
            .operation = wgpu::BlendOperation::Add,
            .srcFactor = wgpu::BlendFactor::SrcAlpha,
            .dstFactor = wgpu::BlendFactor::OneMinusSrcAlpha,
        },
        .alpha = wgpu::BlendComponent{
            .operation = wgpu::BlendOperation::Add,
            .srcFactor = wgpu::BlendFactor::Zero,
            .dstFactor = wgpu::BlendFactor::One,
        }
    };

    wgpu::ColorTargetState colorTargetState = {
        .format = surfaceConfiguration.format,
        .blend = &blendState,
        .writeMask = wgpu::ColorWriteMask::All,
    };

    wgpu::FragmentState fragmentState = {
        .module = shaderModule,
        .entryPoint = "fs_main",
        .constantCount = 0,
        .constants = nullptr,
        .targetCount = 1,
        .targets = &colorTargetState,
    };

    wgpu::RenderPipelineDescriptor renderPipelineDescriptor = {
        .label = "render pipeline descriptor",
        .layout = nullptr,
        .vertex = vertexState,
        .primitive = wgpu::PrimitiveState {
            .topology = wgpu::PrimitiveTopology::TriangleList,
            .stripIndexFormat = wgpu::IndexFormat::Undefined,
            .frontFace = wgpu::FrontFace::CCW,
            .cullMode = wgpu::CullMode::None,
        },
        .depthStencil = nullptr,
        .multisample = wgpu::MultisampleState {
            .count = 1,
            .mask = ~0u,
            .alphaToCoverageEnabled = false,
        },
        .fragment = &fragmentState,
    };

    _renderPipeline = _device.CreateRenderPipeline(&renderPipelineDescriptor);
}

void DawnEngine::draw() {

    //Get next surface texture view
    wgpu::TextureView surfaceTextureView = getNextSurfaceTextureView();

    wgpu::CommandEncoderDescriptor commandEncoderDescriptor = wgpu::CommandEncoderDescriptor();
    commandEncoderDescriptor.label = "My command encoder";
    wgpu::CommandEncoder commandEncoder = _device.CreateCommandEncoder(&commandEncoderDescriptor);

    wgpu::RenderPassColorAttachment renderPassColorAttachment = {};
    renderPassColorAttachment.view = surfaceTextureView;
    renderPassColorAttachment.loadOp = wgpu::LoadOp::Clear;
    renderPassColorAttachment.storeOp = wgpu::StoreOp::Store;
    renderPassColorAttachment.clearValue = wgpu::Color{ 0.9, 0.1, 0.2, 1.0 };
    
    wgpu::RenderPassDescriptor renderPassDescriptor = {
        .colorAttachmentCount = 1,
        .colorAttachments = &renderPassColorAttachment,
    };
    

    wgpu::RenderPassEncoder renderPassEncoder = commandEncoder.BeginRenderPass(&renderPassDescriptor);
    renderPassEncoder.SetPipeline(_renderPipeline);
    renderPassEncoder.Draw(3, 1, 0, 0);
    renderPassEncoder.End();

    wgpu::CommandBufferDescriptor commandBufferDescriptor = {
        .label = "Command Buffer",
    };

    wgpu::CommandBuffer commandBuffer = commandEncoder.Finish(&commandBufferDescriptor);

    _queue.Submit(1, &commandBuffer);

    _device.Tick();

    wgpu::PopErrorScopeCallbackInfo popErrorScopeCallbackInfo = {};
    popErrorScopeCallbackInfo.callback = [](WGPUPopErrorScopeStatus status, WGPUErrorType, struct WGPUStringView message, void*) {
        if (wgpu::PopErrorScopeStatus(status) != wgpu::PopErrorScopeStatus::Success) {
            return;
        }
        std::cout << std::format("Error: {} \r\n", message.data);
    };
	_device.PopErrorScope(popErrorScopeCallbackInfo);

    _surface.Present();
    
}

wgpu::TextureView DawnEngine::getNextSurfaceTextureView() {
    wgpu::SurfaceTexture surfaceTexture;
	_surface.GetCurrentTexture(&surfaceTexture);
	if (surfaceTexture.status != wgpu::SurfaceGetCurrentTextureStatus::Success) {
        throw std::runtime_error("failed to wgpu::SurfaceTexture from getCurrentTexture()");
	}

    wgpu::Texture texture = wgpu::Texture(surfaceTexture.texture);

    wgpu::TextureViewDescriptor textureViewDescriptor = wgpu::TextureViewDescriptor();
    textureViewDescriptor.label = "surface TextureView";
    textureViewDescriptor.format = texture.GetFormat();
    textureViewDescriptor.dimension = wgpu::TextureViewDimension::e2D;
    textureViewDescriptor.mipLevelCount = 1;
    textureViewDescriptor.arrayLayerCount = 1;
    textureViewDescriptor.aspect = wgpu::TextureAspect::All;

    wgpu::TextureView textureView = texture.CreateView(&textureViewDescriptor);
    
    //texture.Destroy();
    return textureView;
}

void DawnEngine::run() {
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
}

void DawnEngine::destroy() {
    _surface.Unconfigure();
    _device.Destroy();
    //aSDL_DestroyWindow(_p_sdl_window);
    //SDL_Quit();
}
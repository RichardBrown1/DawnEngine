#include <cstdlib>
#include <iostream>
#include <format>
#include <chrono>

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

#define SDL_MAIN_HANDLED
#include "../include/sdl3webgpu.hpp"
#include "SDL3/SDL.h"

#include "../include/DawnEngine.hpp"
#include <dawn/webgpu_cpp_print.h>

#include "../models/cube.hpp"
#include "../include/utilities.hpp"

static DawnEngine* loadedEngine = nullptr;

const uint32_t WIDTH = 640;
const uint32_t HEIGHT = 480;

struct UBO {
	alignas(16) glm::mat4x4 projection;
	alignas(16) glm::mat4x4 model;
	alignas(16) glm::mat4x4 view;
};

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
	_instance = wgpu::CreateInstance(&instanceDescriptor);
	if (_instance == nullptr) {
		throw std::runtime_error("Instance creation failed!\n");
	}

	_surface = wgpu::Surface(SDL_GetWGPUSurface(_instance, p_sdl_window));
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
	_instance.WaitAny(_instance.RequestAdapter(&requestAdapterOptions, callbackInfo), UINT64_MAX);
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

	//limits
	wgpu::SupportedLimits supportedLimits;
	adapter.GetLimits(&supportedLimits);
	std::cout << "Vertex Attribute Limit: " << supportedLimits.limits.maxVertexAttributes << std::endl;
	std::cout << "Vertex Buffer Limit: " << supportedLimits.limits.maxVertexBuffers << std::endl;

	wgpu::DeviceDescriptor deviceDescriptor{};
	deviceDescriptor.SetUncapturedErrorCallback([](const wgpu::Device&, wgpu::ErrorType type, const char* message) {
		std::cout << "Uncaptured device error type: " << type << std::endl;
		std::cout << std::format("Uncaptured Error Message: {} \r\n", message);
		//        wgpu::PopErrorScopeCallbackInfo popErrorScopeCallbackInfo = {
		//            .callback = [](WGPUPopErrorScopeStatus, WGPUErrorType type, struct WGPUStringView message, void*) {
		//                std::cout << "Type: " << type << std::endl;
		//                std::cout << "Pop Error Message: " << message.data << std::endl;
		//            }
		//        };
		//        device.PopErrorScope(popErrorScopeCallbackInfo);
		exit(1);
		});
	deviceDescriptor.SetDeviceLostCallback(
		wgpu::CallbackMode::AllowSpontaneous,
		[](const wgpu::Device&, wgpu::DeviceLostReason reason, const char* message) {
			std::cout << "DeviceLostReason: " << reason << std::endl;
			std::cout << std::format(" Message: {}", message) << std::endl;
		});

	_device = adapter.CreateDevice(&deviceDescriptor);
	//_device.PushErrorScope(wgpu::ErrorFilter::);

	int userData;
	_device.SetLoggingCallback(
		[](WGPULoggingType type, struct WGPUStringView message, void*) {
			std::string_view view = { message.data, message.length };
			std::cout << "Type: " << type << std::endl;
			std::cout << "Log Message: " << view << std::endl;
		}, &userData);

	_surfaceConfiguration.width = WIDTH;
	_surfaceConfiguration.height = HEIGHT;
	_surfaceConfiguration.device = _device;
	_surfaceConfiguration.alphaMode = wgpu::CompositeAlphaMode::Auto;
	_surfaceConfiguration.presentMode = wgpu::PresentMode::Immediate;
	_surfaceConfiguration.usage = wgpu::TextureUsage::RenderAttachment;

	wgpu::SurfaceCapabilities surfaceCapabilites;
	wgpu::ConvertibleStatus getCapabilitiesStatus = _surface.GetCapabilities(adapter, &surfaceCapabilites);
	if (getCapabilitiesStatus == wgpu::Status::Error) {
		throw std::runtime_error("failed to get surface capabilities");
	}
	_surfaceConfiguration.format = wgpu::TextureFormat::BGRA8Unorm;
	_surface.Configure(&_surfaceConfiguration);
	_queue = _device.GetQueue();


	initBuffers();
	initRenderPipeline();
}

void DawnEngine::initBuffers() {
	Cube cube;

	wgpu::BufferDescriptor vertexBufferDescriptor = {
		.label = "vertex buffer",
		.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Vertex,
		.size = sizeof(float) * cube.vertexData.size(),
	};
	_vertexBuffer = _device.CreateBuffer(&vertexBufferDescriptor);
	_queue.WriteBuffer(_vertexBuffer, 0, cube.vertexData.data(), vertexBufferDescriptor.size);

	wgpu::BufferDescriptor indexBufferDescriptor = {
		.label = "index buffer",
		.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Index,
		.size = sizeof(uint16_t) * cube.indexData.size(),
	};
	_indexBuffer = _device.CreateBuffer(&indexBufferDescriptor);
	_queue.WriteBuffer(_indexBuffer, 0, cube.indexData.data(), indexBufferDescriptor.size);

	wgpu::BufferDescriptor uniformBufferDescriptor = {
		.label = "ubo buffer",
		.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Uniform,
		.size = sizeof(UBO),
	};
	_uniformBuffer = _device.CreateBuffer(&uniformBufferDescriptor);
}

void DawnEngine::initRenderPipeline() {
	std::vector<uint32_t> vertexShaderCode = Utilities::readShader(std::string("shaders/v_shader.spv"));
	wgpu::ShaderSourceSPIRV vertexShaderSource = wgpu::ShaderSourceSPIRV();
	vertexShaderSource.codeSize = static_cast<uint32_t>(vertexShaderCode.size());
	vertexShaderSource.code = vertexShaderCode.data();
	wgpu::ShaderModuleDescriptor vertexShaderModuleDescriptor = {
		.nextInChain = &vertexShaderSource,
		.label = "vertex shader module"
	};
	wgpu::ShaderModule vertexShaderModule = _device.CreateShaderModule(&vertexShaderModuleDescriptor);

	wgpu::VertexAttribute positionAttribute = {
		.format = wgpu::VertexFormat::Float32x3,
		.offset = 0,
		.shaderLocation = 0,
	};

	wgpu::VertexBufferLayout vertexBufferLayout = {
		.arrayStride = 3 * sizeof(float),
		.attributeCount = 1,
		.attributes = &positionAttribute,
	};

	wgpu::VertexState vertexState = {
		.module = vertexShaderModule,
		.entryPoint = "VS_main",
		.bufferCount = 1,
		.buffers = &vertexBufferLayout,
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
		.format = _surfaceConfiguration.format,
		.blend = &blendState,
		.writeMask = wgpu::ColorWriteMask::All,
	};

	std::vector<uint32_t> fragmentShaderCode = Utilities::readShader(std::string("shaders/f_shader.spv"));
	wgpu::ShaderSourceSPIRV fragmentShaderSource = wgpu::ShaderSourceSPIRV();
	fragmentShaderSource.codeSize = static_cast<uint32_t>(fragmentShaderCode.size());
	fragmentShaderSource.code = fragmentShaderCode.data();
	wgpu::ShaderModuleDescriptor fragmentShaderModuleDescriptor = {
		.nextInChain = &fragmentShaderSource,
		.label = "fragment shader module"
	};
	wgpu::ShaderModule fragmentShaderModule = _device.CreateShaderModule(&fragmentShaderModuleDescriptor);

	wgpu::FragmentState fragmentState = {
		.module = fragmentShaderModule,
		.entryPoint = "FS_main",
		.constantCount = 0,
		.constants = nullptr,
		.targetCount = 1,
		.targets = &colorTargetState,
	};

	wgpu::BindGroupLayoutEntry bindGroupLayoutEntry = {
		.binding = 0,
		.visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment,
		.buffer = {
			.type = wgpu::BufferBindingType::Uniform,
			.minBindingSize = sizeof(UBO),
		}
	};
	wgpu::BindGroupLayoutDescriptor bindGroupLayoutDescriptor = {
		.label = "Bind Group Layout - Uniform",
		.entryCount = 1,
		.entries = &bindGroupLayoutEntry
	};
	wgpu::BindGroupLayout bindGroupLayout = _device.CreateBindGroupLayout(&bindGroupLayoutDescriptor);

	wgpu::BindGroupEntry bindGroupEntry = {
		.binding = 0,
		.buffer = _uniformBuffer,
		.size = sizeof(UBO),
	};
	wgpu::BindGroupDescriptor bindGroupDescriptor = {
		.label = "uniform bind group",
		.layout = bindGroupLayout,
		.entryCount = bindGroupLayoutDescriptor.entryCount,
		.entries = &bindGroupEntry,
	};
	_bindGroup = _device.CreateBindGroup(&bindGroupDescriptor);

	wgpu::PipelineLayoutDescriptor pipelineLayoutDescriptor = {
		.label = "Pipeline Layout",
		.bindGroupLayoutCount = 1,
		.bindGroupLayouts = &bindGroupLayout,
	};
	wgpu::PipelineLayout pipelineLayout = _device.CreatePipelineLayout(&pipelineLayoutDescriptor);
	

	wgpu::RenderPipelineDescriptor renderPipelineDescriptor = {
		.label = "render pipeline descriptor",
		.layout = pipelineLayout,
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
	updateUniformBuffers();

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
	renderPassEncoder.SetBindGroup(0, _bindGroup, 0, nullptr); //uniform buffer
	renderPassEncoder.SetVertexBuffer(0, _vertexBuffer, 0, _vertexBuffer.GetSize());
	renderPassEncoder.SetIndexBuffer(_indexBuffer, wgpu::IndexFormat::Uint16, 0, _indexBuffer.GetSize());
	renderPassEncoder.DrawIndexed(static_cast<uint32_t>(_indexBuffer.GetSize()) / sizeof(uint16_t));
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

void DawnEngine::updateUniformBuffers() {
	static auto startTime = std::chrono::high_resolution_clock::now();

	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

	UBO ubo{};
	ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.projection = glm::perspective(glm::radians(45.0f), _surfaceConfiguration.width / (float) _surfaceConfiguration.height, 0.1f, 10.0f);

	ubo.projection[1][1] *= -1; //y coordinate is inverted on OpenGL - flip it

	_queue.WriteBuffer(_uniformBuffer, 0, &ubo, sizeof(ubo));
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
#pragma once 
#include <cstdlib>
#include <iostream>
#include <format>
#include <chrono>

#define SDL_MAIN_HANDLED
#include "../include/sdl3webgpu.hpp"
#include "SDL3/SDL.h"

#include "../include/engine.hpp"
#include <dawn/webgpu_cpp_print.h>

#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/string_cast.hpp>

#include <fastgltf/tools.hpp>
#include <fastgltf/types.hpp>

#include "../include/constants.hpp"
#include "../include/utilities.hpp"
#include "../include/renderPipelineHelper.hpp"

static Engine* loadedEngine = nullptr;

const uint32_t WIDTH = 1024;
const uint32_t HEIGHT = 720;

Engine::Engine() {
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
		std::cout << "BindGroup Limit: " << supportedLimits.limits.maxBindGroups << std::endl;
		std::cout << "Max Bindings/BindGroup Limit: " << supportedLimits.limits.maxBindingsPerBindGroup << std::endl;

		std::array<wgpu::FeatureName, 1> requiredFeatures = { 
			wgpu::FeatureName::IndirectFirstInstance
		};
		//wgpu::FeatureName requiredFeatures = wgpu::FeatureName::IndirectFirstInstance;
		wgpu::DeviceDescriptor deviceDescriptor = {};
		deviceDescriptor.label = "device";
		deviceDescriptor.requiredFeatures = requiredFeatures.data();
		deviceDescriptor.requiredFeatureCount = requiredFeatures.size();
		deviceDescriptor.SetUncapturedErrorCallback([](const wgpu::Device&, wgpu::ErrorType type, const char* message) {
			std::cout << "Uncaptured device error type: " << type << std::endl;
			std::cout << std::format("Uncaptured Error Message: {} \r\n", message);
			exit(1);
			});
		deviceDescriptor.SetDeviceLostCallback(
			wgpu::CallbackMode::AllowSpontaneous,
			[](const wgpu::Device&, wgpu::DeviceLostReason reason, const char* message) {
				std::cout << "DeviceLostReason: " << reason << std::endl;
				std::cout << std::format(" Message: {}", message) << std::endl;
			});

		_device = adapter.CreateDevice(&deviceDescriptor);

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

		initGltf();
		initDepthTexture();
		initRenderPipeline();
}


void Engine::initGltf() {
	_gltfParser = fastgltf::Parser::Parser(fastgltf::Extensions::KHR_lights_punctual);

	auto gltfFile = fastgltf::GltfDataBuffer::FromPath("models/cornellbox.gltf");
	//auto gltfFile = fastgltf::GltfDataBuffer::FromPath("models/cube.gltf");
	Utilities::checkFastGltfError(gltfFile.error(), "cube databuffer fromPath");

	auto wholeGltf = _gltfParser.loadGltf(gltfFile.get(), "models", fastgltf::Options::LoadExternalBuffers);
	Utilities::checkFastGltfError(wholeGltf.error(), "cube loadGltf");

	auto& asset = wholeGltf.get();

	initNodes(asset);
	initSceneBuffers();
	initMaterialBuffer(asset);
}


void Engine::initNodes(fastgltf::Asset& asset) {
	//const auto flipX = glm::f32mat4x4(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f);
	//const auto flipX = glm::f32mat4x4(-1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
	size_t sceneIndex = asset.defaultScene.value_or(0);
	fastgltf::iterateSceneNodes(asset, sceneIndex, fastgltf::math::fmat4x4(),
		[&](fastgltf::Node& node, fastgltf::math::fmat4x4 m) {
			glm::f32mat4x4 matrix = Utilities::toGlmFormat(m);// * flipX;
			
			
			if (node.meshIndex.has_value()) {
				addMeshData(asset, matrix, static_cast<uint32_t>(node.meshIndex.value()));
				return;
			}
			if (node.lightIndex.has_value()) {
				addLightData(asset, matrix, static_cast<uint32_t>(node.lightIndex.value()));
				return;
			}
			if (node.cameraIndex.has_value()) {
				addCameraData(asset, matrix, static_cast<uint32_t>(node.cameraIndex.value()));
				return;
			}
			std::cout << "warning: unknown node type: " << node.name << std::endl;
		});
	auto translations = std::vector<glm::f32mat4x4>(0);

}

void Engine::addMeshData(fastgltf::Asset& asset, glm::f32mat4x4& transform, uint32_t meshIndex) {
	if (_meshIndexToDrawInfoMap.count(meshIndex)) {
		++_meshIndexToDrawInfoMap[meshIndex]->instanceCount;
		return;
	};

	auto& mesh = asset.meshes[meshIndex];

	for (auto& primitive : mesh.primitives) {
		_transforms.push_back(transform);

		//vertice
		fastgltf::Attribute& positionAttribute = *primitive.findAttribute("POSITION");
		fastgltf::Accessor& positionAccessor = asset.accessors[positionAttribute.accessorIndex];
		size_t vbosOffset = _vbos.size();
		_vbos.resize(_vbos.size() + positionAccessor.count);
		fastgltf::iterateAccessorWithIndex<fastgltf::math::f32vec3>(
			asset, positionAccessor, [&](fastgltf::math::f32vec3 vertex, size_t i) {
				memcpy(&_vbos[i + vbosOffset].vertex, &vertex, sizeof(glm::f32vec3));
			}
		);
	
		//normal
		fastgltf::Attribute& normalAttribute = *primitive.findAttribute("NORMAL");
		fastgltf::Accessor& normalAccessor = asset.accessors[normalAttribute.accessorIndex];
		fastgltf::iterateAccessorWithIndex<fastgltf::math::f32vec3>(
			asset, normalAccessor, [&](fastgltf::math::f32vec3 normal, size_t i) {
				memcpy(&_vbos[i + vbosOffset].normal, &normal, sizeof(glm::f32vec3));			
			}
		);

		//indice
		if (!primitive.indicesAccessor.has_value()) {
			throw std::runtime_error("no indicies accessor value");
		}
		auto& accessor = asset.accessors[primitive.indicesAccessor.value()];
		size_t indicesOffset = _indices.size();
		_indices.resize(_indices.size() + accessor.count);
		fastgltf::iterateAccessorWithIndex<uint16_t>(
			asset, accessor, [&](uint16_t index, size_t i) {
				_indices[i + indicesOffset] = static_cast<uint16_t>(vbosOffset) + index;
			}
		);
		
		//instanceProperty
		InstanceProperty instanceProperty = {
			.materialIndex = static_cast<uint32_t>(primitive.materialIndex.value_or(asset.materials.size())),
		};
		_instanceProperties.push_back(instanceProperty);

		//drawCall
		DrawInfo drawCall = {
			.indexCount = static_cast<uint32_t>(accessor.count),
			.instanceCount = 1,
			.firstIndex = static_cast<uint32_t>(indicesOffset),
			.firstInstance = static_cast<uint32_t>(_drawCalls.size()),
		};
		_meshIndexToDrawInfoMap.insert(std::make_pair(meshIndex, &_drawCalls.emplace_back(drawCall)));
	}
}

void::Engine::addLightData(fastgltf::Asset& asset, glm::f32mat4x4& transform, uint32_t lightIndex) {
	std::cout << "Light " << std::endl;
	Light l;
	glm::f32quat quaterion;
	glm::f32vec3 scale, skew;
	glm::f32vec4 perspective;
	bool success = glm::decompose(transform, scale, quaterion, l.position, skew, perspective);
	if (!success) {
		throw std::runtime_error("could not decompose matrix");
	}

	quaterion = glm::normalize(quaterion);
	l.rotation = glm::eulerAngles(quaterion);

	memcpy(&l.color, &asset.lights[lightIndex].color, sizeof(glm::f32vec3));
	l.type = static_cast<uint32_t>(asset.lights[lightIndex].type);
	memcpy(&l.intensity, &asset.lights[lightIndex].intensity, sizeof(glm::f32) * 4);

	glm::mat4x4 lightView, lightProjection;

	constexpr float forwardAmount = 8.0f;
	const glm::vec3 forward = glm::normalize(glm::vec3(transform[2]));
	const auto eye = glm::vec3(transform[3]);
	const glm::vec3 forwardPosition = eye + (forwardAmount * forward);
	std::cout << "Light " << std::endl;
	lightView = glm::lookAt(eye, forwardPosition, glm::vec3(0.0f, 1.0f, 0.0f));
	lightProjection = glm::perspectiveRH_ZO(l.outerConeAngle, 1.0f, 0.1f, l.range);
	std::cout << glm::to_string(lightProjection) << std::endl;
	l.lightSpaceMatrix = lightProjection * lightView;
	std::cout << glm::to_string(l.lightSpaceMatrix) << std::endl;

	_lights.push_back(l);
}

void Engine::addCameraData(fastgltf::Asset& asset, glm::f32mat4x4& transform, uint32_t cameraIndex) {
//	if (_buffers.camera.GetSize() > 0) {
		//TODO what if there is 0 cameras or more than 1 cameras
//		return;
//	}
  constexpr float forwardAmount = 8.0f;
	const glm::vec3 forward = glm::normalize(glm::vec3(transform[2]));
	const auto eye = glm::vec3(transform[3]);
	const glm::vec3 forwardPosition = eye + (forwardAmount * forward);
	std::cout << "Camera " << std::endl;
	Camera camera;
	camera.view = glm::lookAt(eye, forwardPosition, glm::vec3(0.0f, 1.0f, 0.0f));
//	camera.view = transform; //why does this not work?

	fastgltf::Camera::Perspective* perspectiveCamera = std::get_if<fastgltf::Camera::Perspective>(&asset.cameras[cameraIndex].camera );
	camera.projection = glm::perspectiveRH_ZO(perspectiveCamera->yfov, _surfaceConfiguration.width / (float)_surfaceConfiguration.height, perspectiveCamera->znear, perspectiveCamera->zfar.value_or(1024.0f));
	std::cout << glm::to_string(camera.view) << std::endl;
	std::cout << glm::to_string(transform) << std::endl;
	std::cout << glm::to_string(camera.projection) << std::endl;
	_cameras.push_back(camera);

	fastgltf::Camera::Orthographic* orthographicCamera = std::get_if<fastgltf::Camera::Orthographic>(&asset.cameras[cameraIndex].camera );
	if (orthographicCamera != nullptr) {
		throw std::runtime_error("orthographic camera not supported");
	}
}

void Engine::initSceneBuffers() {
	wgpu::BufferDescriptor cameraBufferDescriptor = {
		.label = "camera buffer",
		.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Uniform,
		.size = sizeof(Camera),
	};
	_buffers.camera = _device.CreateBuffer(&cameraBufferDescriptor);
	_queue.WriteBuffer(_buffers.camera, 0, _cameras.data(), cameraBufferDescriptor.size);

	wgpu::BufferDescriptor lightBufferDescriptor = {
		.label = "light buffer",
		.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Storage,
		.size = sizeof(Light) * _lights.size(),
	};
	_buffers.light = _device.CreateBuffer(&lightBufferDescriptor);
	_queue.WriteBuffer(_buffers.light, 0, _lights.data(), lightBufferDescriptor.size);

	wgpu::BufferDescriptor vboBufferDescriptor = {
			.label = "vbo buffer",
			.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Vertex,
			.size = sizeof(VBO) * _vbos.size(),
	};
	_buffers.vbo = _device.CreateBuffer(&vboBufferDescriptor);
	_queue.WriteBuffer(_buffers.vbo, 0, _vbos.data(), vboBufferDescriptor.size);
	
	wgpu::BufferDescriptor indexBufferDescriptor = {
				.label = "index buffer",
				.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Index,
				.size = sizeof(uint16_t) * _indices.size(),
	};
	_buffers.index = _device.CreateBuffer(&indexBufferDescriptor);
	_queue.WriteBuffer(_buffers.index, 0, _indices.data(), indexBufferDescriptor.size);

	wgpu::BufferDescriptor instancePropertiesBufferDescriptor{
				.label = "instance property buffer",
				.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Storage,
				.size = sizeof(InstanceProperty) * _instanceProperties.size(),
	};
	_buffers.instanceProperties = _device.CreateBuffer(&instancePropertiesBufferDescriptor);
	_queue.WriteBuffer(_buffers.instanceProperties, 0, _instanceProperties.data(), instancePropertiesBufferDescriptor.size);

	wgpu::BufferDescriptor transformBufferDescriptor = {
		.label = "transform buffer",
		.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Storage,
		.size = sizeof(glm::f32mat4x4) * _transforms.size(),
	};
	_buffers.transform = _device.CreateBuffer(&transformBufferDescriptor);
	_queue.WriteBuffer(_buffers.transform, 0, _transforms.data(), transformBufferDescriptor.size);
	
	}

//Default Material will be at end of material buffer
void Engine::initMaterialBuffer(fastgltf::Asset& asset) {
	auto materials = std::vector<Material>(asset.materials.size() + 1);
	
	for (int i = 0; auto & material : asset.materials) {
		memcpy(&materials[i].baseColor, &material.pbrData.baseColorFactor, sizeof(glm::f32vec4));
		i++;
	}

	Material defaultMaterial = {
		.baseColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
	};
	materials[asset.materials.size()] = defaultMaterial;

	wgpu::BufferDescriptor materialBufferDescriptor = {
		.label = "material buffer",
		.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Storage,
		.size = sizeof(Material) * materials.size(),
	};
	_buffers.material = _device.CreateBuffer(&materialBufferDescriptor);
	_queue.WriteBuffer(_buffers.material, 0, materials.data(), materialBufferDescriptor.size);
}

void Engine::initDepthTexture() {
	{
		constexpr uint32_t DEPTH_TEXTURE_RESOLUTION = 8192;
		wgpu::TextureDescriptor depthTextureDescriptor = {
			.label = "shadowmap depth texture",
			.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding,
			.dimension = wgpu::TextureDimension::e2D,
			.size = wgpu::Extent3D(DEPTH_TEXTURE_RESOLUTION, DEPTH_TEXTURE_RESOLUTION),
			.format = DawnEngine::DEPTH_FORMAT,
		};
		wgpu::Texture depthTexture = _device.CreateTexture(&depthTextureDescriptor);
		_textureViews.shadowMaps.push_back(depthTexture.CreateView());
	}
	{
		wgpu::TextureDescriptor depthTextureDescriptor = {
			.label = "camera depth texture",
			.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding,
			.dimension = wgpu::TextureDimension::e2D,
			.size = wgpu::Extent3D(_surfaceConfiguration.width, _surfaceConfiguration.height),
			.format = DawnEngine::DEPTH_FORMAT,
		};
		wgpu::Texture depthTexture = _device.CreateTexture(&depthTextureDescriptor);
		_textureViews.cameraDepth = depthTexture.CreateView();
	}

	wgpu::SamplerDescriptor samplerDescriptor = {
		.label = "depth sampler",
		.compare = wgpu::CompareFunction::Less,
	};

	_samplers.depth =	_device.CreateSampler(&samplerDescriptor);	
}

void Engine::initRenderPipeline() {
	{
		std::vector<uint32_t> vertexShaderCode = Utilities::readShader(std::string("shaders/v_shadowShader.spv"));
		wgpu::ShaderSourceSPIRV vertexShaderSource = wgpu::ShaderSourceSPIRV();
		vertexShaderSource.codeSize = static_cast<uint32_t>(vertexShaderCode.size());
		vertexShaderSource.code = vertexShaderCode.data();
		wgpu::ShaderModuleDescriptor vertexShaderModuleDescriptor = {
			.nextInChain = &vertexShaderSource,
			.label = "vertex shadow shader module"
		};
		wgpu::ShaderModule vertexShaderModule = _device.CreateShaderModule(&vertexShaderModuleDescriptor);

		std::vector<uint32_t> fragmentShaderCode = Utilities::readShader(std::string("shaders/f_shadowShader.spv"));
		wgpu::ShaderSourceSPIRV fragmentShaderSource = wgpu::ShaderSourceSPIRV();
		fragmentShaderSource.codeSize = static_cast<uint32_t>(fragmentShaderCode.size());
		fragmentShaderSource.code = fragmentShaderCode.data();
		wgpu::ShaderModuleDescriptor fragmentShaderModuleDescriptor = {
			.nextInChain = &fragmentShaderSource,
			.label = "fragment shadow shader module"
		};
		wgpu::ShaderModule fragmentShaderModule = _device.CreateShaderModule(&fragmentShaderModuleDescriptor);

		RenderPipelineHelper::RenderPipelineHelperDescriptor renderPipelineDescriptor = {
			.device = _device,
			.buffers = _buffers,
			.textureViews = _textureViews,
			.samplers = _samplers,
			.bindGroups = _bindGroups,
			.vertexShaderModule = vertexShaderModule,
			.fragmentShaderModule = fragmentShaderModule,
			.colorTargetStateFormat = _surfaceConfiguration.format,
		};
		_renderPipelines.shadow = RenderPipelineHelper::createShadowRenderPipeline(renderPipelineDescriptor);
	}

	{
		std::vector<uint32_t> vertexShaderCode = Utilities::readShader(std::string("shaders/v_shader.spv"));
		wgpu::ShaderSourceSPIRV vertexShaderSource = wgpu::ShaderSourceSPIRV();
		vertexShaderSource.codeSize = static_cast<uint32_t>(vertexShaderCode.size());
		vertexShaderSource.code = vertexShaderCode.data();
		wgpu::ShaderModuleDescriptor vertexShaderModuleDescriptor = {
			.nextInChain = &vertexShaderSource,
			.label = "vertex output shader module"
		};
		wgpu::ShaderModule vertexShaderModule = _device.CreateShaderModule(&vertexShaderModuleDescriptor);

		std::vector<uint32_t> fragmentShaderCode = Utilities::readShader(std::string("shaders/f_shader.spv"));
		wgpu::ShaderSourceSPIRV fragmentShaderSource = wgpu::ShaderSourceSPIRV();
		fragmentShaderSource.codeSize = static_cast<uint32_t>(fragmentShaderCode.size());
		fragmentShaderSource.code = fragmentShaderCode.data();
		wgpu::ShaderModuleDescriptor fragmentShaderModuleDescriptor = {
			.nextInChain = &fragmentShaderSource,
			.label = "fragment output shader module"
		};
		wgpu::ShaderModule fragmentShaderModule = _device.CreateShaderModule(&fragmentShaderModuleDescriptor);

		RenderPipelineHelper::RenderPipelineHelperDescriptor renderPipelineDescriptor = {
			.device = _device,
			.buffers = _buffers,
			.textureViews = _textureViews,
			.samplers = _samplers,
			.bindGroups = _bindGroups,
			.vertexShaderModule = vertexShaderModule,
			.fragmentShaderModule = fragmentShaderModule,	
			.colorTargetStateFormat = _surfaceConfiguration.format,
		};
		_renderPipelines.geometry = RenderPipelineHelper::createOutputRenderPipeline(renderPipelineDescriptor);
	}
}


void Engine::draw() {

	//Get next surface texture view
	wgpu::TextureView surfaceTextureView = getNextSurfaceTextureView();

	wgpu::CommandEncoderDescriptor commandEncoderDescriptor = wgpu::CommandEncoderDescriptor();
	commandEncoderDescriptor.label = "My command encoder";
	wgpu::CommandEncoder commandEncoder = _device.CreateCommandEncoder(&commandEncoderDescriptor);
	{ //Shadow Pass
	//	wgpu::RenderPassColorAttachment renderPassColorAttachment = {};
	//	renderPassColorAttachment.view = _textureViews.shadowMaps[0];
	//	renderPassColorAttachment.loadOp = wgpu::LoadOp::Undefined;
	//	renderPassColorAttachment.storeOp = wgpu::StoreOp::Discard;
	//	renderPassColorAttachment.clearValue = wgpu::Color{ 0.3, 0.4, 1.0, 1.0 };

		wgpu::RenderPassDepthStencilAttachment renderPassDepthStencilAttachment = {
			.view = _textureViews.shadowMaps[0],
			.depthLoadOp = wgpu::LoadOp::Clear,
			.depthStoreOp = wgpu::StoreOp::Store,
			.depthClearValue = 1.0f,
		};

		wgpu::RenderPassDescriptor renderPassDescriptor = {
			.label = "shadow render pass",
			//.colorAttachmentCount = 1,
		//	.colorAttachments = &renderPassColorAttachment,
			.depthStencilAttachment = &renderPassDepthStencilAttachment,
		};
		wgpu::RenderPassEncoder renderPassEncoder = commandEncoder.BeginRenderPass(&renderPassDescriptor);
		renderPassEncoder.SetPipeline(_renderPipelines.shadow);
		renderPassEncoder.SetBindGroup(0, _bindGroups.lights);
		renderPassEncoder.SetVertexBuffer(0, _buffers.vbo, 0, _buffers.vbo.GetSize());
		renderPassEncoder.SetIndexBuffer(_buffers.index, wgpu::IndexFormat::Uint16, 0, _buffers.index.GetSize());

		for (auto& dc : _drawCalls) {
			renderPassEncoder.DrawIndexed(dc.indexCount, dc.instanceCount, dc.firstIndex, dc.baseVertex, dc.firstInstance);
		}
		//renderPassEncoder.DrawIndexed(static_cast<uint32_t>(_buffers.index.GetSize()) / sizeof(uint16_t)); //todo
		renderPassEncoder.End();

	}

	{ //Output Pass
		wgpu::RenderPassColorAttachment renderPassColorAttachment = {};
		renderPassColorAttachment.view = surfaceTextureView;
		renderPassColorAttachment.loadOp = wgpu::LoadOp::Clear;
		renderPassColorAttachment.storeOp = wgpu::StoreOp::Store;
		renderPassColorAttachment.clearValue = wgpu::Color{ 0.3, 0.4, 1.0, 1.0 };

		wgpu::RenderPassDepthStencilAttachment renderPassDepthStencilAttachment = {
			.view = _textureViews.cameraDepth,
			.depthLoadOp = wgpu::LoadOp::Clear,
			.depthStoreOp = wgpu::StoreOp::Store,
			.depthClearValue = 1.0f,
		};

		wgpu::RenderPassDescriptor renderPassDescriptor = {
			.label = "output render pass",
			.colorAttachmentCount = 1,
			.colorAttachments = &renderPassColorAttachment,
			.depthStencilAttachment = &renderPassDepthStencilAttachment,
		};


		wgpu::RenderPassEncoder renderPassEncoder = commandEncoder.BeginRenderPass(&renderPassDescriptor);
		renderPassEncoder.SetPipeline(_renderPipelines.geometry);
		renderPassEncoder.SetBindGroup(0, _bindGroups.fixed);
		renderPassEncoder.SetVertexBuffer(0, _buffers.vbo, 0, _buffers.vbo.GetSize());
		renderPassEncoder.SetIndexBuffer(_buffers.index, wgpu::IndexFormat::Uint16, 0, _buffers.index.GetSize());

		for (auto& dc : _drawCalls) {
			renderPassEncoder.DrawIndexed(dc.indexCount, dc.instanceCount, dc.firstIndex, dc.baseVertex, dc.firstInstance);
		}
		//renderPassEncoder.DrawIndexed(static_cast<uint32_t>(_buffers.index.GetSize()) / sizeof(uint16_t)); //todo
		renderPassEncoder.End();
	}
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

wgpu::TextureView Engine::getNextSurfaceTextureView() {
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

	return textureView;
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

void Engine::destroy() {
	//device and gpu object destruction is done by dawn destructor
	_surface.Unconfigure();
	SDL_Quit();
}
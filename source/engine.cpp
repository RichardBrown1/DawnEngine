#pragma once 
#include <cstdlib>
#include <iostream>
#include <format>
#include <chrono>

#include "engine.hpp"
#include <webgpu/webgpu_cpp_print.h>

#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/string_cast.hpp>

#include <fastgltf/tools.hpp>
#include <fastgltf/types.hpp>

#include "constants.hpp"
#include "utilities.hpp"
#include "renderPipelineHelper.hpp"
#include "samplers.hpp"

static Engine* loadedEngine = nullptr;

const uint32_t WIDTH = 1024;
const uint32_t HEIGHT = 720;

Engine::Engine() {
	//Only 1 engine allowed
	assert(loadedEngine == nullptr);
	loadedEngine = this;

	// Initialise SDL
	SDL_Init(SDL_INIT_VIDEO);
	SDL_Window* p_sdl_window = SDL_CreateWindow("DAWN WebGPU Engine", static_cast<int>(WIDTH), static_cast<int>(HEIGHT), 0);

	if (!p_sdl_window) {
		SDL_Quit();
		throw std::runtime_error("SDL_CreateWindow Error: " + std::string(SDL_GetError()));
	}

	constexpr wgpu::InstanceDescriptor instanceDescriptor = {
		.capabilities = {
			.timedWaitAnyEnable = true,
		},
	};
	_instance = wgpu::CreateInstance(&instanceDescriptor);
	if (_instance == nullptr) {
		throw std::runtime_error("Instance creation failed!\n");
	}

	_surface = wgpu::Surface(SDL_GetWGPUSurface(_instance, p_sdl_window));
	if (!_surface) {
		throw std::runtime_error("Failed to get SDL Surface");
	}

	// Synchronously request the adapter.
	const wgpu::RequestAdapterOptions requestAdapterOptions = {
	.powerPreference = wgpu::PowerPreference::HighPerformance
	};
	wgpu::Adapter adapter;

	_instance.WaitAny(_instance.RequestAdapter(
		&requestAdapterOptions, 
		wgpu::CallbackMode::WaitAnyOnly,
		[&](wgpu::RequestAdapterStatus status, 
			 wgpu::Adapter a, 
			 wgpu::StringView message) {
				if (status != wgpu::RequestAdapterStatus::Success) {
					std::cerr << "Failed to get an adapter:" << message.data;
					return;
				}
				adapter = std::move(a);
		}),
	INT64_MAX);
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
	wgpu::Limits limits;
	adapter.GetLimits(&limits);
	std::cout << "Vertex Attribute Limit: " << limits.maxVertexAttributes << std::endl;
	std::cout << "Vertex Buffer Limit: " << limits.maxVertexBuffers << std::endl;
	std::cout << "BindGroup Limit: " << limits.maxBindGroups << std::endl;
	std::cout << "Max Bindings/BindGroup Limit: " << limits.maxBindingsPerBindGroup << std::endl;
	std::cout << "Max Texture Dimension 2D Limit: " << limits.maxTextureDimension2D << std::endl;
	std::cout << "Max Texture Array Layers Limit: " << limits.maxTextureArrayLayers << std::endl;
	std::cout << "Max Compute Workgroups Per Dimension: " << limits.maxComputeWorkgroupsPerDimension << std::endl;
	
	std::array<wgpu::FeatureName, 2> requiredFeatures = { 
		wgpu::FeatureName::IndirectFirstInstance,
		wgpu::FeatureName::TextureCompressionBC,
	};
	//wgpu::FeatureName requiredFeatures = wgpu::FeatureName::IndirectFirstInstance;
	wgpu::DeviceDescriptor deviceDescriptor = {};
	deviceDescriptor.label = "device";
	deviceDescriptor.requiredFeatures = requiredFeatures.data();
	deviceDescriptor.requiredFeatureCount = requiredFeatures.size();
	deviceDescriptor.SetUncapturedErrorCallback(
		[](const wgpu::Device&, wgpu::ErrorType type, wgpu::StringView message) {
			const char* errorTypeName = "";
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
				Utilities::unreachable();
			}
			std::cerr << errorTypeName << " error: " << message;
		}
	);

	deviceDescriptor.SetDeviceLostCallback(
		wgpu::CallbackMode::AllowSpontaneous,
		[](const wgpu::Device&, wgpu::DeviceLostReason reason, wgpu::StringView message) {
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
				Utilities::unreachable();
			}
			std::cerr << "Device lost because of " << reasonName << ": " << message;
		}
	);
	_device = adapter.CreateDevice(&deviceDescriptor);

	_device.SetLoggingCallback(
		[](wgpu::LoggingType type, wgpu::StringView message) {
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
				Utilities::unreachable();
			}
			std::cout << loggingType << message << std::endl;
		});

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
	_gltfParser = fastgltf::Parser::Parser( fastgltf::Extensions::KHR_lights_punctual | fastgltf::Extensions::KHR_texture_basisu);
	auto gltfFile = fastgltf::GltfDataBuffer::FromPath(_gltfDirectory + _gltfFile);
	Utilities::checkFastGltfError(gltfFile.error(), "error loading gltf file");

	auto wholeGltf = _gltfParser.loadGltf(gltfFile.get(), _gltfDirectory, fastgltf::Options::LoadExternalBuffers);
	Utilities::checkFastGltfError(wholeGltf.error(), "error loading gltf directory");

	auto& asset = wholeGltf.get();

	initNodes(asset);
	initSceneBuffers();
	initMaterialBuffer(asset);
}


void Engine::initNodes(fastgltf::Asset& asset) {
	const size_t sceneIndex = asset.defaultScene.value_or(0);
	fastgltf::iterateSceneNodes(asset, sceneIndex, fastgltf::math::fmat4x4(),
		[&](fastgltf::Node& node, fastgltf::math::fmat4x4 m) {
			constexpr glm::f32mat4x4 zMirror = {
				1.0f, 0.0f, 0.0f, 0.0f,
				0.0f, 1.0f, 0.0f, 0.0f,
				0.0f, 0.0f, -1.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f,
			};
			glm::f32mat4x4 matrix = reinterpret_cast<glm::f32mat4x4&>(m);
			matrix *= zMirror;
			
			
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
}

void Engine::addMeshData(fastgltf::Asset& asset, glm::f32mat4x4& transform, uint32_t meshIndex) {
	if (_meshIndexToDrawInfoMap.count(meshIndex)) {
		++_meshIndexToDrawInfoMap[meshIndex]->instanceCount;
		return;
	};

	auto& mesh = asset.meshes[meshIndex];

	for (auto& primitive : mesh.primitives) {
		const size_t vbosOffset = _vbos.size();

		_transforms.push_back(transform);

		//vertice
		fastgltf::Attribute& positionAttribute = *primitive.findAttribute("POSITION");
		fastgltf::Accessor& positionAccessor = asset.accessors[positionAttribute.accessorIndex];
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
		
		//texcoord_0
		fastgltf::Attribute& texcoordAttribute = *primitive.findAttribute("TEXCOORD_0");
		fastgltf::Accessor& texcoordAccessor = asset.accessors[texcoordAttribute.accessorIndex];
		fastgltf::iterateAccessorWithIndex<fastgltf::math::f32vec2>(
			asset, texcoordAccessor, [&](fastgltf::math::f32vec2 texcoord, size_t i) {
				memcpy(&_vbos[i + vbosOffset].texcoord, &texcoord, sizeof(glm::f32vec2));
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
		const DawnEngine::InstanceProperty instanceProperty = {
			.materialIndex = static_cast<uint32_t>(primitive.materialIndex.value_or(asset.materials.size())),
		};
		_instanceProperties.push_back(instanceProperty);

		//drawCall
		const DawnEngine::DrawInfo drawCall = {
			.indexCount = static_cast<uint32_t>(accessor.count),
			.instanceCount = 1,
			.firstIndex = static_cast<uint32_t>(indicesOffset),
			.firstInstance = static_cast<uint32_t>(_drawCalls.size()),
		};
		_meshIndexToDrawInfoMap.insert(std::make_pair(meshIndex, &_drawCalls.emplace_back(drawCall)));
	}
}

void::Engine::addLightData(fastgltf::Asset& asset, glm::f32mat4x4& transform, uint32_t lightIndex) {
	DawnEngine::Light l;
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

	const glm::mat4x4 lightView = glm::inverse(transform);
	const glm::mat4x4 lightProjection = glm::perspectiveRH_ZO(l.outerConeAngle, 1.0f, 0.1f, l.range);
	l.lightSpaceMatrix = lightProjection * lightView;

	_lights.push_back(l);
}

void Engine::addCameraData(fastgltf::Asset& asset, glm::f32mat4x4& transform, uint32_t cameraIndex) {
//	if (_buffers.camera.GetSize() > 0) {
		//TODO what if there is 0 cameras or more than 1 cameras
//		return;
//	}
	const glm::f32mat4x4 view = glm::inverse(transform);
	fastgltf::Camera::Perspective* perspectiveCamera = std::get_if<fastgltf::Camera::Perspective>(&asset.cameras[cameraIndex].camera );
	fastgltf::Camera::Orthographic* orthographicCamera = std::get_if<fastgltf::Camera::Orthographic>(&asset.cameras[cameraIndex].camera );
	if (orthographicCamera != nullptr) {
		throw std::runtime_error("orthographic camera not supported");
	}
	
	DawnEngine::H_Camera h_camera = {
		.projection = glm::perspectiveRH_ZO(perspectiveCamera->yfov, _surfaceConfiguration.width / (float)_surfaceConfiguration.height, perspectiveCamera->znear, perspectiveCamera->zfar.value_or(1024.0f)),
		.position = glm::f32vec3(transform[3]),
		.forward = -glm::normalize(glm::f32vec3(view[2])),
	};

	 _h_cameras.push_back(h_camera);
}


void Engine::initSceneBuffers() {
	if (_h_cameras.size() == 0) {
		_h_cameras.push_back(DawnEngine::getDefaultCamera(_surfaceConfiguration));
	}
	constexpr wgpu::BufferDescriptor cameraBufferDescriptor = {
		.label = "camera buffer",
		.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Uniform,
		.size = sizeof(glm::f32mat4x4),
	};
	_buffers.camera = _device.CreateBuffer(&cameraBufferDescriptor);

	const glm::f32mat4x4 view = glm::lookAt(
		_h_cameras[0].position,
		_h_cameras[0].position + _h_cameras[0].forward,
		DawnEngine::UP
	);
	const glm::f32mat4x4 camera = _h_cameras[0].projection * view;
	_queue.WriteBuffer(_buffers.camera, 0, &camera, cameraBufferDescriptor.size);

	if (_lights.size() == 0) {
		_lights.push_back(DawnEngine::DEFAULT_LIGHT);
	}
	const wgpu::BufferDescriptor lightBufferDescriptor = {
		.label = "light buffer",
		.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Storage,
		.size = sizeof(DawnEngine::Light) * _lights.size(),
	};
	_buffers.light = _device.CreateBuffer(&lightBufferDescriptor);
	_queue.WriteBuffer(_buffers.light, 0, _lights.data(), lightBufferDescriptor.size);

	const wgpu::BufferDescriptor vboBufferDescriptor = {
			.label = "vbo buffer",
			.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Vertex,
			.size = sizeof(DawnEngine::VBO) * _vbos.size(),
	};
	_buffers.vbo = _device.CreateBuffer(&vboBufferDescriptor);
	_queue.WriteBuffer(_buffers.vbo, 0, _vbos.data(), vboBufferDescriptor.size);
	
	const wgpu::BufferDescriptor indexBufferDescriptor = {
				.label = "index buffer",
				.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Index,
				.size = sizeof(uint16_t) * _indices.size(),
	};
	_buffers.index = _device.CreateBuffer(&indexBufferDescriptor);
	_queue.WriteBuffer(_buffers.index, 0, _indices.data(), indexBufferDescriptor.size);

	const wgpu::BufferDescriptor instancePropertiesBufferDescriptor{
				.label = "instance property buffer",
				.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Storage,
				.size = sizeof(DawnEngine::InstanceProperty) * _instanceProperties.size(),
	};
	_buffers.instanceProperties = _device.CreateBuffer(&instancePropertiesBufferDescriptor);
	_queue.WriteBuffer(_buffers.instanceProperties, 0, _instanceProperties.data(), instancePropertiesBufferDescriptor.size);

	const wgpu::BufferDescriptor transformBufferDescriptor = {
		.label = "transform buffer",
		.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Storage,
		.size = sizeof(glm::f32mat4x4) * _transforms.size(),
	};
	_buffers.transform = _device.CreateBuffer(&transformBufferDescriptor);
	_queue.WriteBuffer(_buffers.transform, 0, _transforms.data(), transformBufferDescriptor.size);

}

void Engine::initMaterialBuffer(fastgltf::Asset& asset) {
	auto materials = std::vector<DawnEngine::Material>(asset.materials.size());
	
	for (int i = 0; auto &m : asset.materials) {
		memcpy(&materials[i].pbrMetallicRoughness, &m.pbrData, sizeof(glm::f32vec4) + sizeof(float) * 2);

		if (m.pbrData.baseColorTexture.has_value()) {
			materials[i].textureOptions[DawnEngine::TextureOptionsIndex::HAS_BASE_COLOR_TEXTURE] = 1;
			materials[i].pbrMetallicRoughness.baseColorTextureInfo = DawnEngine::convertType(m.pbrData.baseColorTexture.value());
			
			//DawnEngine::convertType(m.pbrData.metallicRoughnessTexture, materials[i].pbrMetallicRoughness.metallicRoughnessTextureInfo);
		}
		//DawnEngine::convertType(material.pbrData.baseColorTexture, materials[i].pbrMetallicRoughness.baseColorTextureInfo);

		i++;
	}

	const wgpu::BufferDescriptor materialBufferDescriptor = {
		.label = "material buffer",
		.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Storage,
		.size = sizeof(DawnEngine::Material) * materials.size(),
	};
	_buffers.material = _device.CreateBuffer(&materialBufferDescriptor);
	_queue.WriteBuffer(_buffers.material, 0, materials.data(), materialBufferDescriptor.size);
}

void Engine::initDepthTexture() {
	{
		constexpr uint32_t DEPTH_TEXTURE_RESOLUTION = 8192;
		constexpr wgpu::TextureDescriptor depthTextureDescriptor = {
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
		const wgpu::TextureDescriptor depthTextureDescriptor = {
			.label = "camera depth texture",
			.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding,
			.dimension = wgpu::TextureDimension::e2D,
			.size = wgpu::Extent3D(_surfaceConfiguration.width, _surfaceConfiguration.height),
			.format = DawnEngine::DEPTH_FORMAT,
		};
		wgpu::Texture depthTexture = _device.CreateTexture(&depthTextureDescriptor);
		_textureViews.cameraDepth = depthTexture.CreateView();
	}

	constexpr wgpu::SamplerDescriptor samplerDescriptor = {
		.label = "depth sampler",
		.compare = wgpu::CompareFunction::Less,
	};

	_samplers.depth =	_device.CreateSampler(&samplerDescriptor);	
}

void Engine::initRenderPipeline() {
	{
		wgpu::ShaderModule vertexShaderModule =	Utilities::createShaderModule(_device, "vertex shadow shader module", std::string("shaders/v_shadowShader.spv"));
		wgpu::ShaderModule fragmentShaderModule = Utilities::createShaderModule(_device, "fragment shadow shader module", std::string("shaders/f_shadowShader.spv"));

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
		wgpu::ShaderModule vertexShaderModule =	Utilities::createShaderModule(_device, "vertex output shader module", std::string("shaders/v_shader.spv"));
		wgpu::ShaderModule fragmentShaderModule = Utilities::createShaderModule(_device, "fragment output shader module", std::string("shaders/f_shader.spv"));

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

	wgpu::CommandEncoderDescriptor commandEncoderDescriptor = {
		.label = "My command encoder"
	};
	wgpu::CommandEncoder commandEncoder = _device.CreateCommandEncoder(&commandEncoderDescriptor);

	{ //Shadow Pass
		const wgpu::RenderPassDepthStencilAttachment renderPassDepthStencilAttachment = {
			.view = _textureViews.shadowMaps[0],
			.depthLoadOp = wgpu::LoadOp::Clear,
			.depthStoreOp = wgpu::StoreOp::Store,
			.depthClearValue = 1.0f,
		};

		const wgpu::RenderPassDescriptor renderPassDescriptor = {
			.label = "shadow render pass",
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
		renderPassEncoder.End();

	}
	
	{ //Output Pass
		const wgpu::RenderPassColorAttachment renderPassColorAttachment = {
			.view = surfaceTextureView,
			.loadOp = wgpu::LoadOp::Clear,
			.storeOp = wgpu::StoreOp::Store,
			.clearValue = wgpu::Color{ 0.3, 0.4, 1.0, 1.0 },
		};

		const wgpu::RenderPassDepthStencilAttachment renderPassDepthStencilAttachment = {
			.view = _textureViews.cameraDepth,
			.depthLoadOp = wgpu::LoadOp::Clear,
			.depthStoreOp = wgpu::StoreOp::Store,
			.depthClearValue = 1.0f,
		};

		const wgpu::RenderPassDescriptor renderPassDescriptor = {
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

		for (const auto& dc : _drawCalls) {
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

	_device.PopErrorScope(
		wgpu::CallbackMode::AllowSpontaneous,
		[](wgpu::PopErrorScopeStatus status, wgpu::ErrorType, wgpu::StringView message) {
			if (wgpu::PopErrorScopeStatus(status) != wgpu::PopErrorScopeStatus::Success) {
				return;
			}
			std::cerr << std::format("Error: {} \r\n", message.data);
		});

	_surface.Present();

}

wgpu::TextureView Engine::getNextSurfaceTextureView() {
	wgpu::SurfaceTexture surfaceTexture;
	_surface.GetCurrentTexture(&surfaceTexture);
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
	};

	wgpu::TextureView textureView = texture.CreateView(&textureViewDescriptor);

	return textureView;
}

void Engine::handleKeys(const SDL_Event *e) {
	SDL_assert(e->type == SDL_EVENT_KEY_DOWN); /* just checking key presses here... */
	if (e->key.scancode == SDL_SCANCODE_W) {

		//_cameras[0].view = glm::tran
	};
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
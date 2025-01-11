#pragma once 
#include <cstdlib>
#include <iostream>
#include <format>
#include <chrono>

#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

#define SDL_MAIN_HANDLED
#include "../include/sdl3webgpu.hpp"
#include "SDL3/SDL.h"

#include "../include/DawnEngine.hpp"
#include <dawn/webgpu_cpp_print.h>

#include <fastgltf/tools.hpp>
#include <fastgltf/types.hpp>

#include "../include/utilities.hpp"

static DawnEngine* loadedEngine = nullptr;

const uint32_t WIDTH = 1024;
const uint32_t HEIGHT = 720;

const wgpu::TextureFormat DEPTH_FORMAT = wgpu::TextureFormat::Depth16Unorm;

struct Material {
	glm::vec4 baseColor;
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

void DawnEngine::initGltf() {
	_gltfParser = fastgltf::Parser::Parser(fastgltf::Extensions::KHR_lights_punctual);

	auto gltfFile = fastgltf::GltfDataBuffer::FromPath("models/cornellbox.gltf");
	//auto gltfFile = fastgltf::GltfDataBuffer::FromPath("models/cube.gltf");
	Utilities::checkFastGltfError(gltfFile.error(), "cube databuffer fromPath");

	auto wholeGltf = _gltfParser.loadGltf(gltfFile.get(), "models", fastgltf::Options::LoadExternalBuffers);
	Utilities::checkFastGltfError(wholeGltf.error(), "cube loadGltf");

	auto& asset = wholeGltf.get();

	initNodes(asset);
	initMeshBuffers();
	initLightBuffer();
	initMaterialBuffer(asset);
}


void DawnEngine::initNodes(fastgltf::Asset& asset) {
	size_t sceneIndex = asset.defaultScene.value_or(0);
	fastgltf::iterateSceneNodes(asset, sceneIndex, fastgltf::math::fmat4x4(),
		[&](fastgltf::Node& node, fastgltf::math::fmat4x4 matrix) {
			
			if (node.meshIndex.has_value()) {
				addMeshData(asset, Utilities::toGlmFormat(matrix), static_cast<uint32_t>(node.meshIndex.value()));
				return;
			}
			if (node.lightIndex.has_value()) {
				addLightData(asset, Utilities::toGlmFormat(matrix), static_cast<uint32_t>(node.lightIndex.value()));
			}
		});
	auto translations = std::vector<glm::f32mat4x4>(0);

}

void DawnEngine::addMeshData(fastgltf::Asset& asset, glm::f32mat4x4 transform, uint32_t meshIndex) {
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

void::DawnEngine::addLightData(fastgltf::Asset& asset, glm::f32mat4x4 transform, uint32_t lightIndex) {
	Light l;
	l.transform = transform;
	l.type = static_cast<uint32_t>(asset.lights[lightIndex].type);
	memcpy(&l.color, &asset.lights[lightIndex], sizeof(Light) - sizeof(glm::f32mat4x4) - sizeof(glm::u32));
	_lights.push_back(l);
}

void DawnEngine::initMeshBuffers() {
	wgpu::BufferDescriptor uniformBufferDescriptor = {
		.label = "ubo buffer",
		.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Uniform,
		.size = sizeof(UBO),
	};
	_uniformBuffer = _device.CreateBuffer(&uniformBufferDescriptor);


	wgpu::BufferDescriptor vboBufferDescriptor = {
			.label = "vbo buffer",
			.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Vertex,
			.size = sizeof(VBO) * _vbos.size(),
	};
	_vboBuffer = _device.CreateBuffer(&vboBufferDescriptor);
	_queue.WriteBuffer(_vboBuffer, 0, _vbos.data(), vboBufferDescriptor.size);
	
	wgpu::BufferDescriptor indexBufferDescriptor = {
				.label = "index buffer",
				.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Index,
				.size = sizeof(uint16_t) * _indices.size(),
	};
	_indexBuffer = _device.CreateBuffer(&indexBufferDescriptor);
	_queue.WriteBuffer(_indexBuffer, 0, _indices.data(), indexBufferDescriptor.size);

	wgpu::BufferDescriptor instancePropertiesBufferDescriptor{
				.label = "instance property buffer",
				.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Storage,
				.size = sizeof(InstanceProperty) * _instanceProperties.size(),
	};
	_instancePropertiesBuffer = _device.CreateBuffer(&instancePropertiesBufferDescriptor);
	_queue.WriteBuffer(_instancePropertiesBuffer, 0, _instanceProperties.data(), instancePropertiesBufferDescriptor.size);

	wgpu::BufferDescriptor transformBufferDescriptor = {
		.label = "transform buffer",
		.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Storage,
		.size = sizeof(glm::f32mat4x4) * _transforms.size(),
	};
	_transformBuffer = _device.CreateBuffer(&transformBufferDescriptor);
	_queue.WriteBuffer(_transformBuffer, 0, _transforms.data(), transformBufferDescriptor.size);
}

void DawnEngine::initLightBuffer() {	
	wgpu::BufferDescriptor lightBufferDescriptor = {
		.label = "light buffer",
		.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Storage,
		.size = sizeof(Light) * _lights.size(),
	};
	_lightBuffer = _device.CreateBuffer(&lightBufferDescriptor);
	_queue.WriteBuffer(_lightBuffer, 0, _lights.data(), lightBufferDescriptor.size);
}

//Default Material will be at end of material buffer
void DawnEngine::initMaterialBuffer(fastgltf::Asset& asset) {
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
		.label = "ubo buffer",
		.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Storage,
		.size = sizeof(Material) * materials.size(),
	};
	_materialBuffer = _device.CreateBuffer(&materialBufferDescriptor);
	_queue.WriteBuffer(_materialBuffer, 0, materials.data(), materialBufferDescriptor.size);
}

void DawnEngine::initDepthTexture() {

	wgpu::TextureDescriptor depthTextureDescriptor = {
		.label = "depth texture",
		.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding,
		.dimension = wgpu::TextureDimension::e2D,
		.size = wgpu::Extent3D(_surfaceConfiguration.width, _surfaceConfiguration.height),
		.format = DEPTH_FORMAT,
	};
	wgpu::Texture depthTexture = _device.CreateTexture(&depthTextureDescriptor);

	_depthTextureView = depthTexture.CreateView();
	
	wgpu::SamplerDescriptor samplerDescriptor = {
		.label = "depth sampler",
		.compare = wgpu::CompareFunction::Less,
	};

	_depthSampler =	_device.CreateSampler(&samplerDescriptor);	
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
	wgpu::VertexAttribute normalAttribute = {
		.format = wgpu::VertexFormat::Float32x3,
		.offset = offsetof(VBO, normal),
		.shaderLocation = 1,
	};


	auto vertexAttributes = std::vector<wgpu::VertexAttribute> { positionAttribute, normalAttribute };

	wgpu::VertexBufferLayout vertexBufferLayout = {
		.arrayStride = sizeof(VBO),
		.attributeCount = vertexAttributes.size(),
		.attributes = vertexAttributes.data(),
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
		
	wgpu::DepthStencilState depthStencilState = {
		.format = DEPTH_FORMAT,
		.depthWriteEnabled = true,
		.depthCompare = wgpu::CompareFunction::Less,
	};

	wgpu::RenderPipelineDescriptor renderPipelineDescriptor = {
		.label = "render pipeline descriptor",
		.layout = initPipelineLayout(),
		.vertex = vertexState,
		.primitive = wgpu::PrimitiveState {
			.topology = wgpu::PrimitiveTopology::TriangleList,
			.cullMode = wgpu::CullMode::None,
		},
		.depthStencil = &depthStencilState,
		.multisample = wgpu::MultisampleState {
			.count = 1,
			.mask = ~0u,
			.alphaToCoverageEnabled = false,
		},
		.fragment = &fragmentState,
	};

	_renderPipeline = _device.CreateRenderPipeline(&renderPipelineDescriptor);
}

wgpu::PipelineLayout DawnEngine::initPipelineLayout() {
	
	std::array<wgpu::BindGroupLayout, 2> bindGroupLayouts = { initStaticBindGroupLayout(), initInfrequentBindGroupLayout() };
	wgpu::PipelineLayoutDescriptor pipelineLayoutDescriptor = {
		.label = "Pipeline Layout",
		.bindGroupLayoutCount = 2,
		.bindGroupLayouts = bindGroupLayouts.data(),
	};
	return _device.CreatePipelineLayout(&pipelineLayoutDescriptor);
}

wgpu::BindGroupLayout DawnEngine::initStaticBindGroupLayout() {
	wgpu::BindGroupLayoutEntry uboBindGroupLayoutEntry = {
		.binding = 0,
		.visibility = (wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment),
		.buffer = {
			.type = wgpu::BufferBindingType::Uniform,
			.minBindingSize = sizeof(UBO),
		}
	};
	wgpu::BindGroupLayoutEntry transformsBindGroupLayoutEntry = {
		.binding = 1,
		.visibility = wgpu::ShaderStage::Vertex,
		.buffer = {
			.type = wgpu::BufferBindingType::ReadOnlyStorage,
			.minBindingSize = sizeof(glm::f32mat4x4),
		}
	};
	wgpu::BindGroupLayoutEntry instancePropertiesBindGroupLayoutEntry = {
		.binding = 2,
		.visibility = wgpu::ShaderStage::Vertex,
		.buffer = {
			.type = wgpu::BufferBindingType::ReadOnlyStorage,
			.minBindingSize = sizeof(InstanceProperty),
		}
	};
	std::array<wgpu::BindGroupLayoutEntry, 3> bindGroupLayoutEntries = { 
		uboBindGroupLayoutEntry, 
		transformsBindGroupLayoutEntry,
		instancePropertiesBindGroupLayoutEntry,
	};

	wgpu::BindGroupLayoutDescriptor bindGroupLayoutDescriptor = {
		.label = "static bind group",
		.entryCount = bindGroupLayoutEntries.size(),
		.entries = bindGroupLayoutEntries.data(),
	};
	wgpu::BindGroupLayout bindGroupLayout = _device.CreateBindGroupLayout(&bindGroupLayoutDescriptor);


	wgpu::BindGroupEntry uboBindGroupEntry = {
		.binding = 0,
		.buffer = _uniformBuffer,
		.size = _uniformBuffer.GetSize(),
	};
	wgpu::BindGroupEntry transformsBindGroupEntry = {
		.binding = 1,
		.buffer = _transformBuffer,
		.size = _transformBuffer.GetSize(),
	};
	wgpu::BindGroupEntry instancePropertiesBindGroupEntry = {
		.binding = 2,
		.buffer = _instancePropertiesBuffer,
		.size = _instancePropertiesBuffer.GetSize(),
	};
	std::array<wgpu::BindGroupEntry, 3> bindGroupEntries = { 
		uboBindGroupEntry,
		transformsBindGroupEntry,
		instancePropertiesBindGroupEntry,
	};

	wgpu::BindGroupDescriptor bindGroupDescriptor = {
		.label = "static bind group",
		.layout = bindGroupLayout,
		.entryCount = bindGroupEntries.size(),
		.entries = bindGroupEntries.data() ,
	};
	_bindGroups.push_back(_device.CreateBindGroup(&bindGroupDescriptor));

	return bindGroupLayout;
}

wgpu::BindGroupLayout DawnEngine::initInfrequentBindGroupLayout() {
	wgpu::BindGroupLayoutEntry materialBindGroupLayoutEntry = {
		.binding = 0,
		.visibility = wgpu::ShaderStage::Vertex,
		.buffer = {
			.type = wgpu::BufferBindingType::ReadOnlyStorage,
			.minBindingSize = sizeof(Material),
		}
	};
	wgpu::BindGroupLayoutEntry lightBindGroupLayoutEntry = {
		.binding = 1,
		.visibility = wgpu::ShaderStage::Fragment,
		.buffer = {
			.type = wgpu::BufferBindingType::ReadOnlyStorage,
			.minBindingSize = sizeof(Light),
		}
	};
	std::array<wgpu::BindGroupLayoutEntry, 2> bindGroupLayoutEntries = {
		materialBindGroupLayoutEntry,
		lightBindGroupLayoutEntry,
	};

	wgpu::BindGroupLayoutDescriptor bindGroupLayoutDescriptor = {
		.label = "infrequent bind group",
		.entryCount = bindGroupLayoutEntries.size(),
		.entries = bindGroupLayoutEntries.data(),
	};
	wgpu::BindGroupLayout bindGroupLayout = _device.CreateBindGroupLayout(&bindGroupLayoutDescriptor);

	wgpu::BindGroupEntry materialBindGroupEntry = {
		.binding = 0,
		.buffer = _materialBuffer,
		.size = _materialBuffer.GetSize(),
	};
	wgpu::BindGroupEntry lightBindGroupEntry = {
		.binding = 1,
		.buffer = _lightBuffer,
		.size = _lightBuffer.GetSize()
	};
	std::array<wgpu::BindGroupEntry, 2> bindGroupEntries = {
		materialBindGroupEntry,
		lightBindGroupEntry,
	};

	wgpu::BindGroupDescriptor bindGroupDescriptor = {
		.label = "infrequent bind group",
		.layout = bindGroupLayout,
		.entryCount = bindGroupLayoutDescriptor.entryCount,
		.entries = bindGroupEntries.data(),
	};
	_bindGroups.push_back(_device.CreateBindGroup(&bindGroupDescriptor));

	return bindGroupLayout;
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
	renderPassColorAttachment.clearValue = wgpu::Color{ 0.3, 0.4, 1.0, 1.0 };

	wgpu::RenderPassDepthStencilAttachment renderPassDepthStencilAttachment = {
		.view = _depthTextureView,
		.depthLoadOp = wgpu::LoadOp::Clear,
		.depthStoreOp = wgpu::StoreOp::Store,
		.depthClearValue = 1.0f,		
	};

	wgpu::RenderPassDescriptor renderPassDescriptor = {
		.colorAttachmentCount = 1,
		.colorAttachments = &renderPassColorAttachment,
		.depthStencilAttachment = &renderPassDepthStencilAttachment,
	};


	wgpu::RenderPassEncoder renderPassEncoder = commandEncoder.BeginRenderPass(&renderPassDescriptor);
	renderPassEncoder.SetPipeline(_renderPipeline);
	renderPassEncoder.SetBindGroup(0, _bindGroups[0]); //static buffer
	renderPassEncoder.SetBindGroup(1, _bindGroups[1]); //infrequent buffer
	renderPassEncoder.SetVertexBuffer(0, _vboBuffer, 0, _vboBuffer.GetSize());
	renderPassEncoder.SetIndexBuffer(_indexBuffer, wgpu::IndexFormat::Uint16, 0, _indexBuffer.GetSize());

	for (auto& dc : _drawCalls) {
		renderPassEncoder.DrawIndexed(dc.indexCount, dc.instanceCount, dc.firstIndex, dc.baseVertex, dc.firstInstance);
	}
	//renderPassEncoder.DrawIndexed(static_cast<uint32_t>(_indexBuffer.GetSize()) / sizeof(uint16_t)); //todo
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
	//static auto startTime = std::chrono::high_resolution_clock::now();

	//auto currentTime = std::chrono::high_resolution_clock::now();
	//float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
	
	UBO ubo{};
	ubo.model = glm::mat4x4(1.0f);
	ubo.view = glm::lookAt(glm::vec3(0.0f, 4.0f, -0.1f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
	ubo.projection = glm::perspective(glm::radians(45.0f), _surfaceConfiguration.width / (float)_surfaceConfiguration.height, 0.1f, 1500.0f);
	ubo.inversedTransposedModel = glm::transpose(glm::inverse(ubo.model));
	//ubo.projection[1][1] *= -1; //y coordinate is inverted on OpenGL - flip it

	_queue.WriteBuffer(_uniformBuffer, 0, &ubo, sizeof(UBO));

	//Debug
	Light light = _lights[0];
	float lightConstant = 1.0f;
	float lightLinear = 0.5f;
	float lightQuadratic = 0.1f;
	//glm::vec3 lightPosition = glm::vec3(light.transform[0][3], light.transform[1][3], light.transform[2][3]);
	glm::vec3 point = glm::vec3(471.90326,  665.84003,  1359.18103);
	glm::vec3 normal = glm::vec3(0.0, 0.0, 1.0);
	glm::vec3 lightPosition = glm::vec3(light.transform[3][0], light.transform[3][1], light.transform[3][2]);
//	glm::vec3 lightDirection = glm::normalize(lightPosition - point);
	glm::vec3 lightDirection = glm::normalize(point - lightPosition);
	float diff = glm::max(glm::dot(normal, lightDirection), 0.0f);
	float distance = glm::length(lightPosition - point);
	float attenuation = 1.0f / (lightConstant + lightLinear * distance + lightQuadratic * (distance * distance));
	diff *= attenuation;
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
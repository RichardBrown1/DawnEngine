#pragma once 
#include <cstdlib>
#include <iostream>
#include <format>
#include <chrono>

#include "engine.hpp"
#include <dawn/webgpu_cpp_print.h>

#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/string_cast.hpp>

#include <fastgltf/tools.hpp>
#include <fastgltf/types.hpp>

#include "constants.hpp"
#include "utilities.hpp"
#include "renderPipelineHelper.hpp"

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
		.features = {
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
	const wgpu::RequestAdapterOptions requestAdapterOptions = {};
	wgpu::Adapter adapter;
	const wgpu::RequestAdapterCallbackInfo callbackInfo = {
		.nextInChain = nullptr,
		.mode = wgpu::CallbackMode::WaitAnyOnly,
		.callback =
			[](WGPURequestAdapterStatus status, WGPUAdapter adapter,
				WGPUStringView message, void* userdata) {
					if (status != WGPURequestAdapterStatus_Success) {
						std::cerr << "Failed to get an adapter:" << message.data;
						return;
					}
				*static_cast<wgpu::Adapter*>(userdata) = wgpu::Adapter::Acquire(adapter);
		},
		.userdata = &adapter,
	};

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
	std::cout << "Max Texture Dimension 2D Limit: " << supportedLimits.limits.maxTextureDimension2D << std::endl;
	std::cout << "Max Texture Array Layers Limit: " << supportedLimits.limits.maxTextureArrayLayers << std::endl;
	
	std::array<wgpu::FeatureName, 2> requiredFeatures = { 
		wgpu::FeatureName::IndirectFirstInstance,
		wgpu::FeatureName::TextureCompressionBC,
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
			exit(2);
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
	_gltfParser = fastgltf::Parser::Parser( fastgltf::Extensions::KHR_lights_punctual | fastgltf::Extensions::KHR_texture_basisu);
	auto gltfFile = fastgltf::GltfDataBuffer::FromPath(_gltfDirectory + _gltfFile);
	Utilities::checkFastGltfError(gltfFile.error(), "error loading gltf file");

	auto wholeGltf = _gltfParser.loadGltf(gltfFile.get(), _gltfDirectory, fastgltf::Options::LoadExternalBuffers);
	Utilities::checkFastGltfError(wholeGltf.error(), "error loading gltf directory");

	auto& asset = wholeGltf.get();

	initNodes(asset);
	initSceneBuffers();
	initMaterialBuffer(asset);
	initTextures(asset);
	initSamplerTexturePairs(asset);
	initSamplers(asset);
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

void Engine::initTextures(fastgltf::Asset& asset) {
	//Texture Management Architecture - Prototype
	//1. 2D Textures will be in layered Textures with an ArrayIndex
	// Can I use Texture2DArray in HLSL?
	//	a. There may have to be different layered textures depending on amount of channels
	//2. SamplerTexturePair will also have an indicator of Texture Size used to set bounds when sampling
	//	a. 

	std::vector<std::array<std::array<uint32_t, 2048>, 2048 >> hostTextures;
	hostTextures.resize(asset.images.size());

	for (int i = 0; i < asset.images.size(); i++) {
		//	asset.images.
		fastgltf::DataSource ds = asset.images[i].data;
		DawnEngine::getTexture(_device, ds, _gltfDirectory, hostTextures[i]);
	}

	//	wgpu::TextureDimension textureDimension = [&sp_ktxTexture2]() {
	//			switch (sp_ktxTexture2->numDimensions) {
	//			case 1:
	//				return wgpu::TextureDimension::e1D;
	//			case 2:
	//				return wgpu::TextureDimension::e2D;
	//			case 3:
	//				return wgpu::TextureDimension::e3D;
	//			default:
	//				throw std::runtime_error("unknown Texture Dimension");
	//			}
	//		}();

	wgpu::TextureDescriptor textureDescriptor = {
		.label = "2048", //wgpu::StringView(filePath),
		.usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopyDst,
		.dimension = wgpu::TextureDimension::e2D,
		.size = wgpu::Extent3D {
			.width = 2048,
			.height = 2048,
			.depthOrArrayLayers = static_cast<uint32_t>(hostTextures.size())
		},
		.format = wgpu::TextureFormat::BC7RGBAUnormSrgb,
		.mipLevelCount = 1,
	};
	wgpu::Texture texture = _device.CreateTexture(&textureDescriptor);
	const wgpu::ImageCopyTexture imageCopyTexture = {
		.texture = texture,
		.mipLevel = 0,
	};
	const wgpu::TextureDataLayout textureDataLayout = {
			.bytesPerRow = textureDescriptor.size.width * 4, //1 pixel = 4 bytes. Careful this will change with different formats
			.rowsPerImage = textureDescriptor.size.height,
	};

	//const wgpu::Extent3D dataLayoutSize = {
	//	.width = textureDataLayout.bytesPerRow,
	//	.height = textureDataLayout.rowsPerImage,
	//};

	_queue.WriteTexture(
		&imageCopyTexture,
		hostTextures.data(),
		hostTextures.size() * sizeof(uint32_t) * 2048 * 2048,
		&textureDataLayout,
		&textureDescriptor.size
	);

	const wgpu::TextureViewDescriptor textureViewDescriptor = {
		.label = "Textures",
		.format = textureDescriptor.format,
		.dimension = wgpu::TextureViewDimension::e2DArray,
		.mipLevelCount = textureDescriptor.mipLevelCount,
		.arrayLayerCount = textureDescriptor.size.depthOrArrayLayers,
		.usage = textureDescriptor.usage,
	};
	_textureViews.textures = texture.CreateView(&textureViewDescriptor);

}

void Engine::initSamplerTexturePairs(fastgltf::Asset& asset) {
	std::vector<DawnEngine::SamplerTexturePair> samplerTexturePairs;
	for (const auto& t : asset.textures) {
		if (!t.basisuImageIndex.has_value()) {
			throw std::runtime_error("No Basisu Image Texture found");
		}
		if (t.imageIndex.has_value()) {
			throw std::runtime_error("Found Normal Image Texture Index. - Unsupported in this application");
		}

		const DawnEngine::SamplerTexturePair texture = {
			.samplerIndex = static_cast<uint32_t>(t.samplerIndex.value()),
			.textureIndex = static_cast<uint32_t>(t.basisuImageIndex.value()),
		};
		samplerTexturePairs.push_back(texture);
	}

	const wgpu::BufferDescriptor samplerTextureBufferDescriptor = {
		.label = "sampler texture pair buffer",
		.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Storage,
		.size = sizeof(DawnEngine::SamplerTexturePair) * samplerTexturePairs.size(),
	};
	_buffers.samplerTexturePair = _device.CreateBuffer(&samplerTextureBufferDescriptor);
	_queue.WriteBuffer(
		_buffers.samplerTexturePair,
		0, 
		samplerTexturePairs.data(), 
		samplerTextureBufferDescriptor.size
	);
}

void Engine::initSamplers(fastgltf::Asset& asset) {
	auto convertAddressMode = [](fastgltf::Wrap wrap) {
		switch (wrap) {
			case fastgltf::Wrap::ClampToEdge:
				return wgpu::AddressMode::ClampToEdge;
			case fastgltf::Wrap::MirroredRepeat:
				return wgpu::AddressMode::MirrorRepeat;
			case fastgltf::Wrap::Repeat:
				return wgpu::AddressMode::Repeat;
			default:
				throw std::runtime_error("Unknown wrap type in AddressMode conversion");
			}
		};

	auto convertFilter = [](fastgltf::Optional<fastgltf::Filter> filter) {
		if (!filter.has_value()) {
			return wgpu::FilterMode::Nearest; //default value in wgpu::SamplerDescriptor
		}
		switch (filter.value()) {
			case fastgltf::Filter::Linear:
			case fastgltf::Filter::LinearMipMapLinear:
			case fastgltf::Filter::LinearMipMapNearest:
				return wgpu::FilterMode::Linear;
			case fastgltf::Filter::Nearest:
			case fastgltf::Filter::NearestMipMapLinear:
			case fastgltf::Filter::NearestMipMapNearest:
				return wgpu::FilterMode::Nearest;
			default:
				throw std::runtime_error("Unknown Filter value");
		}
	};

	auto convertMipMapFilter = [](fastgltf::Optional<fastgltf::Filter> filter) {
		if (!filter.has_value()) {
			return wgpu::MipmapFilterMode::Nearest; //default value in wgpu::SamplerDescriptor
		}
		switch (filter.value()) {
			case fastgltf::Filter::Linear:
			case fastgltf::Filter::Nearest:
				return wgpu::MipmapFilterMode::Undefined;
			case fastgltf::Filter::LinearMipMapLinear:
			case fastgltf::Filter::NearestMipMapLinear:
				return wgpu::MipmapFilterMode::Linear;
			case fastgltf::Filter::LinearMipMapNearest:
			case fastgltf::Filter::NearestMipMapNearest:
				return wgpu::MipmapFilterMode::Nearest;
			default:
				throw std::runtime_error("Unknown MipMap Filter value");
		}
	};

	for (const auto& s : asset.samplers) {
		const	wgpu::SamplerDescriptor samplerDescriptor = {
			.label = s.name.c_str(),
			.addressModeU = convertAddressMode(s.wrapS),
			.addressModeV = convertAddressMode(s.wrapT),
			.magFilter = convertFilter(s.magFilter),
			.minFilter = convertFilter(s.minFilter),
			.mipmapFilter = convertMipMapFilter(s.minFilter),
		};
		_samplers.texture = _device.CreateSampler(&samplerDescriptor);
	}
}

void Engine::initMaterialBuffer(fastgltf::Asset& asset) {
	auto materials = std::vector<DawnEngine::Material>(asset.materials.size());
	
	for (int i = 0; auto &m : asset.materials) {
		memcpy(&materials[i].pbrMetallicRoughness, &m.pbrData, sizeof(glm::f32vec4) + sizeof(float) * 2);
		if (m.pbrData.baseColorTexture.has_value()) {
			materials[i].textureOptions[DawnEngine::TEXTURE_OPTIONS_INDEX::hasBaseColorTexture] = 1;
			DawnEngine::convertType(m.pbrData.baseColorTexture, materials[i].pbrMetallicRoughness.baseColorTextureInfo);
			//DawnEngine::convertType(m.pbrData.metallicRoughnessTexture, materials[i].pbrMetallicRoughness.metallicRoughnessTextureInfo);
		}
		//DawnEngine::convertType(material.pbrData.baseColorTexture, materials[i].pbrMetallicRoughness.baseColorTextureInfo);

		i++;
	}
//	materials.add(DawnEngine::DE);

//	constexpr DawnEngine::Material defaultMaterial = {
//		.baseColor = {1.0f, 1.0f, 1.0f, 1.0f},
//	};
	//materials[asset.materials.size()] = defaultMaterial;

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
		wgpu::ShaderModule vertexShaderModule =	DawnEngine::createShaderModule(_device, "vertex shadow shader module", std::string("shaders/v_shadowShader.spv"));
		wgpu::ShaderModule fragmentShaderModule = DawnEngine::createShaderModule(_device, "fragment shadow shader module", std::string("shaders/f_shadowShader.spv"));

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
		wgpu::ShaderModule vertexShaderModule =	DawnEngine::createShaderModule(_device, "vertex output shader module", std::string("shaders/v_shader.spv"));
		wgpu::ShaderModule fragmentShaderModule = DawnEngine::createShaderModule(_device, "fragment output shader module", std::string("shaders/f_shader.spv"));

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
#pragma once
#include "../device/device.hpp"
#include "../wgpuContext/wgpuContext.hpp"
#include "../render/initial.hpp"
#include "../render/shadowToCamera.hpp"
#include "../render/shadowMap.hpp"
#include "../render/accumulator/fourChannel.hpp"
#include "../render/ultimate.hpp"
#include "../render/toSurface.hpp"
#include "../render/lighting.hpp"
#include "../render/clear.hpp"
#include "../device/resources.hpp"

class Engine {

public:
	Engine();
	~Engine();
	void run();

private:
	const std::string gltfDirectory = "models/monoBox/"; //must end with "/"
	const std::string gltfFileName = "cornellbox.gltf";
//	const std::string gltfDirectory = "models/avocado/"; // must end with "/"
// const std::string gltfFileName = "avocado2.gltf";
//	const std::string gltfDirectory = "models/boombox/"; //must end with "/"
//	const std::string gltfFileName = "BoomBoxWithAxes.gltf";

	WGPUContext _wgpuContext;
	DeviceResources* _deviceResources;
	render::Initial* _initialRender;
	render::ShadowMap* _shadowMapRender;
	render::ShadowToCamera* _shadowToCamera;
	render::FourChannel* _baseColorAccumulatorRender;
	render::FourChannel* _normalAccumulatorRender;
	render::Lighting* _lightingRender;
	render::Ultimate* _ultimateRender;
	render::ToSurface* _toSurfaceRender;
	render::Clear* _clearLightingRender;
	std::vector<structs::host::DrawCall> _drawCalls;

	void draw();
};

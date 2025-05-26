#pragma once
#include "../device/device.hpp"
#include "../wgpuContext/wgpuContext.hpp"
#include "../render/initial.hpp"
#include "../render/shadow.hpp"
#include "../render/ultimate.hpp"
#include "../render/toSurface.hpp"

class Engine {
public:
	Engine();
	void run();
private:
	WGPUContext _wgpuContext;
	device::SceneResources _deviceSceneResources;
	render::Initial* _initialRender;
	render::Shadow* _shadowRender;
	render::Ultimate* _ultimateRender;
	render::ToSurface* _toSurfaceRender;
	std::vector<structs::host::DrawCall> _drawCalls;

	void draw();
	void destroy();
};

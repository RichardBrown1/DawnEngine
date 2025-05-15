#pragma once
#include "../device/device.hpp"
#include "../wgpuContext/wgpuContext.hpp"
#include "../render/initialRender.hpp"

class Engine {
public:
	Engine();
	void run();
private:
	WGPUContext _wgpuContext;
	device::SceneResources _deviceSceneResources;
	render::Initial* _initialRender;
	std::vector<structs::host::DrawCall> _drawCalls;

	void draw();
	void destroy();
};

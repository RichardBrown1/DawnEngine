#include "sdl3webgpu.hpp"

#include <webgpu/webgpu.h>

#if defined(SDL_PLATFORM_MACOS)
#  include <Cocoa/Cocoa.h>
#  include <Foundation/Foundation.h>
#  include <QuartzCore/CAMetalLayer.h>
#elif defined(SDL_PLATFORM_IOS)
#  include <UIKit/UIKit.h>
#  include <Foundation/Foundation.h>
#  include <QuartzCore/CAMetalLayer.h>
#  include <Metal/Metal.h>
#elif defined(SDL_PLATFORM_WIN32)
#  include <windows.h>
#endif

#include <SDL3/SDL.h>

wgpu::Surface SDL_GetWGPUSurface(wgpu::Instance instance, SDL_Window* window) {
    SDL_PropertiesID props = SDL_GetWindowProperties(window);

#if defined(SDL_PLATFORM_MACOS)
    {
        id metal_layer = NULL;
        NSWindow *ns_window = (__bridge NSWindow *)SDL_GetPointerProperty(props, SDL_PROP_WINDOW_COCOA_WINDOW_POINTER, NULL);
        if (!ns_window) return NULL;
        [ns_window.contentView setWantsLayer : YES];
        metal_layer = [CAMetalLayer layer];
        [ns_window.contentView setLayer : metal_layer];

        WGPUSurfaceSourceMetalLayer fromMetalLayer;
        fromMetalLayer.chain.sType = WGPUSType_SurfaceSourceMetalLayer;
        fromMetalLayer.chain.next = NULL;
        fromMetalLayer.layer = metal_layer;

        WGPUSurfaceDescriptor surfaceDescriptor;
        surfaceDescriptor.nextInChain = &fromMetalLayer.chain;
        surfaceDescriptor.label = (WGPUStringView){ NULL, WGPU_STRLEN };

        return wgpuInstanceCreateSurface(instance, &surfaceDescriptor);
    }
#elif defined(SDL_PLATFORM_IOS)
    {
        UIWindow *ui_window = (__bridge UIWindow *)SDL_GetPointerProperty(props, SDL_PROP_WINDOW_UIKIT_WINDOW_POINTER, NULL);
        if (!uiwindow) return NULL;

        UIView* ui_view = ui_window.rootViewController.view;
        CAMetalLayer* metal_layer = [CAMetalLayer new];
        metal_layer.opaque = true;
        metal_layer.frame = ui_view.frame;
        metal_layer.drawableSize = ui_view.frame.size;

        [ui_view.layer addSublayer: metal_layer];

        WGPUSurfaceSourceMetalLayer fromMetalLayer;
        fromMetalLayer.chain.sType = WGPUSType_SurfaceSourceMetalLayer;
        fromMetalLayer.chain.next = NULL;
        fromMetalLayer.layer = metal_layer;

        WGPUSurfaceDescriptor surfaceDescriptor;
        surfaceDescriptor.nextInChain = &fromMetalLayer.chain;
        surfaceDescriptor.label = (WGPUStringView){ NULL, WGPU_STRLEN };

        return wgpuInstanceCreateSurface(instance, &surfaceDescriptor);
    }
#elif defined(SDL_PLATFORM_LINUX)
    if (SDL_strcmp(SDL_GetCurrentVideoDriver(), "x11") == 0) {
        void *x11_display = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_X11_DISPLAY_POINTER, NULL);
        uint64_t x11_window = SDL_GetNumberProperty(props, SDL_PROP_WINDOW_X11_WINDOW_NUMBER, 0);
        if (!x11_display || !x11_window) return wgpu::Surface();

        wgpu::SurfaceSourceXlibWindow fromXlibWindow;
        fromXlibWindow.display = x11_display;
        fromXlibWindow.window = x11_window;

        wgpu::SurfaceDescriptor surfaceDescriptor;
        surfaceDescriptor.nextInChain = &fromXlibWindow;
        surfaceDescriptor.label = wgpu::StringView("x11 surface");

        return instance.CreateSurface(&surfaceDescriptor);
    }
    else if (SDL_strcmp(SDL_GetCurrentVideoDriver(), "wayland") == 0) {
        void *wayland_display = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WAYLAND_DISPLAY_POINTER, NULL);
        void *wayland_surface = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WAYLAND_SURFACE_POINTER, NULL);
        if (!wayland_display || !wayland_surface) return wgpu::Surface();

        constexpr wgpu::ChainedStruct chainedStruct = {
            .sType = wgpu::SType::SurfaceSourceWaylandSurface,
        };
        wgpu::SurfaceSourceWaylandSurface fromWaylandSurface;
        fromWaylandSurface.display = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WAYLAND_DISPLAY_POINTER, NULL);
        fromWaylandSurface.surface = wayland_surface;

        wgpu::SurfaceDescriptor surfaceDescriptor;
        surfaceDescriptor.nextInChain = &chainedStruct;
        surfaceDescriptor.label = wgpu::StringView("wayland surface");

        return instance.CreateSurface(&surfaceDescriptor);
    }
#elif defined(SDL_PLATFORM_WIN32)
    {
        HWND hwnd = (HWND)SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);
        if (!hwnd) return NULL;
        HINSTANCE hinstance = GetModuleHandle(NULL);

        WGPUSurfaceSourceWindowsHWND fromWindowsHWND;
        fromWindowsHWND.chain.sType = WGPUSType_SurfaceSourceWindowsHWND;
        fromWindowsHWND.chain.next = NULL;
        fromWindowsHWND.hinstance = hinstance;
        fromWindowsHWND.hwnd = hwnd;

        WGPUSurfaceDescriptor surfaceDescriptor;
        surfaceDescriptor.nextInChain = &fromWindowsHWND.chain;
        surfaceDescriptor.label = (WGPUStringView){ NULL, WGPU_STRLEN };

        return wgpuInstanceCreateSurface(instance, &surfaceDescriptor);
    }
#elif defined(__EMSCRIPTEN__)
    {
#  ifdef WEBGPU_BACKEND_EMDAWNWEBGPU
        WGPUEmscriptenSurfaceSourceCanvasHTMLSelector fromCanvasHTMLSelector;
        fromCanvasHTMLSelector.chain.sType = WGPUSType_EmscriptenSurfaceSourceCanvasHTMLSelector;
#  else
        WGPUSurfaceDescriptorFromCanvasHTMLSelector fromCanvasHTMLSelector;
        fromCanvasHTMLSelector.chain.sType = WGPUSType_SurfaceDescriptorFromCanvasHTMLSelector;
#  endif
        fromCanvasHTMLSelector.chain.next = NULL;
        fromCanvasHTMLSelector.selector = "canvas";

        WGPUSurfaceDescriptor surfaceDescriptor;
        surfaceDescriptor.nextInChain = &fromCanvasHTMLSelector.chain;
        surfaceDescriptor.label = NULL;

        return wgpuInstanceCreateSurface(instance, &surfaceDescriptor);
    }  
#else
    // TODO: See SDL_syswm.h for other possible enum values!
#error "Unsupported WGPU_TARGET"
#endif
return wgpu::Surface();
}
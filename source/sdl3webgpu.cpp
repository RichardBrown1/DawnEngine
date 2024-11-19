//Taken from https://github.com/eliemichel/sdl3webgpu

/**
 * This is an extension of SDL3 for WebGPU, abstracting away the details of
 * OS-specific operations.
 * 
 * This file is part of the "Learn WebGPU for C++" book.
 *   https://eliemichel.github.io/LearnWebGPU
 * 
 * Most of this code comes from the wgpu-native triangle example:
 *   https://github.com/gfx-rs/wgpu-native/blob/master/examples/triangle/main.c
 * 
 * MIT License
 * Copyright (c) 2022-2024 Elie Michel and the wgpu-native authors
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */



#include "../include/sdl3webgpu.hpp"

#include <webgpu/webgpu_cpp.h>
#include <windows.h>

#include <SDL3/SDL.h>

wgpu::Surface SDL_GetWGPUSurface(wgpu::Instance instance, SDL_Window* window) {
	auto chainedDescriptor = setupWindowAndGetSurfaceDescriptor(window);

    wgpu::SurfaceDescriptor surfaceDescriptor;
    surfaceDescriptor.nextInChain = chainedDescriptor.get();

    wgpu::Surface surface = instance.CreateSurface(&surfaceDescriptor);
    return surface;
}

std::unique_ptr<wgpu::ChainedStruct, void (*)(wgpu::ChainedStruct*)> setupWindowAndGetSurfaceDescriptor(SDL_Window* window) {
    SDL_PropertiesID props = SDL_GetWindowProperties(window);

    wgpu::SurfaceSourceWindowsHWND* desc = new wgpu::SurfaceSourceWindowsHWND();
    desc->hwnd = (HWND)SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);
    desc->hinstance = GetModuleHandle(nullptr);

    return { desc, [](wgpu::ChainedStruct* desc) {
                delete reinterpret_cast<wgpu::SurfaceSourceWindowsHWND*>(desc);
            } };
};

//wgpu::Surface SDL_GetWGPUSurface(wgpu::Instance instance, SDL_Window* window) {
//    SDL_PropertiesID props = SDL_GetWindowProperties(window);
//    
//	HWND hwnd = (HWND)SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);
//	if (!hwnd) return nullptr;
//	HINSTANCE hinstance = GetModuleHandle(NULL);
//
//	wgpu::SurfaceSourceWindowsHWND fromWindowsHWND;
//	fromWindowsHWND.sType = wgpu::SType::SurfaceSourceWindowsHWND;
//	fromWindowsHWND.hinstance = hinstance;
//	fromWindowsHWND.hwnd = hwnd;
//
//	wgpu::SurfaceDescriptor surfaceDescriptor;
//	surfaceDescriptor.label = wgpu::StringView();
//	surfaceDescriptor.nextInChain = &fromWindowsHWND;
//
//	return instance.CreateSurface(&surfaceDescriptor);
//    
//}


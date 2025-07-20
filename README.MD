# DawnEngine
My Rudimentary Render engine in C++ using Google's DAWN Webgpu

Shaders are in HLSL -> SPIRV

![screenshot](./_readme_images/cornellbox.jpg)


## Requires
- DAWN - compile from googles source and copy over.
- VCPKG
- Models are taken from Khronos Gltf Sample Assets usually

## Why is the clip space rendered multiple times?
1. There is a bugged triangle generated if the color attachment byte limit is raised
2. No early Z-test support in WebGPU so I can't use storage textures
3. Can't use smaller texture sizes since it's not implemented in Dawn at the time I write this although it is in spec
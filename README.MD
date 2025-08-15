# DawnEngine
My Rudimentary Render engine in C++ using Google's DAWN Webgpu

Shaders are in HLSL -> SPIRV

![screenshot](./_readme_images/cornellbox.jpg)


## Requires
- DAWN - compile from googles source and copy over.
- VCPKG
- Models are taken from Khronos Gltf Sample Assets usually

## Install Instructions
1. git clone --recurse-submodules
2. Make wgsl preprocessor executable
    i. in the build_tools/wgslPreprocessor folder
    ii. mkdir build
    iii. cmake -B build
    iv. cmake --build ./build

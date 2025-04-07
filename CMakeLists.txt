cmake_minimum_required (VERSION 3.28)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("DawnEngine")

# Dawn
list(APPEND CMAKE_PREFIX_PATH "third-party/dawn")
find_package(Dawn REQUIRED)
set(ENV(DAWN_DEBUG_BREAK_ON_ERROR) 1)


#FetchContent Dependencies
include(FetchContent)

## SDL3
set(SDL_SHARED TRUE CACHE BOOL "Build a SDL shared library (if available)")
set(SDL_STATIC TRUE CACHE BOOL "Build a SDL static library (if available)")
FetchContent_Declare(
	SDL
	GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
	GIT_TAG preview-3.1.6  # Replace this with a particular git tag or git hash
	GIT_SHALLOW TRUE
	GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(SDL)
set_property(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/_deps/sdl-src" PROPERTY EXCLUDE_FROM_ALL TRUE)

## glm
FetchContent_Declare(
    glm
 	GIT_REPOSITORY	https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
)
FetchContent_MakeAvailable(glm)

# fastgltf
#option(FASTGLTF_ENABLE_CPP_MODULES 1)
FetchContent_Declare(
    fastgltf
    GIT_REPOSITORY https://github.com/spnda/fastgltf
    GIT_TAG v0.8.0
)
FetchContent_MakeAvailable(fastgltf)
get_target_property(fastgltfDirectories fastgltf::fastgltf INTERFACE_INCLUDE_DIRECTORIES)
target_include_directories(fastgltf SYSTEM INTERFACE ${fastgltfDirectories})

# Build
file(GLOB_RECURSE SRC_FILES "source/*.cpp")
add_executable (DawnEngine ${SRC_FILES})
#add_executable (DawnEngine "source/main.cpp"  "source/engine.cpp" "source/sdl3webgpu.cpp" "include/sdl3webgpu.hpp" "source/utilities.cpp"  "include/renderPipelineHelper.hpp" "source/renderPipelineHelper.cpp")

target_link_libraries(DawnEngine dawn::webgpu_dawn SDL3::SDL3 glm::glm fastgltf::fastgltf)

#Disable compile warnings on libraries
file(GLOB_RECURSE THIRD_PARTY "third_party/*.c" "third_party/*.cpp" "third_party/*.h" "third_party/*.hpp")

if (MSVC)
    target_compile_options(DawnEngine PRIVATE /W4 /WX /std:c++20)
    set_source_files_properties(${THIRD_PARTY} PROPERTIES COMPILE_FLAGS /w)
else()
    target_compile_options(DawnEngine PRIVATE -Wall -Wextra -Wpedantic -Werror -std=c++20 -fsanitize=undefined -fsanitize=address)
endif()

#Models
set(MODELS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/models)
set(MODELS_DEST_DIR ${CMAKE_CURRENT_BINARY_DIR}/models)

add_custom_target(CopyModels
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different 
    ${MODELS_SOURCE_DIR}
    ${MODELS_DEST_DIR}
) 
add_dependencies(DawnEngine CopyModels)

#Shaders
set(SHADERS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders)
set(SHADERS_DEST_DIR ${CMAKE_CURRENT_BINARY_DIR}/shaders)
set(DXC_PATH "C:/VulkanSDK/1.3.268.0/Bin/dxc.exe")

file(GLOB_RECURSE HLSL_SHADER_FILES "shaders/*.hlsl")
file(MAKE_DIRECTORY ${SHADERS_DEST_DIR})
add_custom_target(Shaders)

foreach(FILE ${HLSL_SHADER_FILES})
  get_filename_component(FILE_WE ${FILE} NAME_WE)
  add_custom_command(
     TARGET Shaders
     COMMAND ${DXC_PATH} -T vs_6_7 -E VS_main -spirv -Fo ${SHADERS_DEST_DIR}/v_${FILE_WE}.spv ${FILE}
     COMMAND ${DXC_PATH} -T ps_6_7 -E FS_main -spirv -Fo ${SHADERS_DEST_DIR}/f_${FILE_WE}.spv ${FILE}
     MAIN_DEPENDENCY ${FILE}
     COMMENT "HLSL Vertex ${FILE}"
     WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  VERBATIM)
endforeach(FILE)
add_dependencies(DawnEngine Shaders)


#Dawn
add_custom_command(TARGET DawnEngine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                "${CMAKE_CURRENT_SOURCE_DIR}/third-party/dawn/bin/webgpu_dawn.dll"
                ${CMAKE_CURRENT_BINARY_DIR}/webgpu_dawn.dll)

add_custom_command(TARGET DawnEngine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                "${CMAKE_CURRENT_SOURCE_DIR}/third-party/dawn/bin/webgpu_dawn.pdb"
                ${CMAKE_CURRENT_BINARY_DIR}/webgpu_dawn.pdb)

#SDL
add_custom_command(TARGET DawnEngine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                "${CMAKE_CURRENT_BINARY_DIR}/_deps/sdl-build/SDL3.dll"
                ${CMAKE_CURRENT_BINARY_DIR}/SDL3.dll)

add_custom_command(TARGET DawnEngine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                "${CMAKE_CURRENT_BINARY_DIR}/_deps/sdl-build/SDL3.pdb"
                ${CMAKE_CURRENT_BINARY_DIR}/SDL3.pdb)

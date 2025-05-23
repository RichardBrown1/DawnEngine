cmake_minimum_required (VERSION 3.28)

set(CMAKE_CXX_STANDARD 20)
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

#vcpkg Dependendencies
find_package(Ktx CONFIG REQUIRED)
find_package(SDL3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(fastgltf CONFIG REQUIRED)
find_package(directx-dxc CONFIG REQUIRED)
find_package(absl CONFIG REQUIRED)

# Build
file(GLOB_RECURSE SRC_FILES "source/*.cpp")
add_executable (DawnEngine ${SRC_FILES})

target_link_libraries(DawnEngine 
    PUBLIC dawn::webgpu_dawn     
    PRIVATE SDL3::SDL3 KTX::ktx glm::glm fastgltf::fastgltf absl::log
)

#Disable compile warnings on libraries
file(GLOB_RECURSE THIRD_PARTY "third_party/*.c" "third_party/*.cpp" "third_party/*.h" "third_party/*.hpp")

if (MSVC)
    target_compile_options(DawnEngine PRIVATE /W4 /WX)
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

file(MAKE_DIRECTORY ${SHADERS_DEST_DIR})
add_custom_target(Shaders)

#WGSL
#file(GLOB_RECURSE WGSL_VERTEX_SHADER_FILES "shaders/*.wgsl")
file(
    COPY "shaders/" 
    DESTINATION ${SHADERS_DEST_DIR} 
    PATTERN "*.wgsl"
)

#HLSL -> Spirv Shaders
file(GLOB_RECURSE HLSL_VERTEX_SHADER_FILES "shaders/*_v.hlsl")
foreach(FILE ${HLSL_VERTEX_SHADER_FILES})
  get_filename_component(FILE_WE ${FILE} NAME_WE)
  add_custom_command(
     TARGET Shaders
     COMMAND ${DIRECTX_DXC_TOOL} -T vs_6_7 -E vs_main -spirv -Fo ${SHADERS_DEST_DIR}/${FILE_WE}.spv ${FILE}
     MAIN_DEPENDENCY ${FILE}
     COMMENT "HLSL Vertex ${FILE}"
     WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  VERBATIM)
endforeach(FILE)

file(GLOB_RECURSE HLSL_FRAGMENT_SHADER_FILES "shaders/*_f.hlsl")
foreach(FILE ${HLSL_FRAGMENT_SHADER_FILES})
  get_filename_component(FILE_WE ${FILE} NAME_WE)
  add_custom_command(
     TARGET Shaders
     COMMAND ${DIRECTX_DXC_TOOL} -T ps_6_7 -E fs_main -spirv -Fo ${SHADERS_DEST_DIR}/${FILE_WE}.spv ${FILE}
     MAIN_DEPENDENCY ${FILE}
     COMMENT "HLSL Fragment ${FILE}"
     WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  VERBATIM)
endforeach(FILE)

file(GLOB_RECURSE HLSL_VERTEX_SHADER_FILES "shaders/*_c.hlsl")
foreach(FILE ${HLSL_VERTEX_SHADER_FILES})
  get_filename_component(FILE_WE ${FILE} NAME_WE)
  add_custom_command(
     TARGET Shaders
     COMMAND ${DIRECTX_DXC_TOOL} -T cs_6_7 -E cs_main -spirv -Fo ${SHADERS_DEST_DIR}/${FILE_WE}.spv ${FILE}
     MAIN_DEPENDENCY ${FILE}
     COMMENT "HLSL Compute ${FILE}"
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

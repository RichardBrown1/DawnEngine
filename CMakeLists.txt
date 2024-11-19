cmake_minimum_required (VERSION 3.13)

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


# SDL3
include(FetchContent)
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

# Build
add_executable (DawnEngine "source/main.cpp"  "source/dawnEngine.cpp" "source/sdl3webgpu.cpp" "include/sdl3webgpu.hpp")

target_link_libraries(DawnEngine dawn::webgpu_dawn SDL3::SDL3)

set_source_files_properties(third-party/**.cpp third-party/**.h PROPERTIES COMPILE_FLAGS -w)

if (MSVC)
    target_compile_options(DawnEngine PRIVATE /W4 /WX /std:c++20)
else()
    target_compile_options(DawnEngine PRIVATE -Wall -Wextra -Wpedantic -Werror -std=c++20 -fsanitize=undefined -fsanitize=address)
endif()



# Post Build

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

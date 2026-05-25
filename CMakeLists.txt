cmake_minimum_required(VERSION 3.24)
project(RLEngine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)
endif()

if(MSVC)
	add_compile_options(/W4)
else()
	add_compile_options(-Wall)
endif()

# --- Fetch SDL3 dependencies ---
include(FetchContent)

FetchContent_Declare(
	SDL3
	GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
	GIT_TAG release-3.4.4
	GIT_SHALLOW TRUE
	GIT_PROGRESS TRUE
)

FetchContent_Declare(
	SDL3_image
	GIT_REPOSITORY https://github.com/libsdl-org/SDL_image.git
	GIT_TAG release-3.4.2
	GIT_SHALLOW TRUE
	GIT_PROGRESS TRUE
)

FetchContent_Declare(
	SDL3_ttf
	GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf.git
	GIT_TAG release-3.2.2
	GIT_SHALLOW TRUE
	GIT_PROGRESS TRUE
)

FetchContent_Declare(
	flecs
	GIT_REPOSITORY https://github.com/SanderMertens/flecs.git
	GIT_TAG v4.1.0
	GIT_SHALLOW TRUE
	GIT_PROGRESS TRUE
)

# Disable optional SDL3_image codecs that require extra tools (NASM for dav1d)
set(SDLIMAGE_AVIF OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_JXL OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_WEBP OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(SDL3 SDL3_image SDL3_ttf flecs)

# --- Source files ---
file(GLOB_RECURSE SOURCES
	"./src/*.cpp"
	"./src/AssetManager/*.cpp"
	"./src/ECS/*.cpp"
	"./src/Game/*.cpp"
	"./third_party/imgui/*.cpp"
)

add_executable(RLEngine ${SOURCES})

target_include_directories(RLEngine PRIVATE
	"${CMAKE_SOURCE_DIR}/third_party"
	"${CMAKE_SOURCE_DIR}/third_party/lua"
	"${flecs_SOURCE_DIR}/include"
)

set_target_properties(RLEngine PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
	RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/bin"
	RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/bin"
	RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_SOURCE_DIR}/bin"
	RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_SOURCE_DIR}/bin"
)

target_link_libraries(RLEngine PRIVATE
	SDL3::SDL3
	SDL3_image::SDL3_image
	SDL3_ttf::SDL3_ttf
	flecs::flecs_static
	"${CMAKE_SOURCE_DIR}/lib/lua54.lib"
)

# Copy runtime DLLs to output directory
add_custom_command(TARGET RLEngine POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
		$<TARGET_FILE:SDL3::SDL3-shared>
		$<TARGET_FILE:SDL3_image::SDL3_image-shared>
		$<TARGET_FILE:SDL3_ttf::SDL3_ttf-shared>
		"${CMAKE_SOURCE_DIR}/lib/lua54.dll"
		"${CMAKE_SOURCE_DIR}/bin"
)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT RLEngine)
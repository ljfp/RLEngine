cmake_minimum_required(VERSION 3.10)
project(RLEngine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O2 -ggdb")

# Define Flecs as a static library with its implementation
add_library(flecs STATIC "${CMAKE_SOURCE_DIR}/third_party/flecs/flecs.c")
target_include_directories(flecs PUBLIC "${CMAKE_SOURCE_DIR}/third_party/flecs")

include_directories(
	"./third_party/"
	"./third_party/lua/"
	"./third_party/flecs/"
)

file(GLOB_RECURSE SOURCES
	"./src/*.cpp"
	"./src/AssetManager/*.cpp"
	"./src/ECS/*.cpp"
	"./src/Game/*.cpp"
	"./third_party/imgui/*.cpp")

add_executable(RLEngine ${SOURCES})

# Add Flecs example
add_executable(FlecsDirect examples/FlecsDirect.cpp)
target_link_libraries(FlecsDirect flecs)

set_target_properties(RLEngine PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
)

set_target_properties(FlecsDirect PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
)

if (WIN32)
	target_link_libraries(RLEngine
		"${CMAKE_SOURCE_DIR}/lib/SDL2.lib"
		"${CMAKE_SOURCE_DIR}/lib/SDL2main.lib"
		"${CMAKE_SOURCE_DIR}/lib/SDL2_image.lib"
		"${CMAKE_SOURCE_DIR}/lib/SDL2_ttf.lib"
		"${CMAKE_SOURCE_DIR}/lib/lua54.lib"
		flecs
	)

	add_custom_command(TARGET RLEngine POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
			"${CMAKE_SOURCE_DIR}/lib/SDL2.dll"
			"${CMAKE_SOURCE_DIR}/lib/SDL2_image.dll"
			"${CMAKE_SOURCE_DIR}/lib/SDL2_ttf.dll"
			"${CMAKE_SOURCE_DIR}/lib/lua54.dll"
			"${CMAKE_SOURCE_DIR}/bin"
	)
elseif (UNIX)
	find_package(PkgConfig REQUIRED)
	pkg_check_modules(SDL2 REQUIRED sdl2)
	pkg_check_modules(SDL2_IMAGE REQUIRED SDL2_image)
	pkg_check_modules(SDL2_TTF REQUIRED SDL2_ttf)
	pkg_check_modules(LUA REQUIRED lua5.4)

	include_directories(${SDL2_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS} ${SDL2_TTF_INCLUDE_DIRS} ${LUA_INCLUDE_DIRS})
	link_directories(${SDL2_LIBRARY_DIRS} ${SDL2_IMAGE_LIBRARY_DIRS} ${SDL2_TTF_LIBRARY_DIRS} ${LUA_LIBRARY_DIRS})

	target_link_libraries(RLEngine
		${SDL2_LIBRARIES}
		${SDL2_IMAGE_LIBRARIES}
		${SDL2_TTF_LIBRARIES}
		${LUA_LIBRARIES}
		flecs
	)
endif()
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT RLEngine)
cmake_minimum_required(VERSION 3.10)
project(RLEngine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O2 -ggdb")

include_directories(
	"./third_party/"
	"./third_party/lua/"
)

file(GLOB_RECURSE SOURCES
	"./src/*.cpp"
	"./src/AssetManager/*.cpp"
	"./src/ECS/*.cpp"
	"./src/Game/*.cpp"
	"./third_party/imgui/*.cpp")

add_executable(RLEngine ${SOURCES})

set_target_properties(RLEngine PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
)

target_link_libraries(RLEngine
	"${CMAKE_SOURCE_DIR}/lib/SDL2.lib"
	"${CMAKE_SOURCE_DIR}/lib/SDL2main.lib"
	"${CMAKE_SOURCE_DIR}/lib/SDL2_image.lib"
	"${CMAKE_SOURCE_DIR}/lib/SDL2_ttf.lib"
	"${CMAKE_SOURCE_DIR}/lib/lua54.lib"
	"${CMAKE_SOURCE_DIR}/lib/flecs.lib"
)

add_custom_command(TARGET RLEngine POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
		"${CMAKE_SOURCE_DIR}/lib/SDL2.dll"
		"${CMAKE_SOURCE_DIR}/lib/SDL2_image.dll"
		"${CMAKE_SOURCE_DIR}/lib/SDL2_ttf.dll"
		"${CMAKE_SOURCE_DIR}/lib/lua54.dll"
		"${CMAKE_SOURCE_DIR}/lib/flecs.dll"
)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT RLEngine)

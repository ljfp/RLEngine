# RLEngine

RLEngine is a 2D game engine built using SDL3. It provides a framework for developing 2D games with support for various assets like fonts, images, scripts, sounds, and tilemaps.

## Table of Contents

- [Overview](#overview)
- [Installation](#installation)
- [Building](#building)
- [Running](#running)
- [Contributing](#contributing)
- [License](#license)

## Overview

RLEngine is designed to be a flexible and powerful game engine. It leverages SDL3 for window management, input handling, and rendering. The engine uses an Entity-Component-System (ECS) architecture and includes Lua scripting support via sol2.

### Dependencies

The following libraries are fetched automatically during the CMake configure step (via FetchContent):

- [SDL3](https://github.com/libsdl-org/SDL) 3.4.4
- [SDL3_image](https://github.com/libsdl-org/SDL_image) 3.4.2
- [SDL3_ttf](https://github.com/libsdl-org/SDL_ttf) 3.2.2

Bundled in the repository:

- [Dear ImGui](https://github.com/ocornut/imgui) 1.91.0 (with SDL3 backends)
- [Lua](https://www.lua.org/) 5.4
- [spdlog](https://github.com/gabime/spdlog), [GLM](https://github.com/g-truc/glm), [sol2](https://github.com/ThePhD/sol2)

## Installation

To get started with RLEngine, clone the repository:

```sh
git clone https://github.com/yourusername/RLEngine.git
cd RLEngine
```

## Building

### Prerequisites

- [CMake](https://cmake.org/) 3.24 or newer
- A C++ compiler with C++20 support (MSVC recommended on Windows, GCC on Linux)
- [Git](https://git-scm.com/) (required for FetchContent to clone SDL3 dependencies)

On **Windows**, run commands from a [Developer Command Prompt for Visual Studio](https://learn.microsoft.com/en-us/visualstudio/ide/reference/command-prompt-powershell) or use the CMake preset which auto-selects the Visual Studio generator.

### Build Instructions

Configure the project using CMake presets:

```sh
cmake --preset default
```

> **Note:** The first configure takes several minutes as CMake downloads and builds SDL3, SDL3_image, and SDL3_ttf from source.

Build the project:

```sh
cmake --build build
```

By default the project builds in **Debug** mode. To build an optimized release:

```sh
cmake --build build --config Release
```

## Running

After building, the executable and required DLLs are placed in the `bin/` directory:

```sh
.\bin\RLEngine.exe
```

## Contributing
Contributions are welcome! Please fork the repository and submit pull requests. For major changes, please open an issue first to discuss what you would like to change.

## License
This project is licensed under the GNU GPL v3.0 License. See the LICENSE file for details.
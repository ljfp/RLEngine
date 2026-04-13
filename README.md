# RLEngine

RLEngine is a game engine built using SDL2 and OpenGL ES. It provides a robust framework for developing 2D (and maybe some day 3D) games with support for various assets like fonts, images, scripts, sounds, and tilemaps.

## Table of Contents

- [Overview](#overview)
- [Installation](#installation)
- [Building](#building)
- [Running](#running)
- [Contributing](#contributing)
- [License](#license)

## Overview

RLEngine is designed to be a flexible and powerful game engine. It leverages SDL2 for window management and input handling, and OpenGL ES for rendering (with Vulkan support in progress). The engine supports various asset types and includes examples to help you get started.

## Installation

To get started with RLEngine, clone the repository:

```sh
git clone https://github.com/yourusername/RLEngine.git
cd RLEngine
```

## Building
### Prerequisites
 - [CMake](https://cmake.org/) 4.2.0 or newer
 - A C++ compiler with C++20 support (GCC, Clang, or MSVC)
 - [Ninja](https://ninja-build.org/) (recommended build tool)

### Build Instructions

Configure the project using CMake presets:
```sh
cmake --preset default
```

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
./bin/RLEngine.exe
```

## Contributing
Contributions are welcome! Please fork the repository and submit pull requests. For major changes, please open an issue first to discuss what you would like to change.

## License
This project is licensed under the GNU GPL v3.0 License. See the LICENSE file for details.
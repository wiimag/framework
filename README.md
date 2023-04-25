Application Framework
=====================

C/C++ Application Frameowork for Windows, MacOS, and Linux.

This is a framework for building cross-platform applications in C/C++ which provide a common set of features and tools to build applications with. It is designed to be used as a submodule in other projects.

The framework is designed to be as lite and efficient as possible. It uses [Dear ImGui](https://github.com/ocornut/imgui) and [bgfx](https://github.com/bkaradzic/bgfx) to render the UI and graphics. Most of the cross platform code is written in C/C++ and uses [foundation_lib](https://github.com/mjansson/foundation_lib) for the common platform code and basic structures.

On top of these libraries, the framework provides a set of common tools and features to build applications with. These include:
* A common application framework with a common set of features and tools.
* A built-in console for logging and debugging.
* A built-in profiler for performance analysis.
* A built-in test runner to run unit and integration tests.
* A localization system to support multiple languages.
* A minimal document model system to render tabs, windows, and documents.
* Extensions on top of imgui and glfw to provide a common set of UI widgets and controls.
* Some concurrent data structures and algorithms.

## Requirements

- A C++20 compiler
- Git <https://git-scm.com/downloads>
- CMake 3.15 or later <https://cmake.org/download/>
- Python 3.8 or later <https://www.python.org/downloads/>

I also recommend using [Visual Studio Code](https://code.visualstudio.com/download) as an editor to modify CMakelists.txt files, bash scripts, etc.

### Windows

- Microsoft Visual Studio 2022 with C++ support <https://visualstudio.microsoft.com/downloads/>
    - I use the community edition which is free
    - I also recommend installing Entrian Source Search <https://entrian.com/source-search/> to search for anything in the project root folder.
- I personnally recomment using **Git Bash** on Windows since all the scripts are written for it (i.e. `./run`)

### MacOS (OSX)

- A bash shell (i.e. iTerm2 <https://iterm2.com/>) to run the scripts (i.e. `./run`)
- XCode 12.5 or later <https://developer.apple.com/xcode/>
- Apple Command Line Tools <https://developer.apple.com/download/more/>
- Homebrew <https://brew.sh/>

## Using

- imgui <https://github.com/ocornut/imgui> for the UI.
- glfw <https://github.com/glfw/glfw> to manage platform windows.
- bgfx <https://github.com/bkaradzic/bgfx> for the 2D/3D rendering backends.
- foundation_lib <https://github.com/mjansson/foundation_lib> for the common platform code and basic structures.
- libcurl <https://github.com/curl/curl> to execute various https requests.
- doctest <https://github.com/doctest/doctest> to run unit and integration tests for the various apps and framework modules.
- stb_image <https://github.com/nothings/stb> to decode various stock logos downloaded from the web.
- mnyfmt.c from <http://www.di-mare.com/adolfo/p/mnyfmt/mnyfmt.c> to format stock dollar prices.

## Getting Started

### Clone the repository

```bash
git clone git@github.com:Infineis/framework.git <app_name>
```

### Edit build settings

Edit the `config/build.settings` file to set the application name, version, etc.

### Edit sources

Then you usually want to start editing sources under `sources/`. As a starting point, you can edit the `example.cpp` file to get a feel for how the framework works.

Basically, you need to have a `cpp` file that implements minimally the following functions:
- `app_title` - Returns the application title.
- `app_configure` - Defines how the application and foundation is configured.
- `app_initialize` - Defines how the application is initialized (i.e. initialize services and modules once, etc).
- `app_shutdown` - Defines how the application is finalized (i.e. shutdown services and modules once, etc).
- `app_update` - Defines how the application is updated prior to render each frames.
- `app_render` - Defines how the application is rendered each frame.

### Build and run

Then you can build and run the application using the `./run` script.

```bash
./run generate build debug start --verbose --console
```

### Getting confident

Once you are more confident with the framework, you can rename the `example.cpp` file to something more appropriate for your application and start adding your own code as module, etc...

You can also edit the `sources/CMakelists.txt` file to add your own modules, customine the build, etc...

Remember to re-generate the solution after editing the `sources/CMakelists.txt` file.

```bash
./run generate
```

## Documentation

You can find more documentation about the application framework under [docs](docs/README.md)

## Run Command (`./run --help`)

### Generate Solution

```bash
./run generate
```

The solution will be generated in the `projects/.build` folder.

### Build Solution (In Release)

```
./run build
```

### Build Solution In Debug

```
./run build debug
```

### Build Solution and Run Tests

```
./run build tests
```

#### You can also simply run test when the solution is already built

```
./run tests --verbose --bgfx-ignore-logs
```

### Or simply run the solution (if already built)

```
./run --console --log-debug
```

### Open Solution Workspace

```
./run generate open workspace
```

### Batch it all!

```bash
./run generate build deploy tests open workspace start --verbose --console
```

This will generate the solution, build it, run the tests, open the workspace, and start the application (or course if everything went well!).

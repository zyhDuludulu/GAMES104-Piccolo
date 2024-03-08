# Piccolo Engine (formerly Pilot Engine)

<p align="center">
  <a href="https://games104.boomingtech.com">
    <img src="engine/source/editor/resource/PiccoloEngine.png" width="400" alt="Piccolo Engine logo">
  </a>
</p>

**Piccolo Engine** is a tiny game engine used for the [GAMES104](https://games104.boomingtech.com) course.

## Continuous build status

|    Build Type     |                                                                                      Status                                                                                      |
| :---------------: | :------------------------------------------------------------------------------------------------------------------------------------------------------------------------------: |
| **Build Windows** | [![Build Windows](https://github.com/BoomingTech/Piccolo/actions/workflows/build_windows.yml/badge.svg)](https://github.com/BoomingTech/Piccolo/actions/workflows/build_windows.yml) |

## Prerequisites

To build Piccolo, you must first install the following tools.

### Windows 10/11
- CMake 3.19 (or more recent)
- Git 2.1 (or more recent)

## Build Piccolo

### Build on Windows
You may execute the **build_windows.bat**. This batch file will generate the projects, and build the **Release** config of **Piccolo Engine** automatically. After successful build, you can find the PiccoloEditor.exe at the **bin** directory.

Because I want to use Clangd, so I use Ninja as my generator to generate 'compile_commands.json'
```
cmake -S . -B build -G Ninja
```

## Documentation
For documentation, please refer to the Wiki section.

## Extra

### Vulkan Validation Layer: Validation Error
We have noticed some developers on Windows encounted PiccoloEditor.exe could run normally but reported an exception Vulkan Validation Layer: Validation Error
when debugging. You can solve this problem by installing Vulkan SDK (official newest version will do).

### Generate Compilation Database

You can build `compile_commands.json` with the following commands when `Unix Makefiles` generaters are avaliable. `compile_commands.json` is the file
required by `clangd` language server, which is a backend for cpp lsp-mode in Emacs.

For Windows:

``` powershell
cmake -DCMAKE_TRY_COMPILE_TARGET_TYPE="STATIC_LIBRARY" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -S . -B compile_db_temp -G "Unix Makefiles"
copy compile_db_temp\compile_commands.json .
```

### Using Physics Debug Renderer
Currently Physics Debug Renderer is only available on Windows. You can use the following command to generate the solution with the debugger project.

``` powershell
cmake -S . -B build -DENABLE_PHYSICS_DEBUG_RENDERER=ON
```

Note:
1. Please clean the build directory before regenerating the solution. We've encountered building problems in regenerating directly with previous CMakeCache.
2. Physics Debug Renderer will run when you start PiccoloEditor. We've synced the camera position between both scenes. But the initial camera mode in Physics Debug Renderer is wrong. Scrolling down the mouse wheel once will change the camera of Physics Debug Renderer to the correct mode.

### Homework 

# Homework 2

Construct a map from rgb to a uv coordinate on the lut. This method will process a post processing to the rendering pipeline.

Note:
If we want to use the color-grading, we need to restrict the level of mipmap to 1. (At engine/source/runtime/function/render/render_resource.cpp, line 88)
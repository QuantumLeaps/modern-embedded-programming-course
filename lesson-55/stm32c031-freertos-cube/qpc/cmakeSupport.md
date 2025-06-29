# cmake Support in QP/C

This branch adds comprehensive cmake support to QP/C

## Quick Start

create your project with a root `CMakeLists.txt` file, following this blueprint.
1. copy [qpc_sdk_import.cmake](https://github.com/QuantumLeaps/3rd_party/cmake/qpc_sdk_import.cmake) into your project. Make sure, it can be found by `cmake` as an included script
2. Setup your 1<sup>st</sup> `CMakeLists.txt`:
```
# use a recent CMake version
cmake_minimum_required(VERSION 3.23 FATAL_ERROR)
cmake_policy(VERSION 3.23...3.28)
cmake_policy(SET CMP0083 NEW)
cmake_policy(SET CMP0105 NEW)
cmake_policy(SET CMP0116 NEW)
cmake_policy(SET CMP0128 NEW)

# include general project config & import qpc
set(QPC_SDK_PATH ${CMAKE_SOURCE_DIR}/Source/qpc-sdk)
# set(QPC_FETCH_FROM_GIT ON)
# set(QPC_FETCH_FROM_GIT_PATH ${CMAKE_SOURCE_DIR}/Source/qpc-sdk)
include(qpc_sdk_import)

# default image/project name is trafficlight
# Give a special name via -DIMAGE=<image>
# the main project
project(myProject
    VERSION "1.0.0""
    DESCRIPTION "my 1st qpc project"
    LANGUAGES C CXX)

# the project target(s)
add_executable(qpcApp main.c qpcApp.c)

include(${QPC_SDK_PATH}/qpc_sdk_init.cmake)
set(QPC_PROJECT qpcPrj)
set(QPC_CFG_KERNEL QV)
set(QPC_CFG_GUI TRUE)
set(QPC_CFG_PORT win32)
# QP/C 8.0.0: to include a local 'qp_config.h' add the related include path
# to the qpc build settings. Replace "${CMAKE_CURRENT_LIST_DIR}/include" by
# your project specific path!
set(QPC_CFG_QPCONFIG_H_INCLUDE_PATH ${CMAKE_CURRENT_LIST_DIR}/include)
qpc_sdk_init()

target_link_libraries(qpcApp PRIVATE qpc)
```
3. configure your project with
   `cmake -B Build .`
4. build
   `cmake --build Build`

## Usage
### `qpc_sdk_import.cmake`
This file prepares your project for integrating qpc.
Before adding this file to your project with `include(qpc_sdk_import)` make sure to set `CMAKE_MODULE_PATH` accordingly.

To configure the integration of qpc you can provide information either with cmake variables or via environment variables of the very same names.

* Mandatory variables (only one of the two must be set)
  - `QPC_SDK_PATH` - set this variable to point to the full path of an already installed qpc instance.
  - `QPC_FETCH_FROM_GIT` - set this variable to ON or TRUE, if no pre-installed qpc directory exists. QPC
    will then be downloaded from git automatically. The download URL is pre-defined in `qpc_sdk_import.cmake`
* Optional variables
  - `QPC_FETCH_FROM_GIT_PATH` - set this variable to download qpc from git (`QPC_FETCH_FROM_GIT`) into the
    specified directory
  - `QPC_URL`- set this variable to the URL to download qpc from. This must point to a remote git
    repository

### `qpc_sdk_init.cmake`
This file is situated in the root directory of qpc. It performs a pre-initialization of the qpc package and provides the function `qpc_sdk_init`. Call this function from your project's `CMakeLists.txt` file to perform the final integration of qpc into your project. To configure qpc to your projects requirements set these variables before calling `qpc_sdk_init()`

* `QPC_CFG_KERNEL` - STRING: set this variable to the QPC kernel for your project. Valid values are QV, QK or QXK. Default: QV
* `QPC_CFG_PORT` - STRING: set this variable to reflect the target platform of your project. Default: host system. Valid values are:
  + `arm-cm`, `arm-cr` - Arm CortexM or CortexR micro controllers. Tested with GNU cross compiler environments.
  + `freertos`, `esp-idf`, `emb-os`, `threadx`, `uc-os2` - real time OS
  + `msp430`, `pic32` - TI MSP430 or PIC32 micro controllers
  + `riscv`- Risc V micro controllers
  + `qep-only`, `qube` - test environments
  + `win32`, `posix` - host environments MS Winows, Linux (Posix compatible systems)
* `QPC-CFG-GUI` - BOOL: set this boolean variable to ON/TRUE, if GUI support (win32) shall be compiled in. Default: OFF
* `QPC_CFG_UNIT_TEST` - BOOL: set this to ON/TRUE to support qutest, if build configuration `Spy` is active. Default: OFF
* `QPC_CFG_VERBOSE` - BOOL: set this to enable more verbosity in message output. Default: OFF
* `QPC_CFG_QPCONFIG_H_INCLUDE_PATH`: - STRING (PATH): (`QP/C 8.0.0`) set this to have the build of QP/C use your project specific `qp_config.h`.
  Default: `${QPC_SDK_PATH}/ports/config`

### General usage hints
1. Set `QPC_SDK_PATH` or `QPC_FETCH_FROM_GIT` either in your `CMakeLists.txt` file or as an environment variable.
2. Optionally set the configuration variable(s)
3. Include `qpc_sdk_import` __before__ defining the cmake `project()`
4. Define the project
5. Define the cmake target (executable or library)
6. Include `qpc_sdk_init.cmake`
7. configure the qpc SDK
8. call `qpc_sdk_init`
9. Add the qpc library to your cmake target:
   `target_link_libraries(<target>> PRIVATE qpc)`

Generate and build your cmake project

## Generation and building hints
* Generate with configuration support
  The recommendation is to use a multi-configuration cmake generator like `"Ninja Multi-Config"` and set the cmake variable `CMAKE_CONFIGURATION_TYPES` to `"Debug;Release;Spy"`.
  Then you can build with `cmake --build <build directory> --config=<config>.
* Use `CMakePresets.json`
  Define the build configurations for your projects in a presets definitions file.
  Refer to the [CMakePresets.json manual](https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html) for further details.
  Then you generate with `cmake --preset=<preset> .` from your project directory. The build then can be started with `cmake --build --preset=<preset>`.

### QPC configurations support
Many `qpc` examples provide 3 build configurations:
* `Debug` - build with debug support and debug symbols. Most optimizations are turned off
* `Release` - build without debug support. Activate optimizations instead
* `Spy` - build like `Debug`. Additionally activate support for `QSpy`.

These configurations are also supported by qpc with cmake. Different possibilities exist to activate those.

### `qp_config.h` support
With the release of QP/C V8.0.0 the inclusion of `qp_config.h` is mandatory.
The `cmake` build system of qpc addresses this by providing the configuration variable `QPC_CFG_QPCONFIG_H_INCLUDE_PATH`. Set this to the path of your local project's `qp_config.h` and this will automatically be found by the build system. Do this in your main `CMakeLists.txt` file __before__ calling `qpc_sdk_init()`.

You do not need to set this variable, should the qpc default settings be sufficient for your project. In this case the build system uses the `qp_config.h` file, as it can be found in the directory `${QPC_SDK_PATH}/src/ports/config`.

An example can be found in the [cmake dpp example](https://github.com/QuantumLeaps/qpcpp-examples/tree/main/posix-win32-cmake/dpp). Have a look into
the example's [CMakeLists.txt](https://github.com/QuantumLeaps/qpcpp-examples/blob/main/posix-win32-cmake/dpp/CMakeLists.txt).

### Multi configuration generators
The most easy way to make use of the different configurations is to use a multi config generator like `Ninja Multi-Config` or `MS Visual Studio`.
Using one of such generators enables to generate the build system using `cmake` and afterwards simply selecting the desired build configuration like
`cmake --build <Build Directory> --config=<Debug|Release|Spy>`

To support this, the `cmake` variables
* `CMAKE_C_FLAGS_<CONFIGURATION>`
* `CMAKE_CXX_FLAGS_<CONFIGURATION>`
* `CMAKE_ASM_FLAGS_<CONFIGURATION>`
* `CMAKE_EXE_LINKER_FLAGS_<CONFIGURATION>`

have to be set for all configurations. The desired place to hold these settings is the `toolchain` file of the compilation toolchain in use.
If no `toolchain` file is used, the `cmake` default configuration provides settings for the `Debug` and `Release` configuration fot the host
compiler setup. The `Spy` configuration will be added by the qpc `CMakeLists.txt` file.

### Single configuration generators
For single configuration generators like `Makefile` or `Ninja`, specific build configurations need to configured. One for each configuration.
When generationg the build system, set the `cmake` variable `CMAKE_BUILD_TYPE` to the desired configuration (`Debug`, `Release` or `Spy`).

Everything said above concerning the `CMAKE_<LANG>_FLAGS_<CONFIGURATION>` variables, also applies here.

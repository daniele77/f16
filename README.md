# f16

[![ci linux](https://github.com/daniele77/f16/actions/workflows/ubuntu.yml/badge.svg)](https://github.com/daniele77/f16/actions/workflows/ubuntu.yml)
[![ci win](https://github.com/daniele77/f16/actions/workflows/windows.yml/badge.svg)](https://github.com/daniele77/f16/actions/workflows/windows.yml)
[![ci macos](https://github.com/daniele77/f16/actions/workflows/macos.yml/badge.svg)](https://github.com/daniele77/f16/actions/workflows/macos.yml)
[![codecov](https://codecov.io/gh/daniele77/f16/branch/main/graph/badge.svg)](https://codecov.io/gh/daniele77/f16)
[![CodeQL](https://github.com/daniele77/f16/actions/workflows/codeql-analysis.yml/badge.svg)](https://github.com/daniele77/f16/actions/workflows/codeql-analysis.yml)

## About f16

f16 is a versatile and lightweight C++17 library for building efficient HTTP and HTTPS servers, ideal for REST APIs and custom web applications.
It provides a simple and intuitive API for defining routes, handling requests, and serving static or dynamic content.
Additionally, f16 includes a high-performance static web server, f16-server, which leverages the library’s capabilities.

## Features
- Supports both HTTP and HTTPS connections.
- Defines routes using clear and concise syntax.
- Handles GET, PUT, and other HTTP methods.
- Extracts query parameters and path variables from requests.
- Serves static content from the filesystem.
- Enables generation of dynamic content using lambda functions.

## Supported RFC
The f16 project supports the following RFCs:

- **HTTP/1.0**:
  - RFC 1945: Hypertext Transfer Protocol -- HTTP/1.0

- **HTTPS**:
  - RFC 2818: HTTP Over TLS

- **HTTP Methods (GET, PUT, POST, HEAD)**:
  - RFC 7231: Hypertext Transfer Protocol (HTTP/1.1): Semantics and Content

## Requirements

- C++17 compiler
- Asio library
- CMake 3.10 or higher

## Build Instructions

### Clone the repository
   
    git clone https://github.com/daniele77/f16.git
    cd f16

### Specify the compiler using environment variables

By default (if you don't set environment variables `CC` and `CXX`), the system default compiler will be used.

Conan and CMake use the environment variables CC and CXX to decide which compiler to use. So to avoid the conflict issues only specify the compilers using these variables.

CMake will detect which compiler was used to build each of the Conan targets. If you build all of your Conan targets with one compiler, and then build your CMake targets with a different compiler, the project may fail to build.

Commands for setting the compilers:

- Debian/Ubuntu/MacOS:

	Set your desired compiler (`clang`, `gcc`, etc):

	- Temporarily (only for the current shell)

		Run one of the followings in the terminal:

		- clang

				CC=clang CXX=clang++

		- gcc

				CC=gcc CXX=g++

	- Permanent:

		Open `~/.bashrc` using your text editor:

			gedit ~/.bashrc

		Add `CC` and `CXX` to point to the compilers:

			export CC=clang
			export CXX=clang++

		Save and close the file.

- Windows:

	- Permanent:

		Run one of the followings in PowerShell:

		- Visual Studio generator and compiler (cl)

				[Environment]::SetEnvironmentVariable("CC", "cl.exe", "User")
				[Environment]::SetEnvironmentVariable("CXX", "cl.exe", "User")
				refreshenv

		  Set the architecture using [vcvarsall](https://docs.microsoft.com/en-us/cpp/build/building-on-the-command-line?view=vs-2019#vcvarsall-syntax):

				vcvarsall.bat x64

		- clang

				[Environment]::SetEnvironmentVariable("CC", "clang.exe", "User")
				[Environment]::SetEnvironmentVariable("CXX", "clang++.exe", "User")
				refreshenv

		- gcc

				[Environment]::SetEnvironmentVariable("CC", "gcc.exe", "User")
				[Environment]::SetEnvironmentVariable("CXX", "g++.exe", "User")
				refreshenv


  - Temporarily (only for the current shell):

			$Env:CC="clang.exe"
			$Env:CXX="clang++.exe"


### Configure the build

    cmake -S . -B ./build

### Build the project

    cmake --build ./build

For Visual Studio, give the build configuration (Release, RelWithDeb, Debug, etc) like the following:

    cmake --build ./build -- /p:configuration=Release

### Running the tests

You can use the `ctest` command run the tests.

```shell
cd ./build
ctest -C Debug
cd ../
```

## Building with Conan

    conan install . -of build --build=missing
    cmake -S . -B ./build -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=RELEASE
    cmake --build .

## Building with cmake presets

    cmake --list-presets
    cmake . --preset <configure-preset>
    cmake --build <directory>
    ctest --preset <test-preset>

Using Conan (requires cmake >= 3.23)

    conan profile detect # only the first time
    conan install . --output-folder=build
    cmake --list-presets
    cmake --preset conan-release
    cmake --build --preset conan-release
    ctest --preset conan-release

## Docker Instructions

If you have [Docker](https://www.docker.com/) installed, you can run this
in your terminal, when the Dockerfile is inside the `.devcontainer` directory:

```bash
docker build -f ./.devcontainer/Dockerfile --tag=f16-development:latest .
docker run -it f16-development:latest
```

This command will put you in a `bash` session in a Ubuntu 20.04 Docker container,
with all of the tools listed in the [Dependencies](#dependencies) section already installed.
Additionally, you will have `g++-11` and `clang++-13` installed as the default
versions of `g++` and `clang++`.

If you want to build this container using some other versions of gcc and clang,
you may do so with the `GCC_VER` and `LLVM_VER` arguments:

```bash
docker build --tag=f16-development:latest --build-arg GCC_VER=10 --build-arg LLVM_VER=11 .
```

The CC and CXX environment variables are set to GCC version 11 by default.
If you wish to use clang as your default CC and CXX environment variables, you
may do so like this:

```bash
docker build --tag=f16-development:latest --build-arg USE_CLANG=1 .
```

You will be logged in as root, so you will see the `#` symbol as your prompt.

If you want to run the container with a specific user rather than the default root user,
you can use the `USERNAME`, `USER_UID`, and `USER_GID` build arguments.
This can be useful for matching the user inside the container with the user running the Docker commands,
thereby avoiding permission issues when sharing files between the host and the container.

To use the current bash user, you can build the container like this:

```bash
docker build -f ./.devcontainer/Dockerfile \
    --tag=f16-development:latest \
    --build-arg USERNAME=$(whoami) \
    --build-arg USER_UID=$(id -u) \
    --build-arg USER_GID=$(id -g) .
```

This will create a container that runs with the current user's name, user ID, and group ID,
as specified on your host machine.

If you need to mount your local copy directly in the Docker image, see
[Docker volumes docs](https://docs.docker.com/storage/volumes/).
TLDR:

```bash
docker run -it \
	-v absolute_path_on_host_machine:absolute_path_in_guest_container \
	f16-development:latest
```

You can configure and build [as directed above](#build) using these commands:

```bash
/f16# cmake -S . -B ./build
/f16# cmake --build ./build
```

You can configure and build using `clang-13`, without rebuilding the container,
with these commands:

```bash
/f16# CC=clang CXX=clang++ cmake -S . -B ./build
/f16# cmake --build ./build
```

The `ccmake` tool is also installed; you can substitute `ccmake` for `cmake` to
configure the project interactively.
All of the tools this project supports are installed in the Docker image;
enabling them is as simple as flipping a switch using the `ccmake` interface.
Be aware that some of the sanitizers conflict with each other, so be sure to
run them separately.


## Usage


See [f16 library](README_f16lib.md) to learn how to write applications
using the `f16lib` library,
and [f16 server](README_f16server.md) to learn how to launch and use the
`f16 server`.

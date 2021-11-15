## Features
* Logging to file, stdout
* Logging levels (fatal - trace)
* Multithreading safety

## Supported operating systems
* Ubuntu
* MacOS
* Windows

## Build requirements
* C99 compiler
* [CMake 3.10+](https://cmake.org/)

## Cloning
```
git clone https://github.com/cfnptr/logy
cd logy
git submodule update --init --recursive
```

## Building
```
cmake -DCMAKE_BUILD_TYPE=Release -S . -B build/
cmake --build build/
```

## Third-party
* [mpio](https://github.com/cfnptr/mpio/) (Apache-2.0 License)
* [mpmt](https://github.com/cfnptr/mpmt/) (Apache-2.0 License)

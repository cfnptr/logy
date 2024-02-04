# LOGY

A [library](https://github.com/cfnptr/logy) providing generic API for messages **logging** across different platforms.

See the [documentation](https://cfnptr.github.io/logy)

## Features

* Logging to file, stdout
* Logging levels (fatal - trace)
* Log file rotation
* Multithreading safety
* C and C++ implementations

# Usage example

```cpp
void loggerExampleCPP()
{
    logy::Logger logger("logs", INFO_LOG_LEVEL);

    int someValue = 123;
    logger.log(INFO_LOG_LEVEL, "Logged value: %d", someValue);
}
```

```c
void loggerExampleC()
{
    Logger logger;
    LogyResult logyResult = createLogger("logs", INFO_LOG_LEVEL, true, 0.0, &logger);
    if (logyResult != SUCCESS_LOGY_RESULT) abort();

    int someValue = 123;
    logMessage(logger, INFO_LOG_LEVEL, "Logged value: %d", someValue);

    destroyLogger(logger);
}
```

## Supported operating systems

* Windows
* macOS
* Ubuntu (Linux)

## Build requirements

* C99 compiler
* C++17 compiler (optional)
* [Git 2.30+](https://git-scm.com/)
* [CMake 3.16+](https://cmake.org/)

Use building [instructions](BUILDING.md) to install all required tools and libraries.

### CMake options

| Name              | Description               | Default value |
|-------------------|---------------------------|---------------|
| LOGY_BUILD_SHARED | Build LOGY shared library | `ON`          |

## Cloning

```
git clone --recursive https://github.com/cfnptr/logy
```

## Building ![CI](https://github.com/cfnptr/logy/actions/workflows/cmake.yml/badge.svg)

* Windows: ```./scripts/build-release.bat```
* macOS / Ubuntu: ```./scripts/build-release.sh```

## Third-party

* [mpio](https://github.com/cfnptr/mpio/) (Apache-2.0 License)
* [mpmt](https://github.com/cfnptr/mpmt/) (Apache-2.0 License)
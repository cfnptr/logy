# LOGY ![CI](https://github.com/cfnptr/logy/actions/workflows/cmake.yml/badge.svg)

A library providing generic API for messages logging across different platforms.

## Features

* Logging to file, stdout
* Logging levels (fatal - trace)
* Log file rotation
* Multithreading safety
* C++ wrapper

# Usage example
```c
void loggerExample()
{
    Logger logger;

    LogyResult logyResult = createLogger(
        "logs", INFO_LOG_LEVEL, true, 0.0, &logger);

    if (logyResult != SUCCESS_LOGY_RESULT)
        abort();

    int someValue = 123;

    logMessage(logger, INFO_LOG_LEVEL,
        "Logged value: %d", someValue);

    destroyLogger(logger);
}
```

## Supported operating systems

* Ubuntu
* MacOS
* Windows

## Build requirements

* C99 compiler
* [Git 2.30+](https://git-scm.com/)
* [CMake 3.16+](https://cmake.org/)

### CMake options

| Name              | Description               | Default value |
|-------------------|---------------------------|---------------|
| LOGY_BUILD_SHARED | Build LOGY shared library | `ON`          |

## Cloning

```
git clone --recursive https://github.com/cfnptr/logy
```

## Third-party

* [mpio](https://github.com/cfnptr/mpio/) (Apache-2.0 License)
* [mpmt](https://github.com/cfnptr/mpmt/) (Apache-2.0 License)
// Copyright 2021-2022 Nikita Fediuchin. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once
#include "logy/defines.h"

#include <stdarg.h>
#include <stdbool.h>

/*
 * Logger structure.
 */
typedef struct Logger_T Logger_T;
/*
 * Logger instance.
 */
typedef Logger_T* Logger;

/*
 * Create a new logger instance.
 * Returns operation logy result.
 *
 * directoryPath - logs directory path string.
 * level - logging level, inclusive.
 * logToConsole - also log to stdout.
 * rotationTime - log rotation delay time or 0. (seconds)
 * logger - pointer to the logger instance.
 */
LogyResult createLogger(
	const char* directoryPath,
	LogLevel level,
	bool logToStdout,
	double rotationTime,
	Logger* logger);
/*
 * Destroys logger instance.
 * logger - logger instance or NULL.
 */
void destroyLogger(Logger logger);

/*
 * Returns logger directory path string.
 * (Thread safe function)
 *
 * logger - logger instance.
 */
const char* getLoggerDirectoryPath(Logger logger);
/*
 * Returns logger file path string.
 * (Thread safe function)
 *
 * logger - logger instance.
 */
const char* getLoggerFilePath(Logger logger);
/*
 * Returns current logger rotation delay time. (seconds)
 * (Thread safe function)
 *
 * logger - logger instance.
 */
double getLoggerRotationTime(Logger logger);

/*
 * Returns current logger logging level.
 * (Thread safe function)
 *
 * logger - logger instance.
 */
LogLevel getLoggerLevel(Logger logger);
/*
 * Set logger logging level.
 * Log only message <= log level.
 * (Thread safe function)
 *
 * logger - logger instance.
 * level - message logging level.
 */
void setLoggerLevel(
	Logger logger,
	LogLevel level);

/*
 * Returns current logger log to stdout state.
 * (Thread safe function)
 *
 * logger - logger instance.
 */
bool getLoggerLogToStdout(Logger logger);
/*
 * Log messages also to stdout.
 * (Thread safe function)
 *
 * logger - logger instance.
 * logToStdout - value.
 */
void setLoggerLogToStdout(
	Logger logger,
	bool logToStdout);

/*
 * Log message to the log.
 * (Thread safe function)
 *
 * logger - logger instance.
 * level - message logging level.
 * fmt - formatted message.
 * args - message arguments.
 */
void logVaMessage(
	Logger logger,
	LogLevel level,
	const char* fmt,
	va_list args);
/*
 * Log message to the log.
 * (Thread safe function)
 *
 * logger - logger instance.
 * level - message logging level.
 * fmt - formatted message.
 * ... - message arguments.
 */
void logMessage(
	Logger logger,
	LogLevel level,
	const char* fmt,
	...);

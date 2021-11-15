// Copyright 2021 Nikita Fediuchin. All rights reserved.
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

// Logger instance handle.
typedef struct Logger* Logger;

/*
 * Create a new logger instance.
 * Returns operation conf result.
 *
 * filePath - logger file path string.
 * logToConsole - also log to stdout.
 * confReader - pointer to the logger instance.
 */
LogyResult createLogger(
	const char* filePath,
	LogLevel level,
	bool logToStdout,
	Logger* logger);

/*
 * Destroy logger instance.
 * logger - logger instance or NULL.
 */
void destroyLogger(Logger logger);

/*
 * Returns current logger log level.
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
LogLevel getLoggerLogToStdout(Logger logger);

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
 * ... - message arguments.
 */
void logMessage(
	Logger logger,
	LogLevel level,
	const char* fmt,
	...);
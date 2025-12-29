// Copyright 2021-2025 Nikita Fediuchin. All rights reserved.
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

/***********************************************************************************************************************
 * @file
 * @brief Common Logy library functions.
 **********************************************************************************************************************/

#pragma once
#include <stdint.h>
#include <assert.h>

// Note: Using defines here because of preprocessor.

#define OFF_LOG_LEVEL 0
#define FATAL_LOG_LEVEL 1
#define ERROR_LOG_LEVEL 2
#define WARN_LOG_LEVEL 3
#define INFO_LOG_LEVEL 4
#define DEBUG_LOG_LEVEL 5
#define TRACE_LOG_LEVEL 6
#define ALL_LOG_LEVEL 7
#define LOG_LEVEL_COUNT 8

/**
 * @brief Log level type.
 */
typedef uint8_t LogLevel;

/**
 * @brief Logy result codes.
 */
typedef enum LogyResult_T
{
	SUCCESS_LOGY_RESULT = 0,
	FAILED_TO_ALLOCATE_LOGY_RESULT = 1,
	FAILED_TO_OPEN_FILE_LOGY_RESULT = 2,
	FAILED_TO_GET_DIRECTORY_LOGY_RESULT = 3,
	LOGY_RESULT_COUNT = 4,
} LogyResult_T;
/**
 * @brief Logy result code type.
 */
typedef uint8_t LogyResult;

/**
 * @brief Logy result code string array.
 */
static const char* const logyResultStrings[LOGY_RESULT_COUNT] =
{
	"Success",
	"Failed to allocate",
	"Failed to open file",
	"Failed to get directory",
};

/**
 * @brief Returns Logy result code as a string.
 * @param result logy result code
 * @result Logy result code string. Or "Unknown LOGY result" if out of range.
 */
inline static const char* logyResultToString(LogyResult result)
{
	if (result >= LOGY_RESULT_COUNT) return "Unknown LOGY result";
	return logyResultStrings[result];
}

/**
 * @brief Log level string array.
 */
static const char* const logLevelStrings[LOG_LEVEL_COUNT] =
{
	"OFF", "FATAL", "ERROR", "WARN", "INFO", "DEBUG", "TRACE", "ALL",
};

/**
 * @brief Returns log level as string.
 * @param level log level
 * @result Log level string. Or "UNKNOWN" if out of range.
 */
inline static const char* logLevelToString(LogLevel level)
{
	if (level >= LOG_LEVEL_COUNT) return "UNKNOWN";
	return logLevelStrings[level];
}
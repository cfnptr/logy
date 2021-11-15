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

#include "logy/logger.h"
#include "logy/file.h"
#include "mpmt/sync.h"

#include <time.h>

struct Logger
{
	LogLevel level;
	bool logToStdout;
	Mutex mutex;
	FILE* file;
};

LogyResult createLogger(
	const char* filePath,
	LogLevel level,
	bool logToStdout,
	Logger* _logger)
{
	assert(filePath != NULL);
	assert(level >= OFF_LOG_LEVEL);
	assert(level <= ALL_LOG_LEVEL);
	assert(_logger != NULL);

	Logger logger = malloc(
		sizeof(struct Logger));

	if (logger == NULL)
		return FAILED_TO_ALLOCATE_LOGY_RESULT;

	Mutex mutex = createMutex();

	if (mutex == NULL)
	{
		free(logger);
		return  FAILED_TO_ALLOCATE_LOGY_RESULT;
	}

	FILE* file = openFile(
		filePath,
		"a+");

	if (file == NULL)
	{
		destroyMutex(mutex);
		free(logger);
		return FAILED_TO_OPEN_FILE_LOGY_RESULT;
	}

	logger->level = level;
	logger->logToStdout = logToStdout;
	logger->mutex = mutex;
	logger->file = file;

	*_logger = logger;
	return SUCCESS_LOGY_RESULT;
}

void destroyLogger(Logger logger)
{
	if (logger == NULL)
		return;

	closeFile(logger->file);
	destroyMutex(logger->mutex);
	free(logger);
}

LogLevel getLoggerLevel(Logger logger)
{
	assert(logger != NULL);

	Mutex mutex = logger->mutex;
	lockMutex(mutex);
	LogLevel level = logger->level;
	unlockMutex(mutex);
	return level;
}

void setLoggerLevel(
	Logger logger,
	LogLevel level)
{
	assert(logger != NULL);
	assert(level >= OFF_LOG_LEVEL);
	assert(level <= ALL_LOG_LEVEL);

	Mutex mutex = logger->mutex;
	lockMutex(mutex);
	logger->level = level;
	unlockMutex(mutex);
}

LogLevel getLoggerLogToStdout(Logger logger)
{
	assert(logger != NULL);

	Mutex mutex = logger->mutex;
	lockMutex(mutex);
	bool logToStdout = logger->logToStdout;
	unlockMutex(mutex);
	return logToStdout;
}

void setLoggerLogToStdout(
	Logger logger,
	bool logToStdout)
{
	assert(logger != NULL);

	Mutex mutex = logger->mutex;
	lockMutex(mutex);
	logger->logToStdout = logToStdout;
	unlockMutex(mutex);
}

void logMessage(
	Logger logger,
	LogLevel level,
	const char* fmt,
	...)
{
	assert(logger != NULL);
	assert(level > OFF_LOG_LEVEL);
	assert(level < ALL_LOG_LEVEL);
	assert(fmt != NULL);

	Mutex mutex = logger->mutex;
	lockMutex(mutex);

	if (level > logger->level)
	{
		unlockMutex(mutex);
		return;
	}

	time_t rawTime;
	time(&rawTime);

#if __linux__ || __APPLE__
	struct tm timeInfo =
		*localtime(&rawTime);
#elif _WIN32
	struct tm timeInfo;

	errno_t error = localtime_s(
		&timeInfo,
		&rawTime);

	if (error != 0)
		avort();
#else
#error Unknown operating system
#endif

	FILE* file = logger->file;

	fprintf(
		file,
		"[%d-%d-%d %d:%d:%d] [%s]: ",
		timeInfo.tm_year + 1900,
		timeInfo.tm_mon + 1,
		timeInfo.tm_mday,
		timeInfo.tm_hour,
		timeInfo.tm_min,
		timeInfo.tm_sec,
		logLevelToString(level));

	va_list args;
	va_start(args, fmt);
	vfprintf(file, fmt, args);
	va_end(args);

	fputc('\n', file);
	fflush(file);

	if (logger->logToStdout == true)
	{
		fprintf(
			stdout,
			"[%d-%d-%d %d:%d:%d] [%s]: ",
			timeInfo.tm_year + 1900,
			timeInfo.tm_mon + 1,
			timeInfo.tm_mday,
			timeInfo.tm_hour,
			timeInfo.tm_min,
			timeInfo.tm_sec,
			logLevelToString(level));

		va_start(args, fmt);
		vfprintf(stdout, fmt, args);
		va_end(args);

		fputc('\n', stdout);
		fflush(stdout);
	}

	unlockMutex(mutex);
}
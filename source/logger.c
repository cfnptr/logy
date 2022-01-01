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

#include "logy/logger.h"
#include "mpio/file.h"
#include "mpmt/sync.h"

#include <time.h>

struct Logger_T
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
	Logger* logger)
{
	assert(filePath != NULL);
	assert(level <= ALL_LOG_LEVEL);
	assert(logger != NULL);

	Logger loggerInstance = malloc(
		sizeof(Logger_T));

	if (loggerInstance == NULL)
		return FAILED_TO_ALLOCATE_LOGY_RESULT;

	Mutex mutex = createMutex();

	if (mutex == NULL)
	{
		free(loggerInstance);
		return  FAILED_TO_ALLOCATE_LOGY_RESULT;
	}

	FILE* file = openFile(
		filePath,
		"a+");

	if (file == NULL)
	{
		destroyMutex(mutex);
		free(loggerInstance);
		return FAILED_TO_OPEN_FILE_LOGY_RESULT;
	}

	loggerInstance->level = level;
	loggerInstance->logToStdout = logToStdout;
	loggerInstance->mutex = mutex;
	loggerInstance->file = file;

	*logger = loggerInstance;
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
	assert(level <= ALL_LOG_LEVEL);

	Mutex mutex = logger->mutex;
	lockMutex(mutex);
	logger->level = level;
	unlockMutex(mutex);
}

bool getLoggerLogToStdout(Logger logger)
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

void logVaMessage(
	Logger logger,
	LogLevel level,
	const char* fmt,
	va_list args)
{
	assert(logger != NULL);
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
		abort();
#else
#error Unknown operating system
#endif

	if (logger->logToStdout == true)
	{
		fprintf(
			stdout,
			"[%d-%02d-%02d %02d:%02d:%02d] [%s]: ",
			timeInfo.tm_year + 1900,
			timeInfo.tm_mon + 1,
			timeInfo.tm_mday,
			timeInfo.tm_hour,
			timeInfo.tm_min,
			timeInfo.tm_sec,
			logLevelToString(level));

		va_list stdArgs;
		va_copy(stdArgs, args);
		vfprintf(stdout, fmt, stdArgs);
		va_end(stdArgs);

		fputc('\n', stdout);
		fflush(stdout);
	}

	FILE* file = logger->file;

	fprintf(
		file,
		"[%d-%02d-%02d %02d:%02d:%02d] [%s]: ",
		timeInfo.tm_year + 1900,
		timeInfo.tm_mon + 1,
		timeInfo.tm_mday,
		timeInfo.tm_hour,
		timeInfo.tm_min,
		timeInfo.tm_sec,
		logLevelToString(level));

	vfprintf(file, fmt, args);
	fputc('\n', file);
	fflush(file);
	unlockMutex(mutex);
}
void logMessage(
	Logger logger,
	LogLevel level,
	const char* fmt,
	...)
{
	assert(fmt != NULL);

	va_list args;
	va_start(args, fmt);

	logVaMessage(
		logger,
		level,
		fmt,
		args);

	va_end(args);
}

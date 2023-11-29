// Copyright 2021-2023 Nikita Fediuchin. All rights reserved.
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
#include "mpio/os.h"
#include "mpio/file.h"
#include "mpio/directory.h"
#include "mpmt/sync.h"
#include "mpmt/thread.h"

#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#if _WIN32
#define ANSI_NAME_COLOR ""
#define ANSI_RESET_COLOR ""
#else
#define ANSI_NAME_COLOR "\e[0;90m"
#define ANSI_RESET_COLOR "\e[0m"
#endif

// TODO: use ENABLE_VIRTUAL_TERMINAL_PROCESSING on windows

struct Logger_T
{
	char* directoryPath;
	char* filePath;
	Mutex mutex;
	FILE* logFile;
	Thread rotationThread;
	double rotationTime;
	LogLevel level;
	bool logToStdout;
};

inline static char* createLogFilePath(
	const char* directoryPath, bool useRotation)
{
	assert(directoryPath);

	const char* fileName;
	int fileNameLength;

	if (useRotation)
	{
		time_t rawTime;
		time(&rawTime);

#if __linux__ || __APPLE__
		struct tm timeInfo = *localtime(&rawTime);
#elif _WIN32
		struct tm timeInfo;
		if (gmtime_s(&timeInfo, &rawTime) != 0) abort();
#else
#error Unknown operating system
#endif
		char nameBuffer[32];
		fileNameLength = snprintf(nameBuffer, 32,
			"log_%d-%02d-%02d_%02d-%02d-%02d.txt",
			timeInfo.tm_year + 1900, timeInfo.tm_mon + 1,
			timeInfo.tm_mday, timeInfo.tm_hour,
			timeInfo.tm_min, timeInfo.tm_sec);
		if (fileNameLength <= 0) return NULL;
		fileName = nameBuffer;
	}
	else
	{
		fileName = SOLO_LOG_FILE_NAME;
		fileNameLength = 7;
	}

	size_t directoryPathLength = strlen(directoryPath);

	char* filePath = malloc((2 +
		directoryPathLength + fileNameLength) * sizeof(char));
	if (!filePath) return NULL;

	memcpy(filePath, directoryPath, directoryPathLength * sizeof(char));
	filePath[directoryPathLength] = '/';
	memcpy(filePath + directoryPathLength + 1,
		fileName, fileNameLength * sizeof(char));
	filePath[directoryPathLength + 1 + fileNameLength] = '\0';
	return filePath;
}
inline static void compressLogFile(
	Logger logger,
	const char* filePath)
{
	assert(logger);
	assert(filePath);

	size_t filePathLength = strlen(filePath);
	size_t bufferSize = filePathLength * 2 + 32;
	char* buffer = malloc(bufferSize * sizeof(char));

	if (!buffer)
	{
		logMessage(logger, ERROR_LOG_LEVEL,
			"Failed to allocate a log file zip string.");
		return;
	}

	int count = snprintf(buffer, bufferSize,
		"tar -czf %.*s.tar.gz %.*s",
		(int)filePathLength, filePath,
		(int)filePathLength, filePath);

	if (count <= 0)
	{
		logMessage(logger, ERROR_LOG_LEVEL,
			"Failed to write log file zip string.");
		free(buffer);
		return;
	}

	int result = system(buffer);
	free(buffer);

	if (result != 0)
	{
		logMessage(logger, ERROR_LOG_LEVEL,
			"Failed to zip log file.");
		return;
	}

	remove(filePath);
}
static void onRotationUpdate(void* argument)
{
	assert(argument);
	Logger logger = (Logger)argument;
	Mutex mutex = logger->mutex;
	const char* directoryPath = logger->directoryPath;
	double timeDelay = getCurrentClock() + logger->rotationTime;

	while (logger->rotationTime > 0.0)
	{
		double currentTime = getCurrentClock();

		if (currentTime > timeDelay)
		{
			lockMutex(mutex);

			char* newFilePath = createLogFilePath(directoryPath, true);
			if (!newFilePath)
			{
				unlockMutex(mutex);
				logMessage(logger, ERROR_LOG_LEVEL,
					"Failed to allocate a new log file path string.");
				return;
			}

			FILE* newLogFile = openFile(newFilePath, "a");
			if (!newLogFile)
			{
				unlockMutex(mutex);
				logMessage(logger, ERROR_LOG_LEVEL,
					"Failed to open a new log file.");
				free(newFilePath);
				return;
			}

			char* oldFilePath = logger->filePath;
			logger->filePath = newFilePath;
			closeFile(logger->logFile);
			logger->logFile = newLogFile;
			timeDelay = currentTime + logger->rotationTime;
			unlockMutex(mutex);

			compressLogFile(logger, oldFilePath);
			free(oldFilePath);
		}

		sleepThread(0.001);
	}

	lockMutex(mutex);
	closeFile(logger->logFile);
	logger->logFile = NULL;
	compressLogFile(logger, logger->filePath);
	unlockMutex(mutex);
}
LogyResult createLogger(
	const char* _directoryPath, LogLevel level,
	bool logToStdout, double rotationTime,
	bool isDataDirectory, Logger* logger)
{
	assert(_directoryPath);
	assert(level < LOG_LEVEL_COUNT);
	assert(rotationTime >= 0.0);
	assert(logger);

	Logger loggerInstance = calloc(1, sizeof(Logger_T));
	if (!loggerInstance) return FAILED_TO_ALLOCATE_LOGY_RESULT;

	loggerInstance->rotationTime = rotationTime;
	loggerInstance->level = level;
	loggerInstance->logToStdout = logToStdout;

	size_t directoryPathLength = strlen(_directoryPath);

	assert(directoryPathLength == 0 || (directoryPathLength > 0 &&
		_directoryPath[directoryPathLength - 1] != '/' &&
		_directoryPath[directoryPathLength - 1] != '\\'));

	char* directoryPath;

	if (isDataDirectory)
	{
		const char* dataDirectory = getDataDirectory(false);
		if (!dataDirectory)
		{
			destroyLogger(loggerInstance);
			return FAILED_TO_GET_DIRECTORY_LOGY_RESULT;
		}

		size_t dataDirectoryPathLength = strlen(dataDirectory);
		size_t pathLength = dataDirectoryPathLength + directoryPathLength + 2;

		directoryPath = malloc(pathLength * sizeof(char));
		if (!directoryPath)
		{
			destroyLogger(loggerInstance);
			return FAILED_TO_ALLOCATE_LOGY_RESULT;
		}

		loggerInstance->directoryPath = directoryPath;

		memcpy(directoryPath, dataDirectory, dataDirectoryPathLength * sizeof(char));
		directoryPath[dataDirectoryPathLength] = '/';
		memcpy(directoryPath + dataDirectoryPathLength + 1, _directoryPath,
			directoryPathLength * sizeof(char));
		directoryPath[dataDirectoryPathLength + directoryPathLength + 1] = '\0';
	}
	else
	{
		size_t pathLength = directoryPathLength + 1;

		directoryPath = malloc(pathLength * sizeof(char));
		if (!directoryPath)
		{
			destroyLogger(loggerInstance);
			return FAILED_TO_ALLOCATE_LOGY_RESULT;
		}

		loggerInstance->directoryPath = directoryPath;

		memcpy(directoryPath, _directoryPath, directoryPathLength * sizeof(char));
		directoryPath[directoryPathLength] = '\0';
	}

	createDirectory(directoryPath);

	char* filePath = createLogFilePath(directoryPath, rotationTime > 0.0);
	if (!filePath)
	{
		destroyLogger(loggerInstance);
		return FAILED_TO_ALLOCATE_LOGY_RESULT;
	}

	loggerInstance->filePath = filePath;

	Mutex mutex = createMutex();
	if (!mutex)
	{
		destroyLogger(loggerInstance);
		return FAILED_TO_ALLOCATE_LOGY_RESULT;
	}

	loggerInstance->mutex = mutex;

	FILE* logFile = openFile(filePath, "w");
	if (!logFile)
	{
		destroyLogger(loggerInstance);
		return FAILED_TO_OPEN_FILE_LOGY_RESULT;
	}

	loggerInstance->logFile = logFile;

	if (rotationTime > 0.0)
	{
		Thread rotationThread = createThread(onRotationUpdate, loggerInstance);
		if (!rotationThread)
		{
			destroyLogger(loggerInstance);
			return FAILED_TO_ALLOCATE_LOGY_RESULT;
		}

		loggerInstance->rotationThread = rotationThread;
	}
	else
	{
		loggerInstance->rotationThread = NULL;
	}

	*logger = loggerInstance;
	return SUCCESS_LOGY_RESULT;
}
void destroyLogger(Logger logger)
{
	if (!logger) return;

	Thread rotationThread = logger->rotationThread;
	if (rotationThread)
	{
		logger->rotationTime = 0.0;
		joinThread(rotationThread);
		destroyThread(rotationThread);
	}

	if (logger->logFile) closeFile(logger->logFile);

	destroyMutex(logger->mutex);
	free(logger->filePath);
	free(logger->directoryPath);
	free(logger);
}

const char* getLoggerDirectoryPath(Logger logger)
{
	assert(logger);
	return logger->directoryPath;
}
const char* getLoggerFilePath(Logger logger)
{
	assert(logger);
	Mutex mutex = logger->mutex;
	lockMutex(mutex);
	const char* filePath = logger->filePath;
	unlockMutex(mutex);
	return filePath;
}
double getLoggerRotationTime(Logger logger)
{
	assert(logger);
	return logger->rotationTime;
}

LogLevel getLoggerLevel(Logger logger)
{
	assert(logger);
	Mutex mutex = logger->mutex;
	lockMutex(mutex);
	LogLevel level = logger->level;
	unlockMutex(mutex);
	return level;
}
void setLoggerLevel(Logger logger, LogLevel level)
{
	assert(logger);
	assert(level <= ALL_LOG_LEVEL);
	Mutex mutex = logger->mutex;
	lockMutex(mutex);
	logger->level = level;
	unlockMutex(mutex);
}

bool getLoggerLogToStdout(Logger logger)
{
	assert(logger);
	Mutex mutex = logger->mutex;
	lockMutex(mutex);
	bool logToStdout = logger->logToStdout;
	unlockMutex(mutex);
	return logToStdout;
}
void setLoggerLogToStdout(Logger logger, bool logToStdout)
{
	assert(logger);
	Mutex mutex = logger->mutex;
	lockMutex(mutex);
	logger->logToStdout = logToStdout;
	unlockMutex(mutex);
}

void logVaMessage(Logger logger, LogLevel level, const char* fmt, va_list args)
{
	assert(logger);
	assert(level < ALL_LOG_LEVEL);
	assert(fmt);

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
	struct tm timeInfo = *localtime(&rawTime);
#elif _WIN32
	struct tm timeInfo;
	if (gmtime_s(&timeInfo, &rawTime) != 0) abort();
#else
#error Unknown operating system
#endif

	double clock = getCurrentClock();
	int milliseconds = (int)((clock - floor(clock)) * 1000.0);

	char threadName[16];
	getThreadName(threadName, 16);

	if (logger->logToStdout)
	{
#if _WIN32
		const char* color = "";
#else
		const char* color;
		switch (level)
		{
		default: color = "\e[0;37m"; break;
		case FATAL_LOG_LEVEL: color = "\e[0;31m"; break;
		case ERROR_LOG_LEVEL: color = "\e[0;91m"; break;
		case WARN_LOG_LEVEL: color = "\e[0;93m"; break;
		case DEBUG_LOG_LEVEL: color = "\e[0;92m"; break;
		case TRACE_LOG_LEVEL: color = "\e[0;94m"; break;
		}
#endif

		printf("[" ANSI_NAME_COLOR "%d-%02d-%02d %02d:%02d:%02d.%03d"
			ANSI_RESET_COLOR "] [" ANSI_NAME_COLOR "%s"
			ANSI_RESET_COLOR "] [%s%s" ANSI_RESET_COLOR "]: ",
			timeInfo.tm_year + 1900, timeInfo.tm_mon + 1,
			timeInfo.tm_mday, timeInfo.tm_hour,
			timeInfo.tm_min, timeInfo.tm_sec, milliseconds,
			threadName, color, logLevelToString(level));

		va_list stdArgs;
		va_copy(stdArgs, args);
		vfprintf(stdout, fmt, stdArgs);
		va_end(stdArgs);

		fputc('\n', stdout);
		fflush(stdout);
	}

	FILE* logFile = logger->logFile;

	fprintf(logFile, "[%d-%02d-%02d %02d:%02d:%02d.%03d] [%s] [%s]: ",
		timeInfo.tm_year + 1900, timeInfo.tm_mon + 1,
		timeInfo.tm_mday, timeInfo.tm_hour,
		timeInfo.tm_min, timeInfo.tm_sec, milliseconds,
		threadName, logLevelToString(level));

	vfprintf(logFile, fmt, args);
	fputc('\n', logFile);
	fflush(logFile);
	unlockMutex(mutex);
}
void logMessage(Logger logger, LogLevel level, const char* fmt, ...)
{
	assert(logger);
	assert(level < ALL_LOG_LEVEL);
	assert(fmt);
	va_list args;
	va_start(args, fmt);
	logVaMessage(logger, level, fmt, args);
	va_end(args);
}

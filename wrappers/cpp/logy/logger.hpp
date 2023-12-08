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

#pragma once
#include <string>
#include <exception>
#include <filesystem>
#include <string_view>

extern "C"
{
#include "logy/logger.h"
}

namespace logy
{
	using namespace std;

	/*
	 * Logger instance handle.
	 */
	class Logger final
	{
	private:
		Logger_T* instance = nullptr;
	public:
		/*
		 * Create a new file pack reader.
		 */
		Logger() = default;

		Logger(const Logger&) = delete;
		Logger(Logger&& r) noexcept : instance(
			std::exchange(r.instance, nullptr)) { }

		Logger& operator=(Logger&) = delete;
		Logger& operator=(Logger&& r) noexcept
		{
			instance = std::exchange(r.instance, nullptr);
			return *this;
		}

		/*
		 * Create a new logger instance.
		 * Returns operation logy result.
		 *
		 * directoryPath - logs directory path string.
		 * level - logging level, inclusive.
		 * logToConsole - also log to stdout.
		 * rotationTime - log rotation delay time or 0. (seconds)
		 * isDataDirectory - write to data directory.
		 * logger - pointer to the logger instance.
		 */
		Logger(const filesystem::path& directoryPath,
			LogLevel level = ALL_LOG_LEVEL, bool logToStdout = true,
			double rotationTime = 0.0, bool isDataDirectory = true)
		{
			auto string = directoryPath.generic_string();
			auto result = createLogger(string.c_str(), level,
				logToStdout, rotationTime, isDataDirectory, &instance);
			if (result != SUCCESS_LOGY_RESULT)
			{
				throw runtime_error(logyResultToString(result) +
					("(path: " + directoryPath.generic_string() + ")"));
			}
		}
		/*
		 * Destroys logger instance.
		 */
		~Logger() { destroyLogger(instance); }

		/*
		 * Returns logger directory path string. (MT-Safe)
		 * logger - logger instance.
		 */
		string_view getDirectoryPath() const noexcept
		{
			return getLoggerDirectoryPath(instance);
		}
		/*
		 * Returns logger file path string. (MT-Safe)
		 * logger - logger instance.
		 */
		string_view getFilePath() const noexcept
		{
			return getLoggerFilePath(instance);
		}
		/*
		 * Returns current logger rotation delay time in seconds. (MT-Safe)
		 * logger - logger instance.
		 */
		double getRotationTime() const noexcept
		{
			return getLoggerRotationTime(instance);
		}

		/*
		 * Returns current logger logging level. (MT-Safe)
		 * logger - logger instance.
		 */
		LogLevel getLevel() const noexcept
		{
			return getLoggerLevel(instance);
		}
		/*
		 * Set logger logging level. (MT-Safe)
		 * Log only message <= log level.
		 *
		 * logger - logger instance.
		 * level - message logging level.
		 */
		void setLevel(LogLevel level) noexcept
		{
			setLoggerLevel(instance, level);
		}

		/*
		 * Returns current logger log to stdout state. (MT-Safe)
		 * logger - logger instance.
		 */
		bool geLogToStdout()
		{
			return getLoggerLogToStdout(instance);
		}
		/*
		 * Set log messages to stdout. (MT-Safe)
		 *
		 * logger - logger instance.
		 * logToStdout - value.
		 */
		void setLogToStdout(bool value)
		{
			setLoggerLogToStdout(instance, value);
		}

		/*
		 * Log message to the log. (MT-Safe)
		 *
		 * logger - logger instance.
		 * level - message logging level.
		 * fmt - formatted message.
		 * args - message arguments.
		 */
		void log(LogLevel level, const char* fmt, va_list args)
		{
			va_list stdArgs;
			va_copy(stdArgs, args);
			logVaMessage(instance, level, fmt, stdArgs);
			va_end(stdArgs);
		}
		/*
		 * Log message to the log. (MT-Safe)
		 *
		 * logger - logger instance.
		 * level - message logging level.
		 * fmt - formatted message.
		 * ... - message arguments.
		 */
		void log(LogLevel level, const char* fmt, ...)
		{
			va_list args;
			va_start(args, fmt);
			logVaMessage(instance, level, fmt, args);
			va_end(args);
		}
	};

} // conf
// Copyright 2021-2024 Nikita Fediuchin. All rights reserved.
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
 * @brief Message logger.
 * @details See the @ref logger.h
 **********************************************************************************************************************/

#pragma once
#include <string>
#include <utility>
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

	/**
	 * @brief Logger instance handle.
	 * @details See the @ref logger.h
	 */
	class Logger final
	{
	private:
		Logger_T* instance = nullptr;
	public:
		/**
		 * @brief Creates a new logger without stream.
		 */
		Logger() = default;

		Logger(const Logger&) = delete;
		Logger(Logger&& r) noexcept : instance(std::exchange(r.instance, nullptr)) { }

		Logger& operator=(Logger&) = delete;
		Logger& operator=(Logger&& r) noexcept
		{
			instance = std::exchange(r.instance, nullptr);
			return *this;
		}

		/*******************************************************************************************************************
		 * @brief Creates a new logger instance.
		 * @details See the @ref createLogger().
		 *
		 * @param[in] directoryPath logs directory path string
		 * @param level logging level, inclusive
		 * @param logToStdout duplicate messages to the stdout
		 * @param rotationTime log rotation delay time or 0 (in seconds)
		 * @param isAppDataDirectory write to app data directory
		 * 
		 * @throw runtime_error with a @ref LogyResult string on failure.
		 */
		Logger(const filesystem::path& directoryPath, LogLevel level = ALL_LOG_LEVEL,
			bool logToStdout = true, double rotationTime = 0.0, bool isAppDataDirectory = true)
		{
			auto string = directoryPath.generic_string();
			auto result = createLogger(string.c_str(), level,
				logToStdout, rotationTime, isAppDataDirectory, &instance);
			if (result != SUCCESS_LOGY_RESULT)
			{
				throw runtime_error(logyResultToString(result) +
					("(path: " + directoryPath.generic_string() + ")"));
			}
		}

		/**
		 * @brief Destroys logger stream.
		 * @details See the @ref destroyLogger().
		 */
		~Logger() { destroyLogger(instance); }

		/**
		 * @brief Opens a new logger stream.
		 * @details See the @ref createLogger().
		 *
		 * @param[in] directoryPath logs directory path string
		 * @param level logging level, inclusive
		 * @param logToStdout duplicate messages to the stdout
		 * @param rotationTime log rotation delay time or 0 (in seconds)
		 * @param isAppDataDirectory write to app data directory
		 * 
		 * @throw runtime_error with a @ref LogyResult string on failure.
		 */
		void open(const filesystem::path& directoryPath, LogLevel level = ALL_LOG_LEVEL,
			bool logToStdout = true, double rotationTime = 0.0, bool isAppDataDirectory = true)
		{
			destroyLogger(instance);
			auto string = directoryPath.generic_string();
			auto result = createLogger(string.c_str(), level,
				logToStdout, rotationTime, isAppDataDirectory, &instance);
			if (result != SUCCESS_LOGY_RESULT)
			{
				throw runtime_error(logyResultToString(result) +
					("(path: " + directoryPath.generic_string() + ")"));
			}
		}

		/**
		 * @brief Closes the current logger stream.
		 * @details See the @ref destroyLogger().
		 */
		void close() noexcept
		{
			destroyLogger(instance);
			instance = nullptr;
		}

		/**
		 * @brief Returns true if logger stream is open.
		 * @details See the @ref createLogger().
		 */
		bool isOpen() const noexcept { return instance; }

		/*******************************************************************************************************************
		 * @brief Returns logger directory path string. (MT-Safe)
		 * @details See the @ref getLoggerDirectoryPath().
		 */
		string_view getDirectoryPath() const noexcept
		{
			return getLoggerDirectoryPath(instance);
		}

		/**
		 * @brief Returns logger file path string. (MT-Safe)
		 * @details See the @ref getLoggerFilePath().
		 */
		string_view getFilePath() const noexcept
		{
			return getLoggerFilePath(instance);
		}

		/**
		 * @brief Returns current logger rotation delay time in seconds. (MT-Safe)
		 * @details See the @ref getLoggerRotationTime().
		 */
		double getRotationTime() const noexcept
		{
			return getLoggerRotationTime(instance);
		}

		/**
		 * @brief Returns current logger logging level. (MT-Safe)
		 * @details See the @ref getLoggerLevel().
		 */
		LogLevel getLevel() const noexcept
		{
			return getLoggerLevel(instance);
		}

		/**
		 * @brief Set logger logging level. (MT-Safe)
		 * @details See the @ref setLoggerLevel().
		 * @param level message logging level
		 */
		void setLevel(LogLevel level) noexcept
		{
			setLoggerLevel(instance, level);
		}

		/**
		 * @brief Returns current logger log to stdout state. (MT-Safe)
		 * @details See the @ref getLoggerLogToStdout().
		 */
		bool getLogToStdout() noexcept
		{
			return getLoggerLogToStdout(instance);
		}

		/**
		 * @brief Sets log messages to stdout. (MT-Safe)
		 * @details See the @ref setLoggerLogToStdout().
		 * @param value logToStdout value
		 */
		void setLogToStdout(bool value) noexcept
		{
			setLoggerLogToStdout(instance, value);
		}

		/**
		 * @brief Logs message to the log. (MT-Safe)
		 * @details See the @ref logVaMessage().
		 *
		 * @param level message logging level
		 * @param[in] fmt formatted message
		 * @param args message arguments
		 */
		void log(LogLevel level, const char* fmt, va_list args) noexcept
		{
			va_list stdArgs;
			va_copy(stdArgs, args);
			logVaMessage(instance, level, fmt, stdArgs);
			va_end(stdArgs);
		}

		/**
		 * @brief Logs message to the log. (MT-Safe)
		 * @details See the @ref logVaMessage().
		 *
		 * @param level message logging level
		 * @param[in] fmt formatted message
		 * @param ... message arguments
		 */
		void log(LogLevel level, const char* fmt, ...) noexcept
		{
			va_list args;
			va_start(args, fmt);
			logVaMessage(instance, level, fmt, args);
			va_end(args);
		}
	};
} // logy
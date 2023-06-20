#pragma once

#include <spdlog/spdlog.h>

namespace pbrt
{
	class Logger final
	{
	public:
		enum class LogLevel : uint8_t
		{
			debug,
			info,
			warn,
			error,
			fatal
		};

	public:
		Logger();
		~Logger();

		template<typename... TARGS>
		void log(LogLevel level, TARGS&&... args)
		{
			switch (level)
			{
				case LogLevel::debug:
					m_logger->debug(std::forward<TARGS>(args)...);
					break;
				case LogLevel::info:
					m_logger->info(std::forward<TARGS>(args)...);
					break;
				case LogLevel::warn:
					m_logger->warn(std::forward<TARGS>(args)...);
					break;
				case LogLevel::error:
					m_logger->error(std::forward<TARGS>(args)...);
					break;
				case LogLevel::fatal:
					m_logger->critical(std::forward<TARGS>(args)...);
					break;
				default:
					break;
			}
		}

		template<typename... TARGS>
		void fatalCallback(TARGS... args)
		{
			const std::string format_str = fmt::format(std::forward<TARGS>(args)...);
			throw std::runtime_error(format_str);
		}

	private:
		std::shared_ptr<spdlog::logger> m_logger;
	};

	extern std::shared_ptr<Logger> log_system;

#define LOG_HELPER(LOG_LEVEL, ...) \
	log_system->log(LOG_LEVEL, "[" + std::string(__FUNCTION__) + "] " + __VA_ARGS__)

#define Debug(...) LOG_HELPER(Logger::LogLevel::debug, __VA_ARGS__)

#define Info(...) LOG_HELPER(Logger::LogLevel::info, __VA_ARGS__)

#define Warn(...) LOG_HELPER(Logger::LogLevel::warn, __VA_ARGS__)

#define Error(...) LOG_HELPER(Logger::LogLevel::error, __VA_ARGS__)

#define Fatal(...) LOG_HELPER(Logger::LogLevel::fatal, __VA_ARGS__)

}


//
// Created by AmazingBuff on 2023/6/20.
//

#include "log.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/async.h>

namespace pbrt
{
	Logger::Logger()
	{
		auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		console_sink->set_level(spdlog::level::trace);
		console_sink->set_pattern("[%^%l%$] %v");

		const spdlog::sinks_init_list sink_list = {console_sink};

		spdlog::init_thread_pool(8192, 1);

		m_logger = std::make_shared<spdlog::async_logger>("muggle_logger",
														  sink_list.begin(),
														  sink_list.end(),
														  spdlog::thread_pool(),
														  spdlog::async_overflow_policy::block);

		m_logger->set_level(spdlog::level::trace);

		spdlog::register_logger(m_logger);
	}

	Logger::~Logger()
	{
		m_logger->flush();
		spdlog::drop_all();
	}

}

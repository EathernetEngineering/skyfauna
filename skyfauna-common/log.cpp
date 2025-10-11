// SPDX-License-Identifier: MIT
// Copyright (C) 2025 Chloe Eather

#include <skyfauna/common/log.h>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace skyfauna::common {
std::shared_ptr<spdlog::logger> Logger::s_Logger = nullptr;

void Logger::Init()
{
	if (s_Logger) {
		s_Logger->warn("Logger::Init() called more than once");
		return;
	}

	auto sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
	sink->set_pattern("[%T] [%l] %^%n: %v%$");

	s_Logger = std::make_shared<spdlog::logger>("sfc", sink);
	s_Logger->set_level(spdlog::level::trace);
	s_Logger->flush_on(spdlog::level::trace);

	spdlog::register_logger(s_Logger);
}

void Logger::Shutdown() noexcept
{
	spdlog::drop(s_Logger->name());
	s_Logger = nullptr;
}
}


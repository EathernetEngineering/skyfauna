// SPDX-License-Identifier: MIT
// Copyright (C) 2025 Chloe Eather

#ifndef SKYFAUNA_COMMON_LOGGER_H_
#define SKYFAUNA_COMMON_LOGGER_H_

#include <spdlog/spdlog.h>

namespace skyfauna::common {
class Logger {
public:
	static void Init();
	static void Shutdown() noexcept;

	static auto GetLogger() { return s_Logger; }

	static void SetLogLevel(spdlog::level::level_enum level) {
		s_Logger->set_level(level);
		s_Logger->flush_on(level);
	}

private:
	static std::shared_ptr<spdlog::logger> s_Logger;
};
}

#define SF_TRACE(msg, ...) \
	::skyfauna::common::Logger::GetLogger()->trace(msg __VA_OPT__(,) __VA_ARGS__)
#define SF_DEBUG(msg, ...) \
	::skyfauna::common::Logger::GetLogger()->debug(msg __VA_OPT__(,) __VA_ARGS__)
#define SF_INFO(msg, ...) \
	::skyfauna::common::Logger::GetLogger()->info(msg __VA_OPT__(,) __VA_ARGS__)
#define SF_WARN(msg, ...) \
	::skyfauna::common::Logger::GetLogger()->warn(msg __VA_OPT__(,) __VA_ARGS__)
#define SF_ERROR(msg, ...) \
	::skyfauna::common::Logger::GetLogger()->error(msg __VA_OPT__(,) __VA_ARGS__)
#define SF_CRITICAL(msg, ...) \
	::skyfauna::common::Logger::GetLogger()->critical(msg __VA_OPT__(,) __VA_ARGS__)

#endif


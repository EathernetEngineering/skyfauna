// SPDX-License-Identifier: MIT
// Copyright (C) 2025 Chloe Eather

#include <filesystem>
#include <cstdio>

namespace skyfauna::common {
[[noreturn]] void assert_fail(const char *cond, const char *file, int line)
{
	std::fprintf(stderr, "Assertion (%s) failed\n", cond);
	std::fprintf(stderr, "%s:%d\n", std::filesystem::path(file).filename().c_str(), line);
	std::abort();
}
}

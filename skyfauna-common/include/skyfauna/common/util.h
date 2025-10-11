// SPDX-License-Identifier: MIT
// Copyright (C) 2025 Chloe Eather

#ifndef SKYFAUNA_COMMON_UTIL_H_
#define SKYFAUNA_COMMON_UTIL_H_

#include <skyfauna/common/config.h>

#include <format>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace skyfauna::common {
template<typename... Args>
inline void assert_message(std::string_view msg, const Args&... args) {
	std::fprintf(stderr, std::format(msg, std::forward<Args>(args)...));
}
[[noreturn]] void assert_fail(const char *cond, const char *file, int line);
}

#define SF_VERIFY(cond, ...) do { \
	if (!(cond)) { \
		__VA_OPT__(::skyfauna::common::assert_message(__VA_ARGS__)); \
		::skyfauna::common::assert_fail(#cond, __FILE__, __LINE__);\
	} \
} while(0)
#ifndef NDEBUG
#ifdef SKYFAUNA_ENABLE_ASSERTIONS
# define SF_ASSERT(cond, ...) do { \
	if (!(cond)) { \
		__VA_OPT__(::skyfauna::common::assert_message(__VA_ARGS__)); \
		::skyfauna::common::assert_fail(#cond, __FILE__, __LINE__);\
	} \
} while(0)
#endif
#endif

template<typename charT,
	typename Tr = std::char_traits<charT>,
	typename Alloc = std::allocator<charT>>
static constexpr std::basic_string<charT, Tr, Alloc> XSpaces(typename Alloc::size_type n) {
	std::basic_string<charT, Tr, Alloc> str;
	str.reserve(n);
	for (int i = 0; i < n; i++) {
		str.push_back(' ');
	}
	return str;
}

template<typename charT>
auto SplitDelim(std::basic_string_view<charT> str, charT delim) {
	std::vector<std::basic_string_view<charT>> split;

	auto begin = str.begin();
	for (auto c = str.begin(); c != str.end(); ++c) {
		if (*c == delim) {
			split.emplace_back(begin, c);

			// begin of next line doesn't include the delimiter
			begin = std::next(c);
		}

	}
	return split;
}

namespace skyfauna {
constexpr std::size_t Strlen(const char *str)
{
	std::size_t l = 0;
	while (str[l] != '\0') ++l;
	return l;
}

inline constexpr std::size_t Strlen(std::string_view str)
{
	return str.size();
}

template<typename T, std::size_t N>
constexpr std::size_t LongestStrlen(std::span<T, N> arr)
{
	std::size_t m = 0;
	std::for_each(arr.begin(), arr.end(),
			   [&m](T str){ m = std::max(m, Strlen(str)); });
	return m;
}

template<typename T, std::size_t N>
constexpr std::size_t LongestStrlen(const std::array<T, N>& arr)
{
	std::span<const T, N> s(arr);
	return LongestStrlen(s);
}

template<typename Key>
constexpr std::size_t Hash(Key) = delete;

template<>
constexpr std::size_t Hash<const char *>(const char *str)
{
	std::size_t h = 1469598103934665603ull, i = 0;
	while (str[i] != '\0') {
		h = (h ^ str[i]) * 1099511628211ull;
		++i;
	}
	return h;
}

template<>
constexpr std::size_t Hash<std::string_view>(std::string_view view)
{
	return Hash(view.data());
}

template<typename T, size_t N>
requires (N != std::dynamic_extent)
constexpr std::array<std::size_t, N> HashStrings(std::span<T, N> inArr)
{
	std::array<std::size_t, N> outArr;
	static_assert(outArr.size() == inArr.size());
	std::transform(inArr.begin(), inArr.end(), outArr.begin(),
				[](const T old) -> std::size_t { return Hash(old); });
	return outArr;
}

template<typename T, size_t N>
inline constexpr std::array<std::size_t, N> HashStrings(const std::array<T, N>& inArr)
{
	std::span<const T, N> s(inArr);
	return HashStrings(s);
}
}

#endif


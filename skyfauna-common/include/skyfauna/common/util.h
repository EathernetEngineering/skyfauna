// SPDX-License-Identifier: MIT
// Copyright (C) 2025 Chloe Eather

#ifndef SKYFAUNA_COMMON_UTIL_H_
#define SKYFAUNA_COMMON_UTIL_H_

#include <skyfauna/common/config.h>

#include <algorithm>
#include <iostream>
#include <iterator>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include <spdlog/fmt/fmt.h>

namespace skyfauna::common {
template<typename... Args>
inline void assert_message(std::string_view msg, const Args&... args) {
	std::cerr << fmt::format(msg, std::forward<Args>(args)...);
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

#define BIT(x) (1llu << x)

namespace skyfauna {
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

constexpr std::size_t Strlen(const char *str)
{
	std::size_t l = 0;
	while (str[l] != '\0') ++l;
	return l;
}

constexpr std::size_t Strlen(const std::string_view str)
{
	return str.size();
}

constexpr std::size_t Strlen(const std::string& str)
{
	return str.size();
}

constexpr std::size_t Strcmp(const char *lhs, const char *rhs)
{
	while (*lhs != '\0' && *lhs == *rhs) lhs++, rhs++;
	return lhs - rhs;
}

constexpr std::size_t Strcmp(const char *lhs, const char *rhs, std::size_t n)
{
	while (*lhs != '\0' && *lhs == *rhs && n > 0) ++lhs, ++rhs, --n;
	return lhs - rhs;
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

template<typename InputIt, typename End>
requires std::input_iterator<InputIt> &&
	std::sentinel_for<End, InputIt>
constexpr std::size_t HashStr(InputIt it, End end)
{
	std::size_t h = 1469598103934665603ull;
	for (; it != end; ++it)
		h = (h ^ *it) * 1099511628211ull;
	return h;
}

template<typename Key>
struct Hash
{
	std::size_t operator()(Key) = delete;
};

template<>
struct Hash<std::string_view>
{
	constexpr std::size_t operator()(const std::string_view view) {
		return HashStr(view.begin(), view.end());
	}
};

template<>
struct Hash<const char *>
{
	constexpr std::size_t operator()(const char *str) {
		return HashStr(str, str + Strlen(str));
	}
};

template<>
struct Hash<std::string>
{
	constexpr std::size_t operator()(const std::string& str) {
		return HashStr(str.begin(), str.end());
	}
};

template<typename T, typename = void>
struct IsHashable : std::false_type {};

template<typename T>
struct IsHashable<T, std::void_t<decltype(std::declval<Hash<
	std::remove_cvref_t<T>>>()(
		std::declval<const std::remove_reference_t<T>&>()))>>
 : std::true_type {};

template<typename T>
inline constexpr bool IsHashableV = IsHashable<T>::value;

template<typename InputIt, typename OutputIt>
requires std::input_iterator<InputIt> &&
	std::output_iterator<OutputIt, std::size_t>
constexpr void HashStringsArr(InputIt begin, InputIt end, OutputIt out)
{
	std::transform(begin, end, out,
				[](const std::iter_reference_t<InputIt> old) ->
					std::size_t { return Hash<std::decay_t<decltype(old)>>{}(old); });
}

template<typename T, size_t N>
requires (N != std::dynamic_extent)
constexpr std::array<std::size_t, N> HashStringsArr(std::span<const T, N> in)
{
	std::array<std::size_t, N> out;
	HashStringsArr(in.begin(), in.end(), out.begin());
	return out;
}

template<typename T, size_t N>
constexpr std::array<std::size_t, N> HashStringsArr(const std::array<T, N>& in)
{
	std::array<std::size_t, N> out;
	HashStringsArr(in.begin(), in.end(), out.begin());
	return out;
}


template<typename InputIt>
requires std::input_iterator<InputIt>
std::vector<std::size_t> HashStrings(InputIt begin, InputIt end) {
	std::vector<std::size_t> outArr;
	outArr.resize(std::ranges::distance(begin, end));
	std::transform(begin, end, outArr.begin(),
				[](const std::iter_reference_t<InputIt> str) ->
					std::size_t { return Hash<std::decay_t<decltype(str)>>{}(str); });
	return outArr;
}

template<typename T, std::size_t N = std::dynamic_extent>
std::vector<std::size_t> HashStrings(std::span<const T, N> span) {
	return HashStrings(span.begin(), span.end());
}

template<typename T, std::size_t N>
std::vector<std::size_t> HashStrings(const std::array<T, N>& arr) {
	return HashStrings(arr.begin(), arr.end());
}

template<typename T>
std::vector<std::size_t> HashStrings(const std::vector<T>& vec) {
	return HashStrings(vec.begin(), vec.end());
}

}

#endif


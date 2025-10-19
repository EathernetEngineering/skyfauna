// SPDX-License-Identifier: MIT
// Copyright (C) 2025 Chloe Eather

#ifndef SKYFAUNA_TEST_LITERALS_H_
#define SKYFAUNA_TEST_LITERALS_H_

#include <array>
#include <string_view>
#include <skyfauna/lex.h>

namespace skyfauna::tests {
inline constexpr std::size_t g_LanguageKeywordCount = 13;

extern const
std::array<std::string_view, g_LanguageKeywordCount> g_LanguageKeywords;

extern const std::string_view g_LexerTestCase;

inline constexpr std::size_t g_LexerExpextedTokenCount = 248;

extern const
std::array<skyfauna::Token, g_LexerExpextedTokenCount> g_LexerExpectedTokens;

extern const std::string_view g_PreprocessorValidCode;

inline constexpr std::size_t g_PreprocessorValidExpextedTokenCount = 11;

extern const
std::array<skyfauna::Token, g_PreprocessorValidExpextedTokenCount>
	g_PreprocessorValidExpextedTokens;
}

#endif


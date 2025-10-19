// SPDX-License-Identifier: MIT
// Copyright (C) 2025 Chloe Eather

#include <skyfauna/lex.h>
#include <skyfauna/common/log.h>

#include "test_literals.h"
#include "util.h"

#include <string_view>

#include <gtest/gtest.h>

using namespace skyfauna::tests;

class LexTest : public testing::Test {
protected:
	LexTest() {
		skyfauna::common::Logger::Init();
		skyfauna::common::Logger::SetLogLevel(spdlog::level::err);
	}
	~LexTest() {
		skyfauna::common::Logger::Shutdown();
	}
};

TEST_F(LexTest, Tokenize)
{

	skyfauna::Lexer l(std::vector<std::string_view>(g_LanguageKeywords.begin(), g_LanguageKeywords.end()), g_LexerTestCase);
	const std::vector<skyfauna::Token>& result =  l.Tokenize().GetTokens();
	std::cout << std::endl;

	ASSERT_TRUE(CompareTokens(g_LexerExpectedTokens, result));

}


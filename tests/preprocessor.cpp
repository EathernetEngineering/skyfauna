// SPDX-License-Identifier: MIT
// Copyright (C) 2025 Chloe Eather

#include <skyfauna/lex.h>
#include <skyfauna/preproc.h>
#include <skyfauna/common/log.h>

#include "test_literals.h"
#include "util.h"

#include <gtest/gtest.h>

using namespace skyfauna::tests;

class PreprocTest : public testing::Test {
protected:
	PreprocTest() {
		skyfauna::common::Logger::Init();
		skyfauna::common::Logger::SetLogLevel(spdlog::level::err);
	}
	~PreprocTest() {
		skyfauna::common::Logger::Shutdown();
	}
};

struct StreamDiagnostics {
	const std::vector<std::string>& msgs;
	const char delim;

	constexpr StreamDiagnostics(const std::vector<std::string>& diag, char delimiter = '\n')
	 : msgs(diag), delim(delimiter)
	{
	}

	friend std::ostream& operator<<(const std::ostream& os, const StreamDiagnostics& diag);
};

constexpr std::ostream& operator<<(std::ostream& os, const StreamDiagnostics& diag) {
		for (const auto& msg : diag.msgs) {
			os << msg << diag.delim;
		}
	return os;
}

TEST_F(PreprocTest, ValidDirectives)
{
	skyfauna::Lexer l(g_LanguageKeywords,
				   std::string(g_PreprocessorValidCode.begin(),
						g_PreprocessorValidCode.end()));
	;
	skyfauna::Preprocessor preproc(l.Tokenize().GetTokens());
	preproc.Preprocess();

	EXPECT_EQ(preproc.GetDiagnotics().size(), 0) << '\n' <<
		StreamDiagnostics(preproc.GetDiagnotics());
	const std::vector<skyfauna::Token>& tokens = preproc.GetOutput();

	EXPECT_TRUE(CompareTokens(tokens, g_PreprocessorValidExpextedTokens));
}


TEST_F(PreprocTest, ErrorEmissions)
{
}


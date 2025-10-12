// SPDX-License-Identifier: MIT
// Copyright (C) 2025 Chloe Eather

#include "gtest/gtest.h"
#include <skyfauna/lex.h>
#include <skyfauna/common/log.h>

#include <string_view>

#include <gtest/gtest.h>

const std::string_view g_TestCode =
	"// Line comment\n"
	"/* Block \n"
	"comment */\n"
	""
	"class Main {"
	"	public static main(string[]: args)(void) {"
	"		ptr: address = null;"
	"		print(args[0]);"
	"		print(args[0 + 1]);"
	"	}"
	"};";
std::array g_TestCodeExpextedTokens = std::to_array({
	skyfauna::Token(skyfauna::TTComment::LINE, "// Line comment"),
	skyfauna::Token(skyfauna::TTComment::BLOCK, "/* Block \ncomment */"),
	skyfauna::Token(skyfauna::TTIdentifier::KEYWORD, "class"),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "Main"),
	skyfauna::Token(skyfauna::TTPunctuator::DELIMITER, "{"),
	skyfauna::Token(skyfauna::TTIdentifier::KEYWORD, "public"),
	skyfauna::Token(skyfauna::TTIdentifier::KEYWORD, "static"),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "main"),
	skyfauna::Token(skyfauna::TTPunctuator::DELIMITER, "("),
	skyfauna::Token(skyfauna::TTIdentifier::KEYWORD, "string"),
	skyfauna::Token(skyfauna::TTPunctuator::DELIMITER, "["),
	skyfauna::Token(skyfauna::TTPunctuator::DELIMITER, "]"),
	skyfauna::Token(skyfauna::TTPunctuator::DELIMITER, ":"),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "args"),
	skyfauna::Token(skyfauna::TTPunctuator::DELIMITER, ")"),
	skyfauna::Token(skyfauna::TTPunctuator::DELIMITER, "("),
	skyfauna::Token(skyfauna::TTIdentifier::KEYWORD, "void"),
	skyfauna::Token(skyfauna::TTPunctuator::DELIMITER, ")"),
	skyfauna::Token(skyfauna::TTPunctuator::DELIMITER, "{"),
	skyfauna::Token(skyfauna::TTIdentifier::KEYWORD, "ptr"),
	skyfauna::Token(skyfauna::TTPunctuator::DELIMITER, ":"),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "address"),
	skyfauna::Token(skyfauna::TTPunctuator::OPERATOR, "="),
	skyfauna::Token(skyfauna::TTIdentifier::KEYWORD, "null"),
	skyfauna::Token(skyfauna::TTPunctuator::DELIMITER, ";"),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "print"),
	skyfauna::Token(skyfauna::TTPunctuator::DELIMITER, "("),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "args"),
	skyfauna::Token(skyfauna::TTPunctuator::DELIMITER, "["),
	skyfauna::Token(skyfauna::TTLiteral::INTEGER, "0"),
	skyfauna::Token(skyfauna::TTPunctuator::DELIMITER, "]"),
	skyfauna::Token(skyfauna::TTPunctuator::DELIMITER, ")"),
	skyfauna::Token(skyfauna::TTPunctuator::DELIMITER, ";"),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "print"),
	skyfauna::Token(skyfauna::TTPunctuator::DELIMITER, "("),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "args"),
	skyfauna::Token(skyfauna::TTPunctuator::DELIMITER, "["),
	skyfauna::Token(skyfauna::TTLiteral::INTEGER, "0"),
	skyfauna::Token(skyfauna::TTPunctuator::OPERATOR, "+"),
	skyfauna::Token(skyfauna::TTLiteral::INTEGER, "1"),
	skyfauna::Token(skyfauna::TTPunctuator::DELIMITER, "]"),
	skyfauna::Token(skyfauna::TTPunctuator::DELIMITER, ")"),
	skyfauna::Token(skyfauna::TTPunctuator::DELIMITER, ";"),
	skyfauna::Token(skyfauna::TTPunctuator::DELIMITER, "}"),
	skyfauna::Token(skyfauna::TTPunctuator::DELIMITER, "}"),
	skyfauna::Token(skyfauna::TTPunctuator::DELIMITER, ";"),
	skyfauna::Token(skyfauna::TTPunctuator::DELIMITER, "EOF"),
});

template<typename Lhs, typename Rhs>
testing::AssertionResult CompareTokenArray(const Lhs& lhs, const Rhs& rhs)
{
	if (lhs.size() != rhs.size())
		return testing::AssertionFailure() << "Amount of tokens different (" <<
			lhs.size() << " vs " << rhs.size() << ")";
	std::vector<std::string> errors;

	if (std::equal(lhs.begin(), lhs.end(), rhs.begin(),
				   [&errors](const skyfauna::Token& l, const skyfauna::Token& r){
					bool eq = true;
					if (l.type() != r.type()) {
						eq = false;
						errors.push_back(fmt::format("Token {} type not eq ({} vs {})", l.text(), l.type(), r.type()));
					}
					if (l.type() != r.type()) {
						eq = false;
						errors.push_back(fmt::format("Token {} subtype not eq ({} vs {})", l.text(), l.type(), r.type()));
					}
					if (strcmp(l.text().c_str(), r.text().c_str())) {
						eq = false;
						errors.push_back(fmt::format("Token {} text not eq ({})", l.text(), r.text()));
					}
					return eq;
				   })) {
		return testing::AssertionSuccess();
	}
	auto res = testing::AssertionFailure();
	for (auto& s : errors) {
		res << s << std::endl;
	}
	return res;
}

TEST(Lex, Tokenize)
{
	skyfauna::common::Logger::Init();

	skyfauna::Lexer l(g_TestCode.data());
	std::vector<skyfauna::Token> result =  l.Tokenize();

	ASSERT_TRUE(CompareTokenArray(g_TestCodeExpextedTokens, result));

	skyfauna::common::Logger::Shutdown();
}


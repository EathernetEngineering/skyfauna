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
	skyfauna::Token(skyfauna::TokenType::COMMENT, skyfauna::CommentType::LINE, "// Line comment"),
	skyfauna::Token(skyfauna::TokenType::COMMENT, skyfauna::CommentType::BLOCK, "/* Block \ncomment */"),
	skyfauna::Token(skyfauna::TokenType::IDENTIFIER, skyfauna::IdentifierType::KEYWORD, "class"),
	skyfauna::Token(skyfauna::TokenType::IDENTIFIER, skyfauna::IdentifierType::SYMBOL, "Main"),
	skyfauna::Token(skyfauna::TokenType::PUNCTUATOR, skyfauna::PuncType::DELIMITER, "{"),
	skyfauna::Token(skyfauna::TokenType::IDENTIFIER, skyfauna::IdentifierType::KEYWORD, "public"),
	skyfauna::Token(skyfauna::TokenType::IDENTIFIER, skyfauna::IdentifierType::KEYWORD, "static"),
	skyfauna::Token(skyfauna::TokenType::IDENTIFIER, skyfauna::IdentifierType::SYMBOL, "main"),
	skyfauna::Token(skyfauna::TokenType::PUNCTUATOR, skyfauna::PuncType::DELIMITER, "("),
	skyfauna::Token(skyfauna::TokenType::IDENTIFIER, skyfauna::IdentifierType::KEYWORD, "string"),
	skyfauna::Token(skyfauna::TokenType::PUNCTUATOR, skyfauna::PuncType::DELIMITER, "["),
	skyfauna::Token(skyfauna::TokenType::PUNCTUATOR, skyfauna::PuncType::DELIMITER, "]"),
	skyfauna::Token(skyfauna::TokenType::PUNCTUATOR, skyfauna::PuncType::DELIMITER, ":"),
	skyfauna::Token(skyfauna::TokenType::IDENTIFIER, skyfauna::IdentifierType::SYMBOL, "args"),
	skyfauna::Token(skyfauna::TokenType::PUNCTUATOR, skyfauna::PuncType::DELIMITER, ")"),
	skyfauna::Token(skyfauna::TokenType::PUNCTUATOR, skyfauna::PuncType::DELIMITER, "("),
	skyfauna::Token(skyfauna::TokenType::IDENTIFIER, skyfauna::IdentifierType::KEYWORD, "void"),
	skyfauna::Token(skyfauna::TokenType::PUNCTUATOR, skyfauna::PuncType::DELIMITER, ")"),
	skyfauna::Token(skyfauna::TokenType::PUNCTUATOR, skyfauna::PuncType::DELIMITER, "{"),
	skyfauna::Token(skyfauna::TokenType::IDENTIFIER, skyfauna::IdentifierType::KEYWORD, "ptr"),
	skyfauna::Token(skyfauna::TokenType::PUNCTUATOR, skyfauna::PuncType::DELIMITER, ":"),
	skyfauna::Token(skyfauna::TokenType::IDENTIFIER, skyfauna::IdentifierType::SYMBOL, "address"),
	skyfauna::Token(skyfauna::TokenType::PUNCTUATOR, skyfauna::PuncType::OPERATOR, "="),
	skyfauna::Token(skyfauna::TokenType::IDENTIFIER, skyfauna::IdentifierType::KEYWORD, "null"),
	skyfauna::Token(skyfauna::TokenType::PUNCTUATOR, skyfauna::PuncType::DELIMITER, ";"),
	skyfauna::Token(skyfauna::TokenType::IDENTIFIER, skyfauna::IdentifierType::SYMBOL, "print"),
	skyfauna::Token(skyfauna::TokenType::PUNCTUATOR, skyfauna::PuncType::DELIMITER, "("),
	skyfauna::Token(skyfauna::TokenType::IDENTIFIER, skyfauna::IdentifierType::SYMBOL, "args"),
	skyfauna::Token(skyfauna::TokenType::PUNCTUATOR, skyfauna::PuncType::DELIMITER, "["),
	skyfauna::Token(skyfauna::TokenType::LITERAL, skyfauna::LiteralType::INTEGER, "0"),
	skyfauna::Token(skyfauna::TokenType::PUNCTUATOR, skyfauna::PuncType::DELIMITER, "]"),
	skyfauna::Token(skyfauna::TokenType::PUNCTUATOR, skyfauna::PuncType::DELIMITER, ")"),
	skyfauna::Token(skyfauna::TokenType::PUNCTUATOR, skyfauna::PuncType::DELIMITER, ";"),
	skyfauna::Token(skyfauna::TokenType::IDENTIFIER, skyfauna::IdentifierType::SYMBOL, "print"),
	skyfauna::Token(skyfauna::TokenType::PUNCTUATOR, skyfauna::PuncType::DELIMITER, "("),
	skyfauna::Token(skyfauna::TokenType::IDENTIFIER, skyfauna::IdentifierType::SYMBOL, "args"),
	skyfauna::Token(skyfauna::TokenType::PUNCTUATOR, skyfauna::PuncType::DELIMITER, "["),
	skyfauna::Token(skyfauna::TokenType::LITERAL, skyfauna::LiteralType::INTEGER, "0"),
	skyfauna::Token(skyfauna::TokenType::PUNCTUATOR, skyfauna::PuncType::OPERATOR, "+"),
	skyfauna::Token(skyfauna::TokenType::LITERAL, skyfauna::LiteralType::INTEGER, "1"),
	skyfauna::Token(skyfauna::TokenType::PUNCTUATOR, skyfauna::PuncType::DELIMITER, "]"),
	skyfauna::Token(skyfauna::TokenType::PUNCTUATOR, skyfauna::PuncType::DELIMITER, ")"),
	skyfauna::Token(skyfauna::TokenType::PUNCTUATOR, skyfauna::PuncType::DELIMITER, ";"),
	skyfauna::Token(skyfauna::TokenType::PUNCTUATOR, skyfauna::PuncType::DELIMITER, "}"),
	skyfauna::Token(skyfauna::TokenType::PUNCTUATOR, skyfauna::PuncType::DELIMITER, "}"),
	skyfauna::Token(skyfauna::TokenType::PUNCTUATOR, skyfauna::PuncType::DELIMITER, ";"),
	skyfauna::Token(skyfauna::TokenType::PUNCTUATOR, skyfauna::PuncType::DELIMITER, "EOF"),
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
					if (l.type != r.type) {
						eq = false;
						errors.push_back(fmt::format("Token {} type not eq ({} vs {})", l.text, l.type, r.type));
					}
					if (l.subtype != r.subtype) {
						eq = false;
						errors.push_back(fmt::format("Token {} subtype not eq", l.text));
					}
					if (strcmp(l.text.c_str(), r.text.c_str())) {
						eq = false;
						errors.push_back(fmt::format("Token {} text not eq ({})", l.text, r.text));
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


// SPDX-License-Identifier: MIT
// Copyright (C) 2025 Chloe Eather

#include <skyfauna/lex.h>
#include <skyfauna/common/log.h>

#include <string_view>

#include <gtest/gtest.h>

static constexpr std::array g_Keywords = std::to_array<std::string_view>({
	"int",
	"float",
	"char",
	"string",
	"void",
	"ptr",
	"return",
	"static",
	"class",
	"public",
	"private",
	"protected",
	"null",
});
const std::string_view g_TestCode =
	"// Line comment\n"
	"/* Block \n"
	"comment */\n"
	"\n"
	"int : x, y;\n"
	"float : f = 2.2;\n"
	"char : z = 'c';\n"
	"string : s = \"Hello, World!\";\n"
	"ptr : p;\n"
	"\n"
	"x = x + x;\n"
	"x = x - x;\n"
	"x = x * x;\n"
	"x = x / x;\n"
	"x = x % x;\n"
	"x = x & x;\n"
	"x = x | x;\n"
	"x = x ^ x;\n"
	"x = x << x;\n"
	"x = x >> x;\n"
	"x++;\n"
	"++x;\n"
	"x = !x;\n"
	"x = x && x;\n"
	"x = x || x;\n"
	"x = x == x;\n"
	"x = x != x;\n"
	"x = x < x;\n"
	"x = x <= x;\n"
	"x = x > x;\n"
	"x = x >= x;\n"
	"x += x;\n"
	"x -= x;\n"
	"x *= x;\n"
	"x /= x;\n"
	"x %= x;\n"
	"x &= x;\n"
	"x |= x;\n"
	"x ^= x;\n"
	"x <<= x;\n"
	"x >>= x;\n"
	"\n"
	"class Main {\n"
	"	public static main(string[]: args)(int) {\n"
	"		ptr: address = null;\n"
	"		print(args[0]);\n"
	"		print(args[0 + 1]);\n"
	"		return 0;\n"
	"	}\n"
	"	private int : m_I;\n"
	"	protected ptr : m_P = null;\n"
	"};";
std::array g_TestCodeExpextedTokens = std::to_array({
	skyfauna::Token(skyfauna::TTComment::LINE, "// Line comment"),
	skyfauna::Token(skyfauna::TTComment::BLOCK, "/* Block \ncomment */"),
	skyfauna::Token(skyfauna::TTIdentifier::KEYWORD, "int"),
	skyfauna::Token(skyfauna::TTPunctuator::DELIMITER, ":"),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "x"),
	skyfauna::Token(skyfauna::TTPunctuator::DELIMITER, ","),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "y"),
	skyfauna::Token(skyfauna::TTPunctuator::DELIMITER, ";"),
	skyfauna::Token(skyfauna::TTIdentifier::KEYWORD, "float"),
	skyfauna::Token(skyfauna::TTPunctuator::DELIMITER, ":"),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "f"),
	skyfauna::Token(skyfauna::TTPunctuator::OPERATOR, "="),
	skyfauna::Token(skyfauna::TTLiteral::FLOATING_POINT, "2.2"),
	skyfauna::Token(skyfauna::TTPunctuator::DELIMITER, ";"),
	skyfauna::Token(skyfauna::TTIdentifier::KEYWORD, "char"),
	skyfauna::Token(skyfauna::TTPunctuator::DELIMITER, ":"),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "z"),
	skyfauna::Token(skyfauna::TTPunctuator::OPERATOR, "="),
	skyfauna::Token(skyfauna::TTLiteral::CHAR, "'c'"),
	skyfauna::Token(skyfauna::TTPunctuator::DELIMITER, ";"),
	skyfauna::Token(skyfauna::TTIdentifier::KEYWORD, "string"),
	skyfauna::Token(skyfauna::TTPunctuator::DELIMITER, ":"),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "s"),
	skyfauna::Token(skyfauna::TTPunctuator::OPERATOR, "="),
	skyfauna::Token(skyfauna::TTLiteral::STRING, "\"Hello, World!\""),
	skyfauna::Token(skyfauna::TTPunctuator::DELIMITER, ";"),
	skyfauna::Token(skyfauna::TTIdentifier::KEYWORD, "ptr"),
	skyfauna::Token(skyfauna::TTPunctuator::DELIMITER, ":"),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "p"),
	skyfauna::Token(skyfauna::TTPunctuator::DELIMITER, ";"),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "x"),
	skyfauna::Token(skyfauna::TTPunctuator::OPERATOR, "="),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "x"),
	skyfauna::Token(skyfauna::TTPunctuator::OPERATOR, "+"),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "x"),
	skyfauna::Token(skyfauna::TTPunctuator::DELIMITER, ";"),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "x"),
	skyfauna::Token(skyfauna::TTPunctuator::OPERATOR, "="),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "x"),
	skyfauna::Token(skyfauna::TTPunctuator::OPERATOR, "-"),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "x"),
	skyfauna::Token(skyfauna::TTPunctuator::DELIMITER, ";"),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "x"),
	skyfauna::Token(skyfauna::TTPunctuator::OPERATOR, "="),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "x"),
	skyfauna::Token(skyfauna::TTPunctuator::OPERATOR, "*"),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "x"),
	skyfauna::Token(skyfauna::TTPunctuator::DELIMITER, ";"),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "x"),
	skyfauna::Token(skyfauna::TTPunctuator::OPERATOR, "="),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "x"),
	skyfauna::Token(skyfauna::TTPunctuator::OPERATOR, "/"),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "x"),
	skyfauna::Token(skyfauna::TTPunctuator::DELIMITER, ";"),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "x"),
	skyfauna::Token(skyfauna::TTPunctuator::OPERATOR, "="),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "x"),
	skyfauna::Token(skyfauna::TTPunctuator::OPERATOR, "%"),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "x"),
	skyfauna::Token(skyfauna::TTPunctuator::DELIMITER, ";"),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "x"),
	skyfauna::Token(skyfauna::TTPunctuator::OPERATOR, "="),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "x"),
	skyfauna::Token(skyfauna::TTPunctuator::OPERATOR, "&"),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "x"),
	skyfauna::Token(skyfauna::TTPunctuator::DELIMITER, ";"),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "x"),
	skyfauna::Token(skyfauna::TTPunctuator::OPERATOR, "="),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "x"),
	skyfauna::Token(skyfauna::TTPunctuator::OPERATOR, "|"),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "x"),
	skyfauna::Token(skyfauna::TTPunctuator::DELIMITER, ";"),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "x"),
	skyfauna::Token(skyfauna::TTPunctuator::OPERATOR, "="),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "x"),
	skyfauna::Token(skyfauna::TTPunctuator::OPERATOR, "^"),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "x"),
	skyfauna::Token(skyfauna::TTPunctuator::DELIMITER, ";"),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "x"),
	skyfauna::Token(skyfauna::TTPunctuator::OPERATOR, "="),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "x"),
	skyfauna::Token(skyfauna::TTPunctuator::OPERATOR, "<<"),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "x"),
	skyfauna::Token(skyfauna::TTPunctuator::DELIMITER, ";"),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "x"),
	skyfauna::Token(skyfauna::TTPunctuator::OPERATOR, "="),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "x"),
	skyfauna::Token(skyfauna::TTPunctuator::OPERATOR, ">>"),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "x"),
	skyfauna::Token(skyfauna::TTPunctuator::DELIMITER, ";"),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "x"),
	skyfauna::Token(skyfauna::TTPunctuator::OPERATOR, "++"),
	skyfauna::Token(skyfauna::TTPunctuator::DELIMITER, ";"),
	skyfauna::Token(skyfauna::TTPunctuator::OPERATOR, "++"),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "x"),
	skyfauna::Token(skyfauna::TTPunctuator::DELIMITER, ";"),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "x"),
	skyfauna::Token(skyfauna::TTPunctuator::OPERATOR, "="),
	skyfauna::Token(skyfauna::TTPunctuator::OPERATOR, "!"),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "x"),
	skyfauna::Token(skyfauna::TTPunctuator::DELIMITER, ";"),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "x"),
	skyfauna::Token(skyfauna::TTPunctuator::OPERATOR, "="),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "x"),
	skyfauna::Token(skyfauna::TTPunctuator::OPERATOR, "&&"),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "x"),
	skyfauna::Token(skyfauna::TTPunctuator::DELIMITER, ";"),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "x"),
	skyfauna::Token(skyfauna::TTPunctuator::OPERATOR, "="),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "x"),
	skyfauna::Token(skyfauna::TTPunctuator::OPERATOR, "||"),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "x"),
	skyfauna::Token(skyfauna::TTPunctuator::DELIMITER, ";"),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "x"),
	skyfauna::Token(skyfauna::TTPunctuator::OPERATOR, "="),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "x"),
	skyfauna::Token(skyfauna::TTPunctuator::OPERATOR, "=="),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "x"),
	skyfauna::Token(skyfauna::TTPunctuator::DELIMITER, ";"),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "x"),
	skyfauna::Token(skyfauna::TTPunctuator::OPERATOR, "="),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "x"),
	skyfauna::Token(skyfauna::TTPunctuator::OPERATOR, "!="),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "x"),
	skyfauna::Token(skyfauna::TTPunctuator::DELIMITER, ";"),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "x"),
	skyfauna::Token(skyfauna::TTPunctuator::OPERATOR, "="),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "x"),
	skyfauna::Token(skyfauna::TTPunctuator::OPERATOR, "<"),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "x"),
	skyfauna::Token(skyfauna::TTPunctuator::DELIMITER, ";"),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "x"),
	skyfauna::Token(skyfauna::TTPunctuator::OPERATOR, "="),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "x"),
	skyfauna::Token(skyfauna::TTPunctuator::OPERATOR, "<="),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "x"),
	skyfauna::Token(skyfauna::TTPunctuator::DELIMITER, ";"),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "x"),
	skyfauna::Token(skyfauna::TTPunctuator::OPERATOR, "="),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "x"),
	skyfauna::Token(skyfauna::TTPunctuator::OPERATOR, ">"),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "x"),
	skyfauna::Token(skyfauna::TTPunctuator::DELIMITER, ";"),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "x"),
	skyfauna::Token(skyfauna::TTPunctuator::OPERATOR, "="),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "x"),
	skyfauna::Token(skyfauna::TTPunctuator::OPERATOR, ">="),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "x"),
	skyfauna::Token(skyfauna::TTPunctuator::DELIMITER, ";"),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "x"),
	skyfauna::Token(skyfauna::TTPunctuator::OPERATOR, "+="),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "x"),
	skyfauna::Token(skyfauna::TTPunctuator::DELIMITER, ";"),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "x"),
	skyfauna::Token(skyfauna::TTPunctuator::OPERATOR, "-="),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "x"),
	skyfauna::Token(skyfauna::TTPunctuator::DELIMITER, ";"),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "x"),
	skyfauna::Token(skyfauna::TTPunctuator::OPERATOR, "*="),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "x"),
	skyfauna::Token(skyfauna::TTPunctuator::DELIMITER, ";"),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "x"),
	skyfauna::Token(skyfauna::TTPunctuator::OPERATOR, "/="),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "x"),
	skyfauna::Token(skyfauna::TTPunctuator::DELIMITER, ";"),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "x"),
	skyfauna::Token(skyfauna::TTPunctuator::OPERATOR, "%="),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "x"),
	skyfauna::Token(skyfauna::TTPunctuator::DELIMITER, ";"),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "x"),
	skyfauna::Token(skyfauna::TTPunctuator::OPERATOR, "&="),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "x"),
	skyfauna::Token(skyfauna::TTPunctuator::DELIMITER, ";"),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "x"),
	skyfauna::Token(skyfauna::TTPunctuator::OPERATOR, "|="),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "x"),
	skyfauna::Token(skyfauna::TTPunctuator::DELIMITER, ";"),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "x"),
	skyfauna::Token(skyfauna::TTPunctuator::OPERATOR, "^="),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "x"),
	skyfauna::Token(skyfauna::TTPunctuator::DELIMITER, ";"),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "x"),
	skyfauna::Token(skyfauna::TTPunctuator::OPERATOR, "<<="),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "x"),
	skyfauna::Token(skyfauna::TTPunctuator::DELIMITER, ";"),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "x"),
	skyfauna::Token(skyfauna::TTPunctuator::OPERATOR, ">>="),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "x"),
	skyfauna::Token(skyfauna::TTPunctuator::DELIMITER, ";"),
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
	skyfauna::Token(skyfauna::TTIdentifier::KEYWORD, "int"),
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
	skyfauna::Token(skyfauna::TTIdentifier::KEYWORD, "return"),
	skyfauna::Token(skyfauna::TTLiteral::INTEGER, "0"),
	skyfauna::Token(skyfauna::TTPunctuator::DELIMITER, ";"),
	skyfauna::Token(skyfauna::TTPunctuator::DELIMITER, "}"),
	skyfauna::Token(skyfauna::TTIdentifier::KEYWORD, "private"),
	skyfauna::Token(skyfauna::TTIdentifier::KEYWORD, "int"),
	skyfauna::Token(skyfauna::TTPunctuator::DELIMITER, ":"),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "m_I"),
	skyfauna::Token(skyfauna::TTPunctuator::DELIMITER, ";"),
	skyfauna::Token(skyfauna::TTIdentifier::KEYWORD, "protected"),
	skyfauna::Token(skyfauna::TTIdentifier::KEYWORD, "ptr"),
	skyfauna::Token(skyfauna::TTPunctuator::DELIMITER, ":"),
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "m_P"),
	skyfauna::Token(skyfauna::TTPunctuator::OPERATOR, "="),
	skyfauna::Token(skyfauna::TTIdentifier::KEYWORD, "null"),
	skyfauna::Token(skyfauna::TTPunctuator::DELIMITER, ";"),
	skyfauna::Token(skyfauna::TTPunctuator::DELIMITER, "}"),
	skyfauna::Token(skyfauna::TTPunctuator::DELIMITER, ";"),
});

template<typename Lhs, typename Rhs>
testing::AssertionResult CompareTokenArray(const Lhs& lhs, const Rhs& rhs)
{
	if (lhs.size() != rhs.size())
		return testing::AssertionFailure() << "Token count different to expected (" <<
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
	skyfauna::common::Logger::SetLogLevel(spdlog::level::err);

	skyfauna::Lexer l(std::vector<std::string_view>(g_Keywords.begin(), g_Keywords.end()), g_TestCode);
	const std::vector<skyfauna::Token>& result =  l.Tokenize().GetTokens();
	std::cout << std::endl;

	ASSERT_TRUE(CompareTokenArray(g_TestCodeExpextedTokens, result));

	skyfauna::common::Logger::Shutdown();
}


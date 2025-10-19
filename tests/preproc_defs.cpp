// SPDX-License-Identifier: MIT
// Copyright (C) 2025 Chloe Eather

#include "test_literals.h"

namespace skyfauna::tests {
const std::string_view g_PreprocessorValidCode = "#include <print>\n"
	"#define macro 22\n"
	"#define fmacro(...) print(vargs, macro)()\n"
	"#if 1\n"
	"#elif 0\n"
	"#endif\n"
	"fmacro(\"{}\", macro);\n";

const std::array<skyfauna::Token, g_PreprocessorValidExpextedTokenCount>
g_PreprocessorValidExpextedTokens = std::to_array({
	skyfauna::Token(skyfauna::TTIdentifier::SYMBOL, "print"),
	skyfauna::Token(skyfauna::TTPunctuator::DELIMITER, "("),
	skyfauna::Token(skyfauna::TTLiteral::STRING, "\"{}\""),
	skyfauna::Token(skyfauna::TTPunctuator::DELIMITER, ","),
	skyfauna::Token(skyfauna::TTLiteral::INTEGER, "22"),
	skyfauna::Token(skyfauna::TTPunctuator::DELIMITER, ","),
	skyfauna::Token(skyfauna::TTLiteral::INTEGER, "22"),
	skyfauna::Token(skyfauna::TTPunctuator::DELIMITER, ")"),
	skyfauna::Token(skyfauna::TTPunctuator::DELIMITER, "("),
	skyfauna::Token(skyfauna::TTPunctuator::DELIMITER, ")"),
	skyfauna::Token(skyfauna::TTPunctuator::DELIMITER, ";"),
});
}


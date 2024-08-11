#ifndef CEE_TOKENIZER_H_
#define CEE_TOKENIZER_H_

#include <string>
#include <string_view>
#include <variant>
#include <vector>
#include <cstdint>

enum class TokenType : uint32_t {
	INVALID,
	KEYWORD,
	DECIMAL_LITERAL,
	INT_LITERAL,
	CHAR_LITERAL,
	STRING_LITERAL,
	SYMBOL,
	ADD,
	INC,
	SUB,
	DEC,
	MUL,
	DIV,
	MOD,
	XOR,
	AND,
	OR,
	BITAND,
	BITOR,
	BITSL,
	BITSR,
	LESSTHAN,
	GREATERTHAN,
	LESSTHANEQ,
	GREATERTHANEQ,
	EQUAL,
	ASSIGN,
	SEMICOLON,
	COLON,
	PAREN_OPEN,
	PAREN_CLOSE,
	BRACKET_OPEN,
	BRACKET_CLOSE,
	BRACE_OPEN,
	BRACE_CLOSE
};

struct Token {
	TokenType type;
	std::string text;
	std::variant <int64_t, double> value;
};

std::vector<Token> Tokenize(const std::string& fileName);

#endif

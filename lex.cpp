#include "lex.h"

#include <cctype>
#include <map>
#include <set>
#include <stdexcept>
#include <format>
#include <string>
#include <string_view>

#include <unistd.h>
#include <fcntl.h>

static const std::map<std::string, TokenType> g_OperatorMap = {
	{ "+", TokenType::ADD },
	{ "++", TokenType::INC },
	{ "-", TokenType::SUB },
	{ "--", TokenType::DEC },
	{ "*", TokenType::MUL },
	{ "/", TokenType::DIV },
	{ "%", TokenType::MOD },
	{ "^", TokenType::XOR },
	{ "&&", TokenType::AND },
	{ "||", TokenType::OR },
	{ "&", TokenType::BITAND },
	{ "|", TokenType::BITOR },
	{ "<<", TokenType::BITSL },
	{ ">>", TokenType::BITSR },
	{ "<", TokenType::LESSTHAN },
	{ ">", TokenType::GREATERTHAN },
	{ "<=", TokenType::LESSTHANEQ },
	{ ">=", TokenType::GREATERTHANEQ },
	{ "==", TokenType::EQUAL },
	{ "=", TokenType::ASSIGN },
	{ ";", TokenType::SEMICOLON },
	{ ":", TokenType::COLON },
	{ "(", TokenType::PAREN_OPEN },
	{ ")", TokenType::PAREN_CLOSE },
	{ "[", TokenType::BRACKET_OPEN },
	{ "]", TokenType::BRACKET_CLOSE },
	{ "{", TokenType::BRACE_OPEN },
	{ "}", TokenType::BRACE_CLOSE },
};

std::set<std::string> g_Keywords = {
	"null",
	"int",
	"double",
	"bool",
	"str",
	"class",
	"if",
	"else",
	"switch",
	"for",
	"do",
	"while",
	"break",
	"return",
};

enum class LexerState : uint16_t {
	NEW_TOKEN,
	NUMERIC_LITERAL,
	STRING_LITERAL,
	CHAR_LITERAL,
	OPERATOR,
	SYMBOL,
	COMPLETE_TOKEN,
	EOF_TOKEN
};

static std::string XSpaces(int n) {
	std::string str;
	for (uint32_t i = 0; i < n; i++) {
		str.push_back(' ');
	}
	return str;
}

static std::vector<std::string_view> SplitDelim(const std::string& str, char delim) {
	std::vector<std::string_view> split;

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

std::vector<Token> Tokenize(const std::string& fileName) {
	int fileDesc = open(fileName.c_str(), 0, O_RDONLY);
	if (fileDesc == -1) {
		throw std::runtime_error(std::format("Failed to open file: {}", fileName));
	}

	std::string file;

	{
		size_t fileSize = lseek(fileDesc, 0, SEEK_END);
		lseek(fileDesc, 0, SEEK_SET);

		file.resize(fileSize);
		read(fileDesc, file.data(), fileSize);
	}

	close(fileDesc);
	
	auto lines = SplitDelim(file, '\n');

	int parenCount = 0;
	int bracketCount = 0;
	int braceCount = 0;

	bool decimal = false;

	Token currentToken;
	std::vector<Token> tokens;
	LexerState lexState = LexerState::NEW_TOKEN;

	auto lineIt = lines.begin();
	if (lineIt == lines.end()) {
		return tokens;
	}
	auto charIt = (*lineIt).begin();

	for (;;) {
		if (charIt == (*lineIt).end()) {
			std::advance(lineIt, 1);
			if (lineIt == lines.end()) {
				break;
			}
			charIt = (*lineIt).begin();
			continue;
		}
		char c = *charIt;
		switch (lexState) {
			case LexerState::NEW_TOKEN:
				if (isspace(c)) {
					break;
				}
				currentToken.text.push_back(c);
				if (std::isdigit(c)) {
					lexState = LexerState::NUMERIC_LITERAL;
				}
				if (g_OperatorMap.contains(&c)) {
					lexState = LexerState::OPERATOR;
				}
				if (isalpha(c)) {
					lexState = LexerState::SYMBOL;
				}
				if (c == '\'') {
					currentToken.text.pop_back();
					lexState = LexerState::CHAR_LITERAL;
				}
				if (c == '\"') {
					currentToken.text.pop_back();
					lexState = LexerState::STRING_LITERAL;
				}
				if (std::string("()[]{}").find(c) != -1) {
					switch (c) {
						case '(':
							++parenCount;
							currentToken.type = TokenType::PAREN_OPEN;
							break;
						case ')':
							--parenCount;
							currentToken.type = TokenType::PAREN_CLOSE;
							break;
						case '[':
							++bracketCount;
							currentToken.type = TokenType::BRACKET_OPEN;
							break;
						case ']':
							--bracketCount;
							currentToken.type = TokenType::BRACKET_CLOSE;
							break;
						case '{':
							++braceCount;
							currentToken.type = TokenType::BRACE_OPEN;
							break;
						case '}':
							--braceCount;
							currentToken.type = TokenType::BRACE_CLOSE;
							break;
					}
					lexState = LexerState::COMPLETE_TOKEN;
				}
				break;

			case LexerState::NUMERIC_LITERAL:
				if (c == '.') {
					if (decimal == true) {
						throw std::runtime_error(std::format("Syntax Error: too many decimal points in number\n{}", *lineIt));
					}
					decimal = true;
					currentToken.text.push_back(c);
					break;
				}
				if (isdigit(c)) {
					currentToken.text.push_back(c);
				} else {
					if (decimal) {
						currentToken.value = std::stod(currentToken.text);
						decimal = false;
					} else {
						currentToken.value = std::stoll(currentToken.text);
					}
					currentToken.type = decimal ? TokenType::DECIMAL_LITERAL : TokenType::INT_LITERAL;
					lexState = LexerState::COMPLETE_TOKEN;
					continue;
				}
				break;

			case LexerState::OPERATOR:
				currentToken.text.push_back(c);
				if (!g_OperatorMap.contains(currentToken.text)) {
					currentToken.text.pop_back();
					currentToken.type = g_OperatorMap.at(currentToken.text);
					lexState = LexerState::COMPLETE_TOKEN;
					continue;
				}
				break;

			case LexerState::SYMBOL:
				if (std::isalnum(c)) {
					currentToken.text.push_back(c);
				} else {
					if (g_Keywords.contains(currentToken.text)) {
						currentToken.type = TokenType::KEYWORD;
					} else {
						currentToken.type = TokenType::SYMBOL;
					}
					lexState = LexerState::COMPLETE_TOKEN;
					continue;
				}
				break;

			case LexerState::CHAR_LITERAL:
				if (c == '\'') {
					if (currentToken.text.length() > 1) {
						throw std::runtime_error("Syntax Error: Expected \"\'\"");
					}
					currentToken.type = TokenType::CHAR_LITERAL;
					lexState = LexerState::COMPLETE_TOKEN;
				} else {
					currentToken.text.push_back(c);
				}
				break;

			case LexerState::STRING_LITERAL:
				if (c == '\"') {
					currentToken.type = TokenType::STRING_LITERAL;
					lexState = LexerState::COMPLETE_TOKEN;
				} else {
					currentToken.text.push_back(c);
				}
				break;

			case LexerState::COMPLETE_TOKEN:
				tokens.push_back(currentToken);
				currentToken = Token();
				lexState = LexerState::NEW_TOKEN;
				continue;

			default:
				throw std::runtime_error("Unknown lexer state.");
		}
		std::advance(charIt, 1);
	}

	if (parenCount != 0) {
		throw std::runtime_error("Syntax Error: Unbalaced parentheses (\"()\").");
	}
	if (bracketCount != 0) {
		throw std::runtime_error("Syntax Error: Unbalaced brackets (\"[]\").");
	}
	if (braceCount != 0) {
		throw std::runtime_error("Syntax Error: Unbalaced braces (\"{}\").");
	}

	return tokens;
}


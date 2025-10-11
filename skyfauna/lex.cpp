// SPDX-License-Identifier: MIT
// Copyright (C) 2025 Chloe Eather

#include <skyfauna/lex.h>
#include <skyfauna/common/log.h>
#include <skyfauna/common/util.h>

#include <algorithm>
#include <array>
#include <stdexcept>
#include <string>
#include <string_view>
#include <cctype>
#include <fcntl.h>
#include <unistd.h>

// ***********************************************
// ********** Types looked for by lexer **********
// ***************** Lexer state *****************
// ***********************************************

static constexpr std::array g_Operators = std::to_array<std::string_view>({
	"+", "-", "*", "/", "%", "&", "|", "^", "<<", ">>", "++", "--",
	"!", "&&", "||", "==", "!=", "<", "<=", ">", ">=",
	"=", "+=", "-=" , "*=", "/=", "%=", "&=", "|=", "^=", "<<=", ">>="
});
static constexpr size_t g_LongestOperator =
	skyfauna::LongestStrlen(g_Operators);

static constexpr std::array g_Delimiters = std::to_array<std::string_view>({
	"(", ")", "[", "]", "{", "}", ",", ".", ";", ":", ".", "?", "->",
	"#", "...",
	"//", "/*"
});
static constexpr size_t g_LongestDelimiter =
	skyfauna::LongestStrlen(g_Delimiters);

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

static constexpr std::array g_KeywordHashes = skyfauna::HashStrings(g_Keywords);


namespace fmt {
template<>
struct formatter<skyfauna::Lexer::State> : formatter<std::string> {
	auto format(skyfauna::Lexer::State t, format_context& ctx) const -> decltype(ctx.out())
	{
		const char *name;
		switch (t) {
			case skyfauna::Lexer::State::BAD:
				name = "BAD";
				break;

			case skyfauna::Lexer::State::NEW_TOKEN:
				name = "NEW_TOKEN";
				break;

			case skyfauna::Lexer::State::NUMERIC_LITERAL:
				name = "NUMERIC_LITERAL";
				break;

			case skyfauna::Lexer::State::STRING_LITERAL:
				name = "STRING_LITERAL";
				break;

			case skyfauna::Lexer::State::CHAR_LITERAL:
				name = "CHAR_LITERAL";
				break;

			case skyfauna::Lexer::State::OPERATOR:
				name = "OPERATOR";
				break;

			case skyfauna::Lexer::State::DELIMITER:
				name = "DELIMITER";
				break;

			case skyfauna::Lexer::State::IDENTIFIER:
				name = "IDENTIFIER";
				break;

			case skyfauna::Lexer::State::LINE_COMMENT:
				name = "LINE_COMMENT";
				break;

			case skyfauna::Lexer::State::BLOCK_COMMENT:
				name = "BLOCK_COMMENT";
				break;

			case skyfauna::Lexer::State::COMPLETE_TOKEN:
				name = "COMPLETE_TOKEN";
				break;

			case skyfauna::Lexer::State::EOF_TOKEN:
				name = "EOF_TOKEN";
				break;

			default:
				throw std::runtime_error("fmt: invalid State");
		}
		return format_to(ctx.out(), "{}", name);
	}
};
}

namespace skyfauna {
Lexer::Lexer(std::string&& code) noexcept
 : m_Code(std::move(code))
{
}

std::vector<Token>& Lexer::Tokenize()
{
	if (m_Tokens.size() != 0) {
		SF_WARN("Lexer::Tokenize() called with full buffer");
		m_Tokens.clear();
	}

	m_Tokens.reserve(1024);
	m_State = State::NEW_TOKEN;
	m_CToken = Token();

	int a = 0;
	auto it = m_Code.cbegin();
	for (; it != m_Code.cend();) {
		auto c = *it;
		a = Transition(c);
		
		// This is not in the for to conditionally avoid incrementing in
		// steps that don't consume characters, using `continue`
		std::advance(it, a);
	}
	while (m_State != State::BAD && m_State != State::NEW_TOKEN)
		if (Transition('\0') != 0)
			break;
	if (m_CToken.text.length() > 0) {
		m_CToken.type = skyfauna::TokenType::INVALID;
		m_Tokens.emplace_back(std::move(m_CToken));
	}
	m_Tokens.emplace_back(TokenType::PUNCTUATOR, PuncType::DELIMITER, "EOF", 0);

	m_Tokens.shrink_to_fit();
	return m_Tokens;
}

int Lexer::Transition(char c)
{
	// Early return in switch on success, break on invalid which
	// throws std::logic_error
	switch (m_State) {
		case State::BAD:
			if (m_RecoveryFunc && m_RecoveryFunc(c)) {
				m_State = State::COMPLETE_TOKEN;
				return 0;
			} else {
				m_CToken.text.push_back(c);
				return 1;
			}
			break;

		case State::NEW_TOKEN:
			if (c == '\0') {
				throw std::runtime_error("unexpexted null terminator");
			}
			if (std::isspace(c)) {
				return 1;
			}
			m_CToken.text.push_back(c);
			if (std::isalpha(c)) {
				m_State = State::IDENTIFIER;
				m_CToken.type = TokenType::IDENTIFIER;
				m_RecoveryFunc = [](char c){ return !std::isalnum(c); };
			} else if (std::isdigit(c)) {
				m_State = State::NUMERIC_LITERAL;
				m_CToken.type = TokenType::LITERAL;
				m_RecoveryFunc = [](char c){
					return !(std::isalnum(c) || c == '.');
				};
			} else if (c == '\'') {
				m_State = State::CHAR_LITERAL;
				m_CToken.type = TokenType::LITERAL;
				m_CToken.subtype = LiteralType::CHAR;
				m_RecoveryFunc = [](char c){
					return (std::ispunct(c) || std::isspace(c));
				};
			} else if (c == '\"') {
				m_State = State::STRING_LITERAL;
				m_CToken.type = TokenType::LITERAL;
				m_CToken.subtype = LiteralType::STRING;
				m_RecoveryFunc = [](char c){ return std::iscntrl(c); };
			} else if (std::ispunct(c)) {
				m_State = State::PUNCTUATOR;
				m_CToken.type = TokenType::PUNCTUATOR;
				m_RecoveryFunc = [](char c){ (void)c; return true; };
			} else {
				m_State = State::BAD;
				m_CToken.type = TokenType::INVALID;
				m_RecoveryFunc = [](char c){ return std::isprint(c); };
			}
			return 1;

		case State::NUMERIC_LITERAL:
			if (c == '.') {
				if (m_CToken.text.find('.') != m_CToken.text.npos)
					m_State = State::BAD;
			} else if (std::isdigit(c)) {
			} else if (std::isalpha(c)) {
				m_State = State::BAD;
			} else {
				if (m_CToken.text.find('.') != m_CToken.text.npos) {
					m_CToken.subtype = LiteralType::FLOATING_POINT;
					m_CToken.value = std::make_any<float>(std::atof(m_CToken.text.c_str()));
				} else {
					m_CToken.subtype = LiteralType::INTEGER;
					m_CToken.value = std::make_any<int>(std::atoi(m_CToken.text.c_str()));
				}
				m_State = Lexer::State::COMPLETE_TOKEN;
				return 0;
			}
			m_CToken.text.push_back(c);
			return 1;

		case State::STRING_LITERAL:
			if (c == '\"') {
				m_State = State::COMPLETE_TOKEN;
			} else if (c == '\n') {
				m_State = State::BAD;
				return 0;
			}
			m_CToken.text.push_back(c);
			return 1;

		case State::CHAR_LITERAL:
			if (c == '\'') {
				m_State = State::COMPLETE_TOKEN;
			} else if (c == '\n') {
				m_State = State::BAD;
				return 0;
			}
			m_CToken.text.push_back(c);
			return 1;

		case State::PUNCTUATOR: {
			bool cdelim = false, cop = false;
			if (m_CToken.text.length() == 2) {
				if (std::strncmp(m_CToken.text.c_str(), "//", 2) == 0) {
					m_State = State::LINE_COMMENT;
				} else if (std::strncmp(m_CToken.text.c_str(), "/*", 2) == 0) {
					m_State = State::BLOCK_COMMENT;
				}
				if (m_State == State::LINE_COMMENT || m_State == State::BLOCK_COMMENT) {
					m_CToken.text.push_back(c);
					return 1;
				}
			}
			for (const auto& delim : g_Delimiters) {
				if (std::strstr(delim.data(), m_CToken.text.data())) {
					cdelim = true;
					break;
				}
			}
			for (const auto& op : g_Operators) {
				if (std::strstr(op.data(), m_CToken.text.data())) {
					cop = true;
					break;
				}
			}
			if (cop && cdelim) {
			} else if (cop) {
				m_State = State::OPERATOR;
				return 0;
			} else if (cdelim) {
				m_State = State::DELIMITER;
				return 0;
			} else if (m_CToken.text.length() == 1) {
				m_State = State::BAD;
				return 0;
			} else {
				m_CToken.subtype = PuncType::INVALID;
				m_State = State::COMPLETE_TOKEN;
				return 0;
			}
			m_CToken.text.push_back(c);
			return 1;
		}

		case State::OPERATOR: {
			bool cop = false;
			for (const auto& op : g_Operators) {
				if (std::strstr(op.data(), m_CToken.text.data())) {
					cop = true;
					break;
				}
			}
			if (!cop && (m_CToken.text.length() == 1)) {
				m_State = State::BAD;
				return 0;
			} else if (cop) {
				m_CToken.subtype = PuncType::OPERATOR;
				m_State = State::COMPLETE_TOKEN;
				return 0;
			}
			m_CToken.text.push_back(c);
			return 1;
		}

		case State::DELIMITER: {
			bool cdelim = false;
			for (const auto& delim : g_Delimiters) {
				if (std::strstr(delim.data(), m_CToken.text.data())) {
					cdelim = true;
					break;
				}
			}
			if (!cdelim && (m_CToken.text.length() == 1)) {
				m_State = State::BAD;
				return 0;
			} else if (cdelim) {
				m_CToken.subtype = PuncType::DELIMITER;
				m_State = State::COMPLETE_TOKEN;
				return 0;
			}
			m_CToken.text.push_back(c);
			return 1;
		}

		case State::IDENTIFIER:
			if (!std::isalnum(c)) {
				std::string_view text(m_CToken.text);
				auto it = std::find(g_KeywordHashes.begin(),
						   g_KeywordHashes.end(),
						   Hash(text));
				if (it == g_KeywordHashes.end()) {
					m_CToken.subtype = IdentifierType::SYMBOL;
				} else {
					m_CToken.subtype = IdentifierType::KEYWORD;
				}
				m_State = State::COMPLETE_TOKEN;
				return 0;
			}
			m_CToken.text.push_back(c);
			return 1;

		case State::LINE_COMMENT:
			if (c == '\n') {
				m_CToken.subtype = CommentType::LINE;
				m_CToken.type = TokenType::COMMENT;
				m_State = State::COMPLETE_TOKEN;
				return 0;
			}
			m_CToken.text.push_back(c);
			return 1;

		case State::BLOCK_COMMENT: {
			m_CToken.text.push_back(c);
			auto off = (m_CToken.text.length() - 2); 
			if (std::strncmp(m_CToken.text.data() + off, "*/", 2) == 0) {
				m_CToken.subtype = CommentType::BLOCK;
				m_CToken.type = TokenType::COMMENT;
				m_State = State::COMPLETE_TOKEN;
			}
			return 1;
		}

		case State::COMPLETE_TOKEN:
			m_Tokens.emplace_back(std::move(m_CToken));
			m_CToken = Token();
			m_State = State::NEW_TOKEN;
			return 0;

		default:;
	}
	throw std::logic_error(fmt::format("invalid transition from {} with '{}'",
						   m_State, c));
}
}


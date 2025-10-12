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


template<>
struct fmt::formatter<skyfauna::Lexer::State> : fmt::formatter<std::string> {
	auto format(skyfauna::Lexer::State t, format_context& ctx) const -> decltype(ctx.out())
	{
		using namespace std::literals;
		std::string_view name;
		switch (t) {
			case skyfauna::Lexer::State::BAD:
				name = "BAD"sv;
				break;

			case skyfauna::Lexer::State::NEW_TOKEN:
				name = "NEW_TOKEN"sv;
				break;

			case skyfauna::Lexer::State::NUMERIC_LITERAL:
				name = "NUMERIC_LITERAL"sv;
				break;

			case skyfauna::Lexer::State::STRING_LITERAL:
				name = "STRING_LITERAL"sv;
				break;

			case skyfauna::Lexer::State::CHAR_LITERAL:
				name = "CHAR_LITERAL"sv;
				break;

			case skyfauna::Lexer::State::OPERATOR:
				name = "OPERATOR"sv;
				break;

			case skyfauna::Lexer::State::DELIMITER:
				name = "DELIMITER"sv;
				break;

			case skyfauna::Lexer::State::IDENTIFIER:
				name = "IDENTIFIER"sv;
				break;

			case skyfauna::Lexer::State::LINE_COMMENT:
				name = "LINE_COMMENT"sv;
				break;

			case skyfauna::Lexer::State::BLOCK_COMMENT:
				name = "BLOCK_COMMENT"sv;
				break;

			case skyfauna::Lexer::State::COMPLETE_TOKEN:
				name = "COMPLETE_TOKEN"sv;
				break;

			case skyfauna::Lexer::State::EOF_TOKEN:
				name = "EOF_TOKEN"sv;
				break;

			default:
				throw std::runtime_error("fmt: invalid State");
		}
		return std::copy(name.begin(), name.end(), ctx.out());
	}
};

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
	m_CodeIt = m_Code.cbegin();
	for (; m_CodeIt != m_Code.cend();) {
		auto c = *m_CodeIt;
		a = Transition(c);
		
		// This is not in the for to conditionally avoid incrementing in
		// steps that don't consume characters, using `continue`
		std::advance(m_CodeIt, a);
	}
	while (m_State != State::BAD && m_State != State::NEW_TOKEN)
		if (Transition('\0') != 0)
			break;
	if (m_CToken.text().length() > 0) {
		// Sets the category to invalid
		m_CToken.type() &= AnySpecificTokenTypeMask();
		m_Tokens.emplace_back(std::move(m_CToken));
	}
	m_Tokens.emplace_back(TTPunctuator::DELIMITER, "EOF");

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
				m_CToken.text().push_back(c);
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
			if (std::isalpha(c) || c == '_') {
				m_CToken.text().push_back(c);
				m_State = State::IDENTIFIER;
				m_CToken.type() &= CastTokenType<TokenType>(TokenCategory::IDENTIFIER);
				m_RecoveryFunc = [](char c){ return !std::isalnum(c); };
			} else if (std::isdigit(c)) {
				m_CToken.text().push_back(c);
				m_State = State::NUMERIC_LITERAL;
				m_CToken.type() &= CastTokenType<TokenType>(TokenCategory::LITERAL);
				m_RecoveryFunc = [](char c){
					return !(std::isalnum(c) || c == '.');
				};
			} else if (c == '\'') {
				m_CToken.text().push_back(c);
				m_State = State::CHAR_LITERAL;
				m_CToken.type() = CastTokenType<TokenType>(TTLiteral::CHAR);
				m_RecoveryFunc = [](char c){
					return (std::ispunct(c) || std::isspace(c));
				};
			} else if (c == '\"') {
				m_CToken.text().push_back(c);
				m_State = State::STRING_LITERAL;
				m_CToken.type() = CastTokenType<TokenType>(TTLiteral::STRING);
				m_RecoveryFunc = [](char c){ return std::iscntrl(c); };
			} else if (std::ispunct(c)) {
				m_State = State::PUNCTUATOR;
				m_CToken.type() &= CastTokenType<TokenType>(TokenCategory::PUNCTUATOR);
				if (c == '#')
					m_RecoveryFunc = [](char c){ return std::iscntrl(c); };
				else
					m_RecoveryFunc = [](char c){ (void)c; return true; };
				return 0;
			} else {
				m_CToken.text().push_back(c);
				m_State = State::BAD;
				m_CToken.type() &= CastTokenType<TokenType>(TokenCategory::INVALID);
				m_RecoveryFunc = [](char c){ return std::isprint(c); };
			}
			return 1;

		case State::NUMERIC_LITERAL:
			if (c == '.') {
				if (m_CToken.text().find('.') != m_CToken.text().npos)
					m_State = State::BAD;
			} else if (std::isdigit(c)) {
			} else if (std::isalpha(c)) {
				m_State = State::BAD;
			} else {
				if (m_CToken.text().find('.') != m_CToken.text().npos) {
					m_CToken.type() = CastTokenType<TokenType>(TTLiteral::FLOATING_POINT);
					m_CToken.value() = std::make_any<float>(std::atof(m_CToken.text().c_str()));
				} else {
					m_CToken.type() = CastTokenType<TokenType>(TTLiteral::INTEGER);
					m_CToken.value() = std::make_any<int>(std::atoi(m_CToken.text().c_str()));
				}
				m_State = Lexer::State::COMPLETE_TOKEN;
				return 0;
			}
			m_CToken.text().push_back(c);
			return 1;

		case State::STRING_LITERAL:
			if (c == '\"') {
				m_State = State::COMPLETE_TOKEN;
			} else if (c == '\n') {
				m_State = State::BAD;
				return 0;
			}
			m_CToken.text().push_back(c);
			return 1;

		case State::CHAR_LITERAL:
			if (c == '\'') {
				m_State = State::COMPLETE_TOKEN;
			} else if (c == '\n') {
				m_State = State::BAD;
				return 0;
			}
			m_CToken.text().push_back(c);
			return 1;

		case State::PREPROCESSOR_DIRECTIVE:
			if (c == '\\' && Peek(1) == '\n') {
				return 2;
			} else if (c == '\n') {
				m_State = State::COMPLETE_TOKEN;
				return 0;
			}
			m_CToken.text().push_back(c);
			return 1;

		case State::PUNCTUATOR: {
			char cn = 0, cn2 = 0;
			int adv = 0;
			try {
				cn = Peek(1);
				cn2 = Peek(2);
			} catch(const std::range_error& e) {
				SF_TRACE("Lexer::Tokenize() look-ahead failed {}", e.what());
			}
			switch (c) {
				case '!':
					m_CToken.type() = CastTokenType<TokenType>(TTPunctuator::OPERATOR);
					m_CToken.text().push_back(c);
					m_State = State::COMPLETE_TOKEN;
					return 1;

				case '#':
					m_CToken.type() = CastTokenType<TokenType>(TokenCategory::PREPROCESSOR_DIRECTIVE);
					m_CToken.text().push_back(c);
					m_State = State::PREPROCESSOR_DIRECTIVE;
					return 1;

				case '(': // fall through
				case ')': // fall through
				case ':': // fall through
				case ';': // fall through
				case '[': // fall through
				case ']': // fall through
				case '{': // fall through
				case '}':
				case ',':
					m_CToken.type() = CastTokenType<TokenType>(TTPunctuator::DELIMITER);
					m_CToken.text().push_back(c);
					m_State = State::COMPLETE_TOKEN;
					return 1;

				case '%': // fall through
				case '*': // fall through
				case '=': // fall through
				case '^':
					adv = 1;
					m_CToken.type() = CastTokenType<TokenType>(TTPunctuator::OPERATOR);
					m_CToken.text().push_back(c);
					m_State = State::COMPLETE_TOKEN;
					if (cn == '=') {
						++adv;
						m_CToken.text().push_back(cn);
					}
					return adv;

				case '&':
					adv = 1;
					m_CToken.type() = CastTokenType<TokenType>(TTPunctuator::OPERATOR);
					m_CToken.text().push_back(c);
					m_State = State::COMPLETE_TOKEN;
					if (cn == '=' || cn == '&') {
						++adv;
						m_CToken.text().push_back(cn);
					}
					return adv;

				case '|':
					adv = 1;
					m_CToken.type() = CastTokenType<TokenType>(TTPunctuator::OPERATOR);
					m_CToken.text().push_back(c);
					m_State = State::COMPLETE_TOKEN;
					if (cn == '=' || cn == '|') {
						++adv;
						m_CToken.text().push_back(cn);
					}
					return adv;

				case '+':
					adv = 1;
					m_CToken.type() = CastTokenType<TokenType>(TTPunctuator::OPERATOR);
					m_CToken.text().push_back(c);
					m_State = State::COMPLETE_TOKEN;
					if (cn == '=' || cn == '+') {
						++adv;
						m_CToken.text().push_back(cn);
					}
					return adv;

				case '-':
					adv = 1;
					m_CToken.type() = CastTokenType<TokenType>(TTPunctuator::OPERATOR);
					m_CToken.text().push_back(c);
					m_State = State::COMPLETE_TOKEN;
					if (cn == '=' || cn == '-' || cn == '>') {
						++adv;
						m_CToken.text().push_back(cn);
					}
					return adv;

				case '/':
					adv = 1;
					m_CToken.type() = CastTokenType<TokenType>(TTPunctuator::OPERATOR);
					m_CToken.text().push_back(c);
					m_State = State::COMPLETE_TOKEN;
					if (cn == '=') {
						++adv;
						m_CToken.text().push_back(cn);
					} else if (cn == '/') {
						m_CToken.type() = CastTokenType<TokenType>(TTComment::LINE);
						++adv;
						m_CToken.text().push_back(cn);
						m_State = State::LINE_COMMENT;
					} else if (cn == '*') {
						m_CToken.type() = CastTokenType<TokenType>(TTComment::BLOCK);
						++adv;
						m_CToken.text().push_back(cn);
						m_State = State::BLOCK_COMMENT;
					}
					return adv;

				case '<':
				case '>':
					adv = 1;
					m_CToken.type() = CastTokenType<TokenType>(TTPunctuator::OPERATOR);
					m_CToken.text().push_back(c);
					m_State = State::COMPLETE_TOKEN;
					if (cn == '=') {
						++adv;
						m_CToken.text().push_back(cn);
					} else if (cn == c) {
						++adv;
						m_CToken.text().push_back(cn);
						if (cn2 == '=') {
							++adv;
							m_CToken.text().push_back(cn);
						}
					}
					return adv;

				case '.':
					adv = 1;
					m_CToken.type() = CastTokenType<TokenType>(TTPunctuator::OPERATOR);
					m_CToken.text().push_back(c);
					m_State = State::COMPLETE_TOKEN;
					if (cn == c && cn2 == c) {
						adv += 2;
						m_CToken.text().append(c, 2);
					}
					return adv;

				default: {
					m_CToken.type() = CastTokenType<TokenType>(TTPunctuator::INVALID);
					m_CToken.text().push_back(c);
					m_State = State::BAD;
					return 1;
				}
			}
		}

		case State::IDENTIFIER:
			if (!std::isalnum(c) && c != '_') {
				std::string_view text(m_CToken.text());
				auto it = std::find(g_KeywordHashes.begin(),
						   g_KeywordHashes.end(),
						   Hash(text));
				if (it == g_KeywordHashes.end()) {
					m_CToken.type() = CastTokenType<TokenType>(TTIdentifier::SYMBOL);
				} else {
					m_CToken.type() = CastTokenType<TokenType>(TTIdentifier::KEYWORD);
				}
				m_State = State::COMPLETE_TOKEN;
				return 0;
			}
			m_CToken.text().push_back(c);
			return 1;

		case State::LINE_COMMENT:
			if (c == '\n') {
				m_CToken.type() = CastTokenType<TokenType>(TTComment::LINE);
				m_State = State::COMPLETE_TOKEN;
				return 0;
			}
			m_CToken.text().push_back(c);
			return 1;

		case State::BLOCK_COMMENT: {
			m_CToken.text().push_back(c);
			auto off = (m_CToken.text().length() - 2); 
			if (std::strncmp(m_CToken.text().data() + off, "*/", 2) == 0) {
				m_CToken.type() = CastTokenType<TokenType>(TTComment::BLOCK);
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


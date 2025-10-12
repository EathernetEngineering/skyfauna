// SPDX-License-Identifier: MIT
// Copyright (C) 2025 Chloe Eather

#ifndef SKYFAUNA_LEX_H_
#define SKYFAUNA_LEX_H_

#include <skyfauna/common/util.h>

#include <any>
#include <concepts>
#include <functional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include <cstdint>

#include <spdlog/fmt/fmt.h>

namespace skyfauna {
typedef uint64_t TokenTypesBase;
enum class TokenType : TokenTypesBase {
	INVALID = 0,
	IDENTIFIER = BIT(32),
	LITERAL = BIT(33),
	PUNCTUATOR = BIT(34),
	PREPROCESSOR_DIRECTIVE = BIT(35),
	COMMENT = BIT(36)
};

enum class IdentifierType : TokenTypesBase {
	INVALID = 0,
	KEYWORD = BIT(0),
	SYMBOL = BIT(1)
};

enum class LiteralType : TokenTypesBase {
	INVALID = 0,
	INTEGER = BIT(4),
	FLOATING_POINT = BIT(5),
	STRING = BIT(6),
	CHAR = BIT(7)
};

enum class PuncType : TokenTypesBase {
	INVALID = 0,
	OPERATOR = BIT(8),
	DELIMITER = BIT(9)
};

enum class CommentType : TokenTypesBase {
	INVALID = 0,
	LINE = BIT(12),
	BLOCK = BIT(13)
};

template<typename ST>
concept TokenSubtype = std::same_as<ST, IdentifierType> ||
	std::same_as<ST, LiteralType> ||
	std::same_as<ST, PuncType> ||
	std::same_as<ST, CommentType>;

template<typename To, typename From>
requires (TokenSubtype<To> || std::same_as<To, TokenTypesBase>) &&
	(TokenSubtype<From> || std::same_as<From, TokenTypesBase>)
constexpr To ConvertSubtype(From other) {
	return static_cast<To>(static_cast<TokenTypesBase>(other));
}

struct Token {
	skyfauna::TokenType type = skyfauna::TokenType::INVALID;
	TokenTypesBase subtype =
		{ConvertSubtype<TokenTypesBase>(IdentifierType::INVALID)};
	std::string text;
	std::any value;

	Token()
	 : type(skyfauna::TokenType::INVALID),
		subtype(ConvertSubtype<TokenTypesBase>(IdentifierType::INVALID)),
		text(std::move([](){ std::string s; s.reserve(16); return s; }()))
	{}

	template<typename ST>
	requires TokenSubtype<ST>
	Token(skyfauna::TokenType type, ST subtype, std::string text)
	 : type(type), subtype(ConvertSubtype<TokenTypesBase, ST>(subtype)),
		text(std::move(text))
	{}

	template<typename ST, typename T>
	requires TokenSubtype<ST>
	Token(skyfauna::TokenType type, ST subtype, std::string text, T value)
	 : type(type), subtype(ConvertSubtype<TokenTypesBase, ST>(subtype)),
		text(std::move(text)), value(std::make_any<T>(std::move(value)))
	{}

	Token(Token&&) noexcept = default;
	Token& operator=(Token&&) noexcept = default;
	Token(const Token&) = default;
	Token& operator=(const Token&) = default;
};

template<typename ST>
requires TokenSubtype<ST> || std::same_as<ST, TokenTypesBase>
constexpr ST GetTokenSubtype(const Token& t) noexcept {
	switch (t.type) {
		case skyfauna::TokenType::IDENTIFIER:
			return ConvertSubtype<ST>(t.subtype);

		case skyfauna::TokenType::LITERAL:
			return ConvertSubtype<ST>(t.subtype);

		case skyfauna::TokenType::PUNCTUATOR:
			return ConvertSubtype<ST>(t.subtype);

		case skyfauna::TokenType::PREPROCESSOR_DIRECTIVE:
			return ConvertSubtype<ST>(t.subtype);

		case skyfauna::TokenType::COMMENT:
			return ConvertSubtype<ST>(t.subtype);

		default:
			return ConvertSubtype<ST>(0u);
	}
}
}

template<>
struct fmt::formatter<skyfauna::TokenType> : fmt::formatter<std::string> {
	auto format(const skyfauna::TokenType& ls, format_context& ctx) const -> decltype(ctx.out())
	{
		using namespace std::literals;
		std::string_view name;
		switch (ls) {
			case skyfauna::TokenType::INVALID:
				name = "INVALID"sv;
				break;

			case skyfauna::TokenType::IDENTIFIER:
				name = "IDENTIFIER"sv;
				break;

			case skyfauna::TokenType::LITERAL:
				name = "LITERAL"sv;
				break;

			case skyfauna::TokenType::PUNCTUATOR:
				name = "PUNCTUATOR"sv;
				break;

			case skyfauna::TokenType::PREPROCESSOR_DIRECTIVE:
				name = "PREPROCESSOR DIRECTIVE"sv;
				break;

			case skyfauna::TokenType::COMMENT:
				name = "COMMENT"sv;
				break;

			default:
				throw std::runtime_error("fmt: invalid skyfauna::TokenType");
		}
		return fmt::format_to(ctx.out(), "{}", name);
	}
};

template<>
struct fmt::formatter<skyfauna::IdentifierType> : fmt::formatter<std::string> {
	auto format(const skyfauna::IdentifierType& ls, format_context& ctx) const -> decltype(ctx.out())
	{
		using namespace std::literals;
		std::string_view name;
		switch (ls) {
			case skyfauna::IdentifierType::INVALID:
				name = "INVALID"sv;
				break;

			case skyfauna::IdentifierType::KEYWORD:
				name = "KEYWORD"sv;
				break;

			case skyfauna::IdentifierType::SYMBOL:
				name = "SYMBOL"sv;
				break;

			default:
				throw std::runtime_error("fmt: invalid skyfauna::TokenType");
		}
		return std::copy(name.begin(), name.end(), ctx.out());
		return fmt::format_to(ctx.out(), "{}", name);
	}
};

template<>
struct fmt::formatter<skyfauna::LiteralType> : fmt::formatter<std::string> {
	auto format(const skyfauna::LiteralType& ls, format_context& ctx) const -> decltype(ctx.out())
	{
		using namespace std::literals;
		std::string_view name;
		switch (ls) {
			case skyfauna::LiteralType::INVALID:
				name = "INVALID"sv;
				break;

			case skyfauna::LiteralType::INTEGER:
				name = "INTEGERsv";
				break;

			case skyfauna::LiteralType::FLOATING_POINT:
				name = "FLOATING_POINT"sv;
				break;

			case skyfauna::LiteralType::CHAR:
				name = "CHAR"sv;
				break;

			case skyfauna::LiteralType::STRING:
				name = "STRING"sv;
				break;

			default:
				throw std::runtime_error("fmt: invalid skyfauna::TokenType");
		}
		return std::copy(name.begin(), name.end(), ctx.out());
	}
};

template<>
struct fmt::formatter<skyfauna::PuncType> : fmt::formatter<std::string> {
	auto format(const skyfauna::PuncType& ls, format_context& ctx) const -> decltype(ctx.out())
	{
		using namespace std::literals;
		std::string_view name;
		switch (ls) {
			case skyfauna::PuncType::INVALID:
				name = "INVALID"sv;
				break;

			case skyfauna::PuncType::OPERATOR:
				name = "OPERATOR"sv;
				break;

			case skyfauna::PuncType::DELIMITER:
				name = "DELIMITER"sv;
				break;

			default:
				throw std::runtime_error("fmt: invalid skyfauna::TokenType");
		}
		return std::copy(name.begin(), name.end(), ctx.out());
	}
};

template<>
struct fmt::formatter<skyfauna::CommentType> : fmt::formatter<std::string> {
	auto format(const skyfauna::CommentType& ls, format_context& ctx) const -> decltype(ctx.out())
	{
		using namespace std::literals;
		std::string_view name;
		switch (ls) {
			case skyfauna::CommentType::INVALID:
				name = "INVALID"sv;
				break;

			case skyfauna::CommentType::LINE:
				name = "LINE_COMMENT"sv;
				break;

			case skyfauna::CommentType::BLOCK:
				name = "BLOCK_COMMENT"sv;
				break;

			default:
				throw std::runtime_error("fmt: invalid skyfauna::TokenType");
		}
		return std::copy(name.begin(), name.end(), ctx.out());
	}
};

template<>
struct fmt::formatter<skyfauna::Token> : fmt::formatter<std::string> {
	auto format(const skyfauna::Token& tok, format_context& ctx) const -> decltype(ctx.out())
	{
		switch (tok.type) {
			case skyfauna::TokenType::IDENTIFIER:
				return fmt::format_to(ctx.out(), "TOKEN: [TYPE]: {} [SUBTYPE]: {} [TEXT]: {}",
					 tok.type,
					 skyfauna::ConvertSubtype<skyfauna::IdentifierType>(tok.subtype),
					 tok.text);

			case skyfauna::TokenType::LITERAL:
				return fmt::format_to(ctx.out(), "TOKEN: [TYPE]: {} [SUBTYPE]: {} [TEXT]: {}",
					 tok.type,
					 skyfauna::ConvertSubtype<skyfauna::LiteralType>(tok.subtype),
					 tok.text);

			case skyfauna::TokenType::PUNCTUATOR:
				return fmt::format_to(ctx.out(), "TOKEN: [TYPE]: {} [SUBTYPE]: {} [TEXT]: {}",
					 tok.type,
					 skyfauna::ConvertSubtype<skyfauna::PuncType>(tok.subtype),
					 tok.text);

			case skyfauna::TokenType::PREPROCESSOR_DIRECTIVE:
				return fmt::format_to(ctx.out(), "TOKEN: [TYPE]: {} [TEXT]: {}",
					 tok.type, tok.text);

			case skyfauna::TokenType::COMMENT:
				return fmt::format_to(ctx.out(), "TOKEN: [TYPE]: {} [SUBTYPE]: {} [TEXT]: {}",
					 tok.type,
					 skyfauna::ConvertSubtype<skyfauna::CommentType>(tok.subtype),
					 tok.text);

			default:
				return fmt::format_to(ctx.out(), "TOKEN: [TYPE]: {} [TEXT]: {}",
					 tok.type, tok.text);
		}
	}
};

namespace skyfauna {
class Lexer {
private:
	enum class State : uint16_t {
		BAD = 0,
		NEW_TOKEN,
		NUMERIC_LITERAL,
		STRING_LITERAL,
		CHAR_LITERAL,
		PREPROCESSOR_DIRECTIVE,
		PUNCTUATOR,
		OPERATOR,
		DELIMITER,
		IDENTIFIER,
		LINE_COMMENT,
		BLOCK_COMMENT,
		EOF_TOKEN,
		COMPLETE_TOKEN
	};

public:
	Lexer() noexcept = default;
	~Lexer() noexcept = default;
	Lexer(Lexer&&) noexcept = default;
	Lexer& operator=(Lexer&&) noexcept = default;
	Lexer(std::string&& code) noexcept;

	std::vector<Token>& Tokenize();

	inline std::vector<Token>& GetTokens() noexcept { return m_Tokens; }
	inline const std::vector<Token>& GetTokens() const noexcept {
		return m_Tokens;
	}
	inline std::vector<Token> TakeTokens() noexcept {
		return std::exchange(m_Tokens, {});
	}

	inline void Reset() noexcept { *this = Lexer(); }
	inline void Reset(std::string&& code) noexcept {
		*this = std::move(Lexer(std::move(code)));
	}
	inline void SetCode(std::string&& code) noexcept { Reset(); }

	inline std::vector<Token> GetTokensAndReset() noexcept {
		auto tmp(std::move(m_Tokens));
		Reset();
		return tmp;
	}

private:
	int Transition(char c);
	inline char Peek(int distance) {
		auto pos = m_CodeIt + distance;
		if (pos >= m_Code.cend())
			throw std::range_error(fmt::format("Lexer::Peek: it + distance "
									  "(which is {}) >= m_Code.size() (which is {})",
									  std::distance(m_Code.cbegin(), pos), m_Code.size()));
		return *pos;
	}

private:
	std::string m_Code;
	std::vector<Token> m_Tokens;
	State m_State;
	Token m_CToken;
	std::string::const_iterator m_CodeIt;
	std::function<bool(char)> m_RecoveryFunc;

	friend struct fmt::formatter<State>;
};
}

#endif


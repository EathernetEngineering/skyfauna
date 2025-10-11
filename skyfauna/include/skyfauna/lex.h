// SPDX-License-Identifier: MIT
// Copyright (C) 2025 Chloe Eather

#ifndef SKYFAUNA_LEX_H_
#define SKYFAUNA_LEX_H_

#include <any>
#include <concepts>
#include <functional>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>
#include <vector>
#include <cstdint>

#include <spdlog/fmt/fmt.h>

namespace skyfauna {
enum class TokenType : uint32_t {
	INVALID = 0,
	IDENTIFIER,
	LITERAL,
	PUNCTUATOR,
	PREPROCESSOR,
	COMMENT,
};

enum class IdentifierType : uint32_t {
	INVALID = 0,
	KEYWORD,
	SYMBOL
};

enum class LiteralType : uint32_t {
	INVALID = 0,
	INTEGER,
	FLOATING_POINT,
	STRING,
	CHAR
};

enum class PuncType : uint32_t {
	INVALID = 0,
	OPERATOR,
	DELIMITER
};

enum class CommentType : uint32_t {
	INVALID = 0,
	LINE,
	BLOCK
};

template<typename ST>
concept TokenSubtype = std::same_as<ST, IdentifierType> ||
	std::same_as<ST, LiteralType> ||
	std::same_as<ST, PuncType> ||
	std::same_as<ST, CommentType>;

struct Token {
	skyfauna::TokenType type = skyfauna::TokenType::INVALID;
	std::variant<IdentifierType, LiteralType, PuncType, CommentType> subtype =
		{IdentifierType::INVALID};
	std::string text;
	std::any value;

	Token()
	 : type(skyfauna::TokenType::INVALID), subtype(IdentifierType::INVALID),
		text(std::move([](){ std::string s; s.reserve(16); return s; }()))
	{}

	template<typename ST>
	requires TokenSubtype<ST>
	Token(skyfauna::TokenType type, ST subtype, std::string text)
	 : type(type), subtype(subtype), text(std::move(text))
	{}

	template<typename ST, typename T>
	requires TokenSubtype<ST>
	Token(skyfauna::TokenType type, ST subtype, std::string text, T value)
	 : type(type), subtype(subtype), text(std::move(text)),
		value(std::make_any<T>(std::move(value)))
	{}

	Token(Token&&) noexcept = default;
	Token& operator=(Token&&) noexcept = default;
	Token(const Token&) = default;
	Token& operator=(const Token&) = default;
};
}

namespace fmt {
template<>
struct formatter<skyfauna::TokenType> : formatter<std::string> {
	auto format(skyfauna::TokenType ls, format_context& ctx) const -> decltype(ctx.out())
	{
		const char *name;
		switch (ls) {
			case skyfauna::TokenType::INVALID:
				name = "INVALID";
				break;

			case skyfauna::TokenType::IDENTIFIER:
				name = "IDENTIFIER";
				break;

			case skyfauna::TokenType::LITERAL:
				name = "LITERAL";
				break;

			case skyfauna::TokenType::PUNCTUATOR:
				name = "PUNCTUATOR";
				break;

			case skyfauna::TokenType::PREPROCESSOR:
				name = "PREPROCESSOR";
				break;

			case skyfauna::TokenType::COMMENT:
				name = "COMMENT";
				break;

			default:
				throw std::runtime_error("fmt: invalid skyfauna::TokenType");
		}
		return format_to(ctx.out(), "{}", name);
	}
};

template<>
struct formatter<skyfauna::IdentifierType> : formatter<std::string> {
	auto format(skyfauna::IdentifierType ls, format_context& ctx) const -> decltype(ctx.out())
	{
		const char *name;
		switch (ls) {
			case skyfauna::IdentifierType::INVALID:
				name = "INVALID";
				break;

			case skyfauna::IdentifierType::KEYWORD:
				name = "KEYWORD";
				break;

			case skyfauna::IdentifierType::SYMBOL:
				name = "SYMBOL";
				break;

			default:
				throw std::runtime_error("fmt: invalid skyfauna::TokenType");
		}
		return format_to(ctx.out(), "{}", name);
	}
};

template<>
struct formatter<skyfauna::LiteralType> : formatter<std::string> {
	auto format(skyfauna::LiteralType ls, format_context& ctx) const -> decltype(ctx.out())
	{
		const char *name;
		switch (ls) {
			case skyfauna::LiteralType::INVALID:
				name = "INVALID";
				break;

			case skyfauna::LiteralType::INTEGER:
				name = "INTEGER";
				break;

			case skyfauna::LiteralType::FLOATING_POINT:
				name = "FLOATING_POINT";
				break;

			case skyfauna::LiteralType::CHAR:
				name = "CHAR";
				break;

			case skyfauna::LiteralType::STRING:
				name = "STRING";
				break;

			default:
				throw std::runtime_error("fmt: invalid skyfauna::TokenType");
		}
		return format_to(ctx.out(), "{}", name);
	}
};

template<>
struct formatter<skyfauna::PuncType> : formatter<std::string> {
	auto format(skyfauna::PuncType ls, format_context& ctx) const -> decltype(ctx.out())
	{
		const char *name;
		switch (ls) {
			case skyfauna::PuncType::INVALID:
				name = "INVALID";
				break;

			case skyfauna::PuncType::OPERATOR:
				name = "OPERATOR";
				break;

			case skyfauna::PuncType::DELIMITER:
				name = "DELIMITER";
				break;

			default:
				throw std::runtime_error("fmt: invalid skyfauna::TokenType");
		}
		return format_to(ctx.out(), "{}", name);
	}
};

template<>
struct formatter<skyfauna::CommentType> : formatter<std::string> {
	auto format(skyfauna::CommentType ls, format_context& ctx) const -> decltype(ctx.out())
	{
		const char *name;
		switch (ls) {
			case skyfauna::CommentType::INVALID:
				name = "INVALID";
				break;

			case skyfauna::CommentType::LINE:
				name = "LINE_COMMENT";
				break;

			case skyfauna::CommentType::BLOCK:
				name = "BLOCK_COMMENT";
				break;

			default:
				throw std::runtime_error("fmt: invalid skyfauna::TokenType");
		}
		return format_to(ctx.out(), "{}", name);
	}
};

template<>
struct formatter<skyfauna::Token> : formatter<std::string> {
	auto format(const skyfauna::Token& tok, format_context& ctx) const -> decltype(ctx.out())
	{
		switch (tok.type) {
			case skyfauna::TokenType::IDENTIFIER:
				return format_to(ctx.out(), "TOKEN: [TYPE]: {} [SUBTYPE]: {} [TEXT]: {} ",
					 tok.type, std::get<skyfauna::IdentifierType>(tok.subtype), tok.text);

			case skyfauna::TokenType::PUNCTUATOR:
				return format_to(ctx.out(), "TOKEN: [TYPE]: {} [SUBTYPE]: {} [TEXT]: {} ",
					 tok.type, std::get<skyfauna::PuncType>(tok.subtype), tok.text);

			case skyfauna::TokenType::LITERAL:
				return format_to(ctx.out(), "TOKEN: [TYPE]: {} [SUBTYPE]: {} [TEXT]: {} ",
					 tok.type, std::get<skyfauna::LiteralType>(tok.subtype), tok.text);

			case skyfauna::TokenType::COMMENT:
				return format_to(ctx.out(), "TOKEN: [TYPE]: {} [SUBTYPE]: {} [TEXT]: {} ",
					 tok.type, std::get<skyfauna::CommentType>(tok.subtype), tok.text);

			default:
				return format_to(ctx.out(), "TOKEN: [TYPE]: {} [TEXT]: {} ",
					 tok.type, tok.text);
		}
	}
};
}

namespace skyfauna {
class Lexer {
private:
	enum class State : uint16_t {
		BAD = 0,
		NEW_TOKEN,
		NUMERIC_LITERAL,
		STRING_LITERAL,
		CHAR_LITERAL,
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

private:
	std::string m_Code;
	std::vector<Token> m_Tokens;
	State m_State;
	Token m_CToken;
	std::function<bool(char)> m_RecoveryFunc;

	friend struct fmt::formatter<State>;
};
}

#endif


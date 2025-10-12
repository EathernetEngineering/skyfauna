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
// Categories use upper 32 bits and specific types use lower 32 bits
typedef uint64_t TokenType;

enum class TokenCategory : TokenType {
	INVALID = 0,
	IDENTIFIER = BIT(32),
	LITERAL = BIT(33),
	PUNCTUATOR = BIT(34),
	PREPROCESSOR_DIRECTIVE = BIT(35),
	COMMENT = BIT(36)
};

enum class TTIdentifier : TokenType {
	INVALID = static_cast<TokenType>(TokenCategory::IDENTIFIER),
	KEYWORD = BIT(0) | static_cast<TokenType>(TokenCategory::IDENTIFIER),
	SYMBOL = BIT(1) | static_cast<TokenType>(TokenCategory::IDENTIFIER)
};

enum class TTLiteral : TokenType {
	INVALID = static_cast<TokenType>(TokenCategory::LITERAL),
	INTEGER = BIT(0) | static_cast<TokenType>(TokenCategory::LITERAL),
	FLOATING_POINT = BIT(1) | static_cast<TokenType>(TokenCategory::LITERAL),
	STRING = BIT(2) | static_cast<TokenType>(TokenCategory::LITERAL),
	CHAR = BIT(3) | static_cast<TokenType>(TokenCategory::LITERAL)
};

enum class TTPuncuator : TokenType {
	INVALID = static_cast<TokenType>(TokenCategory::PUNCTUATOR),
	OPERATOR = BIT(0) | static_cast<TokenType>(TokenCategory::PUNCTUATOR),
	DELIMITER = BIT(1) | static_cast<TokenType>(TokenCategory::PUNCTUATOR)
};

enum class TTComment : TokenType {
	INVALID = static_cast<TokenType>(TokenCategory::COMMENT),
	LINE = BIT(0) | static_cast<TokenType>(TokenCategory::COMMENT),
	BLOCK = BIT(1) | static_cast<TokenType>(TokenCategory::COMMENT)
};

template<typename T>
concept IsTokenCategory = std::same_as<T, TokenCategory>;

template<typename T>
concept IsSpecificTokenType = std::same_as<T, TTIdentifier> ||
	std::same_as<T, TTLiteral> ||
	std::same_as<T, TTPuncuator> ||
	std::same_as<T, TTComment>;

template<typename T>
concept IsTokenType = IsTokenCategory<T> ||
	IsSpecificTokenType<T> || std::same_as<T, TokenType>;

consteval TokenType AnySpecificTokenTypeMask() { return 0xFFFFFFFFull; }

template<typename T>
requires IsTokenType<T>
consteval TokenType TokenCategoryMask() {
	return static_cast<TokenType>(T::INVALID);
}

template<>
consteval TokenType TokenCategoryMask<TokenType>() {
	return 0ull;
}

consteval TokenType AnyTokenCategoryMask() { return 0xFFFFFFFF00000000ull; }

template<typename T, typename U>
requires IsTokenType<T> && IsTokenType<U>
consteval bool TokenTypesCompatible() {
	if (AnyTokenCategoryMask() & TokenCategoryMask<T>() &&
		AnyTokenCategoryMask() & TokenCategoryMask<T>())
		return TokenCategoryMask<T>() & TokenCategoryMask<U>();
	else if (AnyTokenCategoryMask() & TokenCategoryMask<T>())
		return TokenCategoryMask<T>() & AnyTokenCategoryMask();
	else if (AnyTokenCategoryMask() & TokenCategoryMask<U>())
		return AnyTokenCategoryMask() & TokenCategoryMask<U>();
	else 
		return true;
}

template<typename T, typename U>
requires IsTokenType<T> && IsTokenType<U>
constexpr bool TokenTypesCompatible(T lhs, U rhs) {
	if (!(lhs & AnyTokenCategoryMask()))
		lhs &= AnyTokenCategoryMask();
	if (!(rhs & AnyTokenCategoryMask()))
		rhs &= AnyTokenCategoryMask();
	return static_cast<TokenType>(lhs) & static_cast<TokenType>(rhs);
}

template<typename T, typename U>
requires IsTokenType<T> && IsTokenType<U>
constexpr bool TokenTypesCompatible(U rhs) {
	TokenType lhs, rhsBase = static_cast<TokenType>(rhs);
	if constexpr (TokenCategoryMask<T>() & AnyTokenCategoryMask())
		lhs = TokenCategoryMask<T>();
	else
		lhs = AnyTokenCategoryMask();;
	if (!(rhsBase & AnyTokenCategoryMask()))
		rhsBase &= AnyTokenCategoryMask();
	return lhs & rhsBase;
}

template<typename T, typename U>
requires IsTokenType<T> && IsTokenType<U>
constexpr const T& CastTokenType(const U& other) {
	if (!TokenTypesCompatible<T>(other))
		throw std::logic_error(fmt::format(
									 "Invalid cast from type '{}' to type '{}'",
									 typeid(U).name(), typeid(T).name()));
	return *reinterpret_cast<const T*>(&other);
}

class Token {
public:
	Token()
	 : m_Type(0), m_Text(std::move([](){
			std::string s; s.reserve(16); return s;
		}()))
	{}

	template<typename T>
	requires IsTokenType<T>
	Token(T type, std::string text)
	 : m_Type(static_cast<TokenType>(type)), m_Text(std::move(text))
	{}

	template<typename T, typename V>
	requires IsTokenType<T>
	Token(T type, std::string text, V value)
	 : m_Type(static_cast<TokenType>(type)), m_Text(std::move(text)),
		m_Value(std::make_any<V>(value))
	{}

	Token(Token&&) noexcept = default;
	Token& operator=(Token&&) noexcept = default;
	Token(const Token&) = default;
	Token& operator=(const Token&) = default;

	template<typename T = TokenType>
	requires IsTokenType<T>
	T& type() { return m_Type; }
	template<typename T = TokenType>
	requires IsTokenType<T>
	const T& type() const { return CastTokenType<T>(m_Type); }
	std::string& text() { return m_Text; }
	const std::string& text() const { return m_Text; }
	std::any& value() { return m_Value; }
	const std::any& value() const { return m_Value; }

private:
	TokenType m_Type;
	std::string m_Text;
	std::any m_Value;
};

template<typename T>
requires IsSpecificTokenType<T>
constexpr T GetSpecificTokenType(const Token& t) noexcept {
	return CastTokenType<T>(t.type());
}
}

template<>
struct fmt::formatter<skyfauna::TokenCategory> : fmt::formatter<std::string> {
	auto format(const skyfauna::TokenCategory& ls, format_context& ctx) const -> decltype(ctx.out())
	{
		using namespace std::literals;
		std::string_view name;
		switch (ls) {
			case skyfauna::TokenCategory::INVALID:
				name = "INVALID"sv;
				break;

			case skyfauna::TokenCategory::IDENTIFIER:
				name = "IDENTIFIER"sv;
				break;

			case skyfauna::TokenCategory::LITERAL:
				name = "LITERAL"sv;
				break;

			case skyfauna::TokenCategory::PUNCTUATOR:
				name = "PUNCTUATOR"sv;
				break;

			case skyfauna::TokenCategory::PREPROCESSOR_DIRECTIVE:
				name = "PREPROCESSOR DIRECTIVE"sv;
				break;

			case skyfauna::TokenCategory::COMMENT:
				name = "COMMENT"sv;
				break;

			default:
				throw std::runtime_error("fmt: invalid skyfauna::TokenType");
		}
		return fmt::format_to(ctx.out(), "{}", name);
	}
};

template<>
struct fmt::formatter<skyfauna::TTIdentifier> : fmt::formatter<std::string> {
	auto format(const skyfauna::TTIdentifier& ls, format_context& ctx) const -> decltype(ctx.out())
	{
		using namespace std::literals;
		std::string_view name;
		switch (ls) {
			case skyfauna::TTIdentifier::INVALID:
				name = "INVALID"sv;
				break;

			case skyfauna::TTIdentifier::KEYWORD:
				name = "KEYWORD"sv;
				break;

			case skyfauna::TTIdentifier::SYMBOL:
				name = "SYMBOL"sv;
				break;

			default:
				throw std::runtime_error("fmt: invalid skyfauna::TTIdentifier");
		}
		return std::copy(name.begin(), name.end(), ctx.out());
		return fmt::format_to(ctx.out(), "{}", name);
	}
};

template<>
struct fmt::formatter<skyfauna::TTLiteral> : fmt::formatter<std::string> {
	auto format(const skyfauna::TTLiteral& ls, format_context& ctx) const -> decltype(ctx.out())
	{
		using namespace std::literals;
		std::string_view name;
		switch (ls) {
			case skyfauna::TTLiteral::INVALID:
				name = "INVALID"sv;
				break;

			case skyfauna::TTLiteral::INTEGER:
				name = "INTEGERsv";
				break;

			case skyfauna::TTLiteral::FLOATING_POINT:
				name = "FLOATING_POINT"sv;
				break;

			case skyfauna::TTLiteral::CHAR:
				name = "CHAR"sv;
				break;

			case skyfauna::TTLiteral::STRING:
				name = "STRING"sv;
				break;

			default:
				throw std::runtime_error("fmt: invalid skyfauna::TTLiteral");
		}
		return std::copy(name.begin(), name.end(), ctx.out());
	}
};

template<>
struct fmt::formatter<skyfauna::TTPuncuator> : fmt::formatter<std::string> {
	auto format(const skyfauna::TTPuncuator& ls, format_context& ctx) const -> decltype(ctx.out())
	{
		using namespace std::literals;
		std::string_view name;
		switch (ls) {
			case skyfauna::TTPuncuator::INVALID:
				name = "INVALID"sv;
				break;

			case skyfauna::TTPuncuator::OPERATOR:
				name = "OPERATOR"sv;
				break;

			case skyfauna::TTPuncuator::DELIMITER:
				name = "DELIMITER"sv;
				break;

			default:
				throw std::runtime_error("fmt: invalid skyfauna::TTPuncuator");
		}
		return std::copy(name.begin(), name.end(), ctx.out());
	}
};

template<>
struct fmt::formatter<skyfauna::TTComment> : fmt::formatter<std::string> {
	auto format(const skyfauna::TTComment& ls, format_context& ctx) const -> decltype(ctx.out())
	{
		using namespace std::literals;
		std::string_view name;
		switch (ls) {
			case skyfauna::TTComment::INVALID:
				name = "INVALID"sv;
				break;

			case skyfauna::TTComment::LINE:
				name = "LINE_COMMENT"sv;
				break;

			case skyfauna::TTComment::BLOCK:
				name = "BLOCK_COMMENT"sv;
				break;

			default:
				throw std::runtime_error("fmt: invalid skyfauna::TTComment");
		}
		return std::copy(name.begin(), name.end(), ctx.out());
	}
};

template<>
struct fmt::formatter<skyfauna::TokenType> : fmt::formatter<std::string> {
	auto format(const skyfauna::TokenType& type, format_context& ctx) const -> decltype(ctx.out())
	{
		auto mask = type & skyfauna::AnyTokenCategoryMask();
		auto category = skyfauna::CastTokenType<skyfauna::TokenCategory>(mask);
		switch (category) {
			case skyfauna::TokenCategory::IDENTIFIER:
				return fmt::format_to(ctx.out(), "[CATEGORY]: {} [DETAIL]: {}",
						  category, skyfauna::CastTokenType<skyfauna::TTIdentifier>(type));

			case skyfauna::TokenCategory::LITERAL:
				return fmt::format_to(ctx.out(), "[CATEGORY]: {} [DETAIL]: {}",
						  category, skyfauna::CastTokenType<skyfauna::TTIdentifier>(type));

			case skyfauna::TokenCategory::PUNCTUATOR:
				return fmt::format_to(ctx.out(), "[CATEGORY]: {} [DETAIL]: {}",
						  category, skyfauna::CastTokenType<skyfauna::TTIdentifier>(type));

			case skyfauna::TokenCategory::PREPROCESSOR_DIRECTIVE:
				return fmt::format_to(ctx.out(), "[CATEGORY]: {}", category);

			case skyfauna::TokenCategory::COMMENT:
				return fmt::format_to(ctx.out(), "[CATEGORY]: {} [DETAIL]: {}",
						  category, skyfauna::CastTokenType<skyfauna::TTIdentifier>(type));

			default: {
				static constexpr std::string_view msg = "[CATEGORY]: Invalid";
				return std::copy(msg.begin(), msg.end(), ctx.out());
			}
		}
	}
};

template<>
struct fmt::formatter<skyfauna::Token> : fmt::formatter<std::string> {
	auto format(const skyfauna::Token& tok, format_context& ctx) const -> decltype(ctx.out())
	{
		auto mask = tok.type() & skyfauna::AnyTokenCategoryMask();
		auto category = skyfauna::CastTokenType<skyfauna::TokenCategory>(mask);
		switch (category) {
			case skyfauna::TokenCategory::IDENTIFIER:
				return fmt::format_to(ctx.out(), "TOKEN: {} [TEXT]: {}",
						  tok.type(), tok.text());

			case skyfauna::TokenCategory::LITERAL:
				return fmt::format_to(ctx.out(), "TOKEN: {} [TEXT]: {}",
						  tok.type(), tok.text());

			case skyfauna::TokenCategory::PUNCTUATOR:
				return fmt::format_to(ctx.out(), "TOKEN: {} [TEXT]: {}",
						  tok.type(), tok.text());

			case skyfauna::TokenCategory::PREPROCESSOR_DIRECTIVE:
				return fmt::format_to(ctx.out(), "TOKEN: {} [TEXT]: {}",
						  tok.type(), tok.text());

			case skyfauna::TokenCategory::COMMENT:
				return fmt::format_to(ctx.out(), "TOKEN: {} [TEXT]: {}",
						  tok.type(), tok.text());

			default:
				return fmt::format_to(ctx.out(), "TOKEN: {} [TEXT]: {}",
						  tok.type(), tok.text());
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


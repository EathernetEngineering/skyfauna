// SPDX-License-Identifier: MIT
// Copyright (C) 2025 Chloe Eather

#ifndef SKYFAUNA_LEX_H_
#define SKYFAUNA_LEX_H_

#include <skyfauna/common/set.h>
#include <skyfauna/common/util.h>

#include <any>
#include <functional>
#include <string>
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

enum class TTPunctuator : TokenType {
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
	std::same_as<T, TTPunctuator> ||
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
	if (lhs & AnyTokenCategoryMask())
		lhs &= AnyTokenCategoryMask();
	else
		lhs = AnyTokenCategoryMask();
	if (rhs & AnyTokenCategoryMask())
		rhs &= AnyTokenCategoryMask();
	else
		rhs = AnyTokenCategoryMask();;
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
	if (rhsBase & AnyTokenCategoryMask())
		rhsBase &= AnyTokenCategoryMask();
	else
		rhsBase = AnyTokenCategoryMask();;
	return lhs & rhsBase;;
}

template<typename T, typename U>
requires IsTokenType<T> && IsTokenType<U>
constexpr T& CastTokenType(U& other) {
	if (!TokenTypesCompatible<T>(other))
		throw std::logic_error(fmt::format(
									 "Invalid cast from type '{}' to type '{}'",
									 typeid(U).name(), typeid(T).name()));
	return *reinterpret_cast<T*>(&other);
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

template<typename Lhs, typename Rhs>
requires IsTokenType<Lhs> && IsTokenType<Rhs>
constexpr bool operator==(Lhs lhs, Rhs rhs) {
	try {
		return CastTokenType<TokenType>(lhs) == CastTokenType<TokenType>(rhs);
	} catch (const std::logic_error& e) {
		return false;
	}
}

class Token {
public:
	Token()
	 : m_Type(0), m_Text([](){
			std::string s; s.reserve(16); return s;
		}())
	{}

	template<typename T, typename S>
	requires IsTokenType<T> &&
		std::constructible_from<std::string, S&&>
	Token(T type, S&& text)
		noexcept(std::is_nothrow_constructible_v<std::string, S&&>)
	 : m_Type(static_cast<TokenType>(type)), m_Text(std::forward<S>(text))
	{}

	template<typename T, typename S, typename V>
	requires IsTokenType<T> &&
		std::constructible_from<std::string, S&&>
	Token(T type, S&& text, V value)
		noexcept(std::is_nothrow_constructible_v<std::string, S&&>)
	 : m_Type(static_cast<TokenType>(type)), m_Text(std::forward<S>(text)),
		m_Value(std::make_any<V>(value))
	{}

	Token(Token&&) noexcept = default;
	Token& operator=(Token&&) noexcept = default;
	Token(const Token&) = default;
	Token& operator=(const Token&) = default;

	template<typename T = TokenType>
	requires IsTokenType<T>
	T& type() { return CastTokenType<T>(m_Type); }
	template<typename T = TokenType>
	requires IsTokenType<T>
	const T& type() const { return CastTokenType<T>(m_Type); }

	std::string& text() { return m_Text; }
	const std::string& text() const { return m_Text; }

	template<typename V = std::any>
	V& value() {
		if constexpr (std::is_same_v<V, std::any>)
			return m_Value;
		else
			return std::any_cast<V&>(m_Value);
	}
	template<typename V = std::any>
	const V& value() const { 
		if constexpr (std::is_same_v<V, std::any>)
			return m_Value;
		else
			return std::any_cast<const V&>(m_Value);
	}

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
struct fmt::formatter<skyfauna::TTPunctuator> : fmt::formatter<std::string> {
	auto format(const skyfauna::TTPunctuator& ls, format_context& ctx) const -> decltype(ctx.out())
	{
		using namespace std::literals;
		std::string_view name;
		switch (ls) {
			case skyfauna::TTPunctuator::INVALID:
				name = "INVALID"sv;
				break;

			case skyfauna::TTPunctuator::OPERATOR:
				name = "OPERATOR"sv;
				break;

			case skyfauna::TTPunctuator::DELIMITER:
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
struct fmt::formatter<skyfauna::Token> : fmt::formatter<std::string> {
	auto format(const skyfauna::Token& tok, format_context& ctx) const -> decltype(ctx.out())
	{
		auto mask = tok.type() & skyfauna::AnyTokenCategoryMask();
		auto category = skyfauna::CastTokenType<skyfauna::TokenCategory>(mask);
		auto formatToken = [&](const skyfauna::TokenType& type, format_context& ctx) -> decltype(ctx.out()) {
			switch (category) {
				case skyfauna::TokenCategory::IDENTIFIER:
					return fmt::format_to(ctx.out(), "[CATEGORY]: {} [DETAIL]: {}",
							  category, skyfauna::CastTokenType<skyfauna::TTIdentifier>(type));

				case skyfauna::TokenCategory::LITERAL:
					return fmt::format_to(ctx.out(), "[CATEGORY]: {} [DETAIL]: {}",
							  category, skyfauna::CastTokenType<skyfauna::TTLiteral>(type));

				case skyfauna::TokenCategory::PUNCTUATOR:
					return fmt::format_to(ctx.out(), "[CATEGORY]: {} [DETAIL]: {}",
							  category, skyfauna::CastTokenType<skyfauna::TTPunctuator>(type));

				case skyfauna::TokenCategory::PREPROCESSOR_DIRECTIVE:
					return fmt::format_to(ctx.out(), "[CATEGORY]: {}", category);

				case skyfauna::TokenCategory::COMMENT:
					return fmt::format_to(ctx.out(), "[CATEGORY]: {} [DETAIL]: {}",
							  category, skyfauna::CastTokenType<skyfauna::TTComment>(type));

				default: {
					static constexpr std::string_view msg = "[CATEGORY]: Invalid";
					return std::copy(msg.begin(), msg.end(), ctx.out());
				}
			}
		};

		std::string_view startofmessage = "TOKEN: ";
		std::copy(startofmessage.begin(), startofmessage.end(), ctx.out());

		switch (category) {
			case skyfauna::TokenCategory::IDENTIFIER:
				formatToken(tok.type(), ctx);
				break;

			case skyfauna::TokenCategory::LITERAL:
				formatToken(tok.type(), ctx);
				break;

			case skyfauna::TokenCategory::PUNCTUATOR:
				formatToken(tok.type(), ctx);
				break;

			case skyfauna::TokenCategory::PREPROCESSOR_DIRECTIVE:
				formatToken(tok.type(), ctx);
				break;

			case skyfauna::TokenCategory::COMMENT:
				formatToken(tok.type(), ctx);
				break;

			default:
				formatToken(tok.type(), ctx);
		}
		return fmt::format_to(ctx.out(), " [TEXT]: {}", tok.text());
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
	template<typename Set, typename Str>
	requires std::constructible_from<common::HashSet<std::string_view>, Set&&> &&
		std::constructible_from<std::string, Str&&>
	Lexer(Set&& keywords, Str&& code)
	 : m_Keywords(std::forward<Set>(keywords)),
		m_Code(std::forward<Str>(code)) {}

	template<typename T, std::size_t N, typename S>
	requires std::constructible_from<std::string_view, T> &&
		std::constructible_from<std::string, S&&>
	Lexer(const std::array<T, N>& keywords, S&& code)
	 : m_Keywords(std::vector(keywords.begin(), keywords.end())),
		m_Code(std::forward<S>(code)) {}

	Lexer& Tokenize();

	template<typename Set>
	requires std::assignable_from<common::HashSet<std::string_view>&, Set&&>
	void SetKeywords(Set&& keywords) {
		m_Keywords = keywords;
	}
	common::HashSet<std::string_view>& GetKeywords() { return m_Keywords; }
	const common::HashSet<std::string_view>& GetKeywords() const {
		return m_Keywords;
	}

	template<typename Str>
	requires std::assignable_from<std::string&, Str&&>
	void SetCode(Str&& code)
		noexcept(std::is_nothrow_assignable_v<std::string, Str&&>) {
		m_Code = std::forward<Str>(code);
	}

	std::vector<Token>& GetTokens() noexcept { return m_Tokens; }
	const std::vector<Token>& GetTokens() const noexcept {
		return m_Tokens;
	}

	Lexer& Reset() noexcept;
	template<typename V, typename S>
	requires std::assignable_from<std::vector<std::string_view>&, V&&> &&
		std::assignable_from<std::string&, S&&>
	Lexer& Reset(V&& keywords, S&& code) {
		Reset();
		m_Keywords = std::forward<V>(keywords);
		m_Code = std::forward<S>(code);;
		return *this;
	}

private:
	int Transition(char c);
	char TryPeek(int distance) noexcept;
	char Peek(int distance);

private:
	common::HashSet<std::string_view> m_Keywords;
	std::string m_Code;
	std::vector<Token> m_Tokens;
	State m_State;
	Token m_CToken;
	std::string::const_iterator m_CodeIt;
	std::function<bool(char)> m_RecoveryFunc;

	friend struct fmt::formatter<State>;
};

constexpr char GetMatchingPairedDelimiter(char d) noexcept {
	switch (d) {
		case '[':
			return ']';
		case '{':
			return '}';
		case '(':
			return ')';
		case '<':
			return '>';
		case ']':
			return '[';
		case '}':
			return '{';
		case ')':
			return '(';
		case '>':
			return '<';
		case '"':
			return d;
		case '\'':
			return d;
		default:
			return '\0';
	}
}
}

#endif


// SPDX-License-Identifier: MIT
// Copyright (C) 2025 Chloe Eather

#ifndef SKYFAUNA_LEX_H_
#define SKYFAUNA_LEX_H_

#include <skyfauna/token.h>
#include <skyfauna/common/set.h>
#include <skyfauna/common/util.h>

#include <functional>
#include <string>
#include <vector>
#include <cstdint>

#include <spdlog/fmt/fmt.h>

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


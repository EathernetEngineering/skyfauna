// SPDX-License-Identifier: MIT
// Copyright (C) 2025 Chloe Eather

#include <skyfauna/lex.h>

#include <string>
#include <string_view>
#include <type_traits>
#include <variant>
#include <vector>

namespace skyfauna {
class Preprocessor {
public:
	enum class DirectiveType {
		INVALID = 0,
		OBJECT,
		FUNCTION,
		INCLUDE,
		CHECK_IF,
		CHECK_ELSE,
		CHECK_ELSE_IF,
		CHECK_ENDIF_IF,
		WARN_MESSAGE,
		ERROR_MESSAGE
	};
	struct Directive {
		DirectiveType type = DirectiveType::INVALID;
	};
	struct ObjectMacro : Directive{
		std::string name;
		std::size_t nameHash;
		std::vector<Token> contents;
	};
	struct FunctionMacro : Directive {
		std::string name;
		std::size_t nameHash;
		std::vector<Token> params;
		std::vector<Token> contents;
		int paramCount = 0;
		bool vargs = false;
	};
	struct Include : Directive {
		std::string file;
	};
	struct Message : Directive {
		std::string contents;
	};
	struct Check : Directive {
		std::vector<std::string_view> contents, operators;
	};
	using DirectiveVariant =
		std::variant<std::monostate, ObjectMacro, FunctionMacro,
			Include, Message, Check>;

private:
	using token_in_iterator = std::vector<Token>::const_iterator;
	using token_out_iterator = std::vector<Token>::iterator;

public:
	Preprocessor() = default;
	template<typename V>
	requires std::constructible_from<std::vector<Token>, V&&>
	Preprocessor(V&& code)
		noexcept(std::is_nothrow_constructible_v<std::vector<Token>, V&&>)
	 : m_Code(std::forward<V>(code))
	{
	}

	template<typename V>
	requires std::assignable_from<std::vector<Token>&, V&&>
	Preprocessor& SetCode(V&& code)
		noexcept(std::is_nothrow_assignable_v<std::vector<Token>&, V&&>)
	{
		m_Code = std::forward<V>(code);
		return *this;
	}

	std::vector<Token>& GetOutput() noexcept {
		return m_Preprocessed;
	}
	const std::vector<Token>& GetOutput() const noexcept {
		return m_Preprocessed;
	}

	Preprocessor& Reset();
	template<typename V>
	requires std::assignable_from<std::vector<Token>&, V&&>
	Preprocessor& Reset(V&& code)
		noexcept(std::is_nothrow_assignable_v<std::vector<Token>&, V&&>)
	{
		Reset();
		m_Code = std::forward<V>(code);
		return *this;
	}

	Preprocessor& Preprocess();
	std::vector<std::string>& GetDiagnotics() { return m_Diagnostics; }
	const std::vector<std::string>& GetDiagnotics() const { return m_Diagnostics; }

private:
	std::vector<Token> PreprocessRecurse(token_in_iterator first,
									  token_in_iterator lastt);
	std::vector<Token> PreprocessRecurse(const std::vector<Token>& line)
	{
		return PreprocessRecurse(line.begin(), line.end());
	}
	DirectiveVariant ParseDirective(const std::string_view line);

private:
	std::vector<Token> m_Code;
	std::vector<Token> m_Preprocessed;

	std::vector<DirectiveVariant> m_Directives;
	std::vector<std::string> m_Diagnostics;
};
}


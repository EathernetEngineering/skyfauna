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
		std::string_view name;
		std::size_t nameHash;
		std::vector<Token> contents;
	};
	struct FunctionMacro : Directive {
		std::string_view name;
		std::size_t nameHash;
		std::vector<Token> params;
		std::vector<Token> contents;
		int paramCount = 0;
		bool vargs = false;
	};
	struct Include : Directive {
		std::string_view file;
	};
	struct Message : Directive {
		std::string_view contents;
	};
	struct Check : Directive {
		std::vector<std::string_view> contents, operators;
	};
	using DirectiveVariant =
		std::variant<std::monostate, ObjectMacro, FunctionMacro,
			Include, Message, Check>;

	// Whether defining or using the macro it will have the same data. Use a
	// different name to make it easier to tell in code whether the object being
	// referred to is use case or the definiition.
	using FunctionMacroUse = FunctionMacro;
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
		return m_Code;
	}
	const std::vector<Token>& GetOutput() const noexcept {
		return m_Code;
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
	DirectiveVariant ParseDirective(const std::string_view line);
	std::vector<Token> ExpandMacro(const ObjectMacro& macro);
	std::vector<Token> ExpandMacro(const FunctionMacro& macro, const FunctionMacroUse& code);
	template<typename InputIt>
	requires std::input_iterator<InputIt>
	std::vector<Token> ExpandMacro(const FunctionMacro& macro, InputIt pos);

private:
	std::vector<Token> m_Code;

	std::vector<DirectiveVariant> m_Directives;
	std::vector<std::string> m_Diagnostics;
};
}


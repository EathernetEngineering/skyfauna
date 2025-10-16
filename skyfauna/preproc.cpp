// SPDX-License-Identifier: MIT
// Copyright (C) 2025 Chloe Eather

#include <map>
#include <skyfauna/preproc.h>
#include <skyfauna/lex.h>
#include <skyfauna/common/log.h>
#include <skyfauna/common/set.h>
#include <skyfauna/common/util.h>

#include <array>

#include <spdlog/fmt/fmt.h>

namespace skyfauna {
static constexpr std::array g_PreprocDirectives = std::to_array<std::string_view>({
	"define",
	"if"
	"elif",
	"else",
	"endif",
	"include",
	"error",
	"warn",
	"vargs",
	"vlist"
});

static auto ReplaceObjectMacro(const Preprocessor::ObjectMacro& to,
						 std::vector<Token>::iterator tokenIt,
						 std::vector<Token>& tokens) -> decltype(tokenIt)
{
	tokenIt = tokens.erase(tokenIt);
	return tokens.insert(tokenIt, to.contents.begin(), to.contents.end());
}

static auto ReplaceFunctionMacro(const Preprocessor::FunctionMacro& to,
						 std::vector<Token>::iterator tokenIt,
						 std::vector<Token>& tokens) -> decltype(tokenIt)
{
	// Search the code tokens to find the bounds of the function call-like use
	auto toReplaceBegin = tokenIt;
	auto toReplaceEnd = std::ranges::next(tokenIt, 2, tokens.end());
	std::vector<Token> vargs;
	int argIndex = 0;
	std::map<std::size_t, std::vector<Token>> argsMap;
	TokenType lastTokenType = CastTokenType<TokenType>(TTPunctuator::DELIMITER);
	for (; toReplaceEnd != tokens.end(); ++toReplaceEnd) {
		if (toReplaceEnd->type() == TTPunctuator::DELIMITER &&
			lastTokenType == TTIdentifier::SYMBOL)
		{
			if (toReplaceEnd->text().compare(",") == 0) {
				++argIndex;
				continue;
			} else if (toReplaceEnd->text().compare(")") == 0) {
				break;
			} else {
				std::string errmsg = fmt::format(
					"Unexpected token {}, expected ')'",
					toReplaceEnd->text());
				throw std::runtime_error(errmsg);
			}
			lastTokenType = CastTokenType<TokenType>(TTPunctuator::DELIMITER);
		} else {
			if (argIndex >= to.paramCount) {
				vargs.push_back(*toReplaceEnd);
			} else {
				argsMap[to.params[argIndex].value<std::size_t>()].push_back(*toReplaceEnd);
			}
			lastTokenType = CastTokenType<TokenType>(TTIdentifier::SYMBOL);
		}
	}
	// Validate the macros expected parameters and the given arguments
	// are a valid combination
	if (vargs.size() > 0 && to.vargs == false) {
		std::string errmsg = fmt::format(
			"{} expects {} arguments, {} provided",
			toReplaceBegin->text(), to.paramCount, argIndex);
		throw std::runtime_error(errmsg);
	}
	if (static_cast<int>(argsMap.size()) != to.paramCount) {
		std::string errmsg = fmt::format(
			"{} expects {} arguments, {} provided",
			toReplaceBegin->text(), to.paramCount, argIndex);
		throw std::runtime_error(errmsg);
	}
	// Do the substitution into a copy of the macro
	std::vector<Token> replacement = to.contents;
	for (auto replacementIt = replacement.begin();
		replacementIt != replacement.end();
		++replacementIt)
	{
		if (replacementIt->type() == TTIdentifier::SYMBOL) {
			const auto arg = argsMap.find(replacementIt->value<std::size_t>());
			if (arg == argsMap.end()) {
				continue;
			}

			replacementIt = replacement.erase(replacementIt);
			replacementIt = replacement.insert(replacementIt,
									  arg->second.begin(),
									  arg->second.end());
			++replacementIt;
		} else if (replacementIt->type() == TTIdentifier::KEYWORD) {
			if (replacementIt->text().compare("vargs")) {
				continue;
			}

			if (!to.vargs)
				throw std::runtime_error("vargs used in macro that is not variadic");

			replacementIt = replacement.erase(replacementIt);
			replacementIt = replacement.insert(replacementIt,
									  vargs.begin(),
									  vargs.end());
			++replacementIt;
		}
	}
	// Only erse and copy now that it's known the substition was successful
	tokenIt = tokens.erase(toReplaceBegin, ++toReplaceEnd);
	return ++tokens.insert(tokenIt, replacement.begin(), replacement.end());
}

static Preprocessor::ObjectMacro
	CreateObjectMacro(const std::vector<Token>& tokens)
{
	Preprocessor::ObjectMacro d;
	auto it = tokens.begin();

	// Skip "define"
	it = std::ranges::next(it, 1, tokens.end());
	if (it == tokens.end()) {
		std::string errmsg = fmt::format(
			"Expected identifier after {}",
			tokens.front().text());
		throw std::runtime_error(errmsg);
	}
	d.name = it->text();
	d.nameHash = Hash<std::string>{}(it->text());
	d.type = Preprocessor::DirectiveType::OBJECT;
	++it;
	d.contents.resize(std::ranges::distance(it, tokens.end()));
	std::copy(it, tokens.end(), d.contents.begin());

	return d;
}

static Preprocessor::FunctionMacro
	CreateFunctionMacro(const std::vector<Token>& tokens)
{
	Preprocessor::FunctionMacro d;

	auto it = tokens.begin();
	// Skip "define"
	it = std::ranges::next(it, 1, tokens.end());
	if (it == tokens.end()) {
		std::string errmsg = fmt::format(
			"Expected identifier after {}",
			tokens.front().text());
		throw std::runtime_error(errmsg);
	}
	d.name = it->text();
	d.nameHash = Hash<std::string>{}(it->text());
	++it;
	if (it == tokens.end())
		throw std::logic_error("This state shouldn't be possible?");
	if (it->text().compare("("))
		throw std::runtime_error("Expected '('");

	++it;
	TokenType lastTokenType = CastTokenType<TokenType>(TTPunctuator::DELIMITER);
	for (; it != tokens.end(); ++it) {
		if (it->text().compare(")") == 0)
			break;
		if (lastTokenType == TTPunctuator::DELIMITER &&
			it->type() == TTIdentifier::SYMBOL) {
			++d.paramCount;
			d.params.push_back(*it);
		} else if (lastTokenType == TTIdentifier::SYMBOL &&
			it->type() == TTPunctuator::DELIMITER) {
			if (it->text().compare(",")) {
				std::string errmsg = fmt::format(
					"Unexpected identifier {}",
					it->text());
			}
			// Do nothing
		} else if (lastTokenType == TTPunctuator::DELIMITER &&
			it->type() == TTPunctuator::OPERATOR) {
			if (it->text().compare("...")) {
				std::string errmsg = fmt::format(
					"Unexpected identifier {}",
					it->text());
			}
			d.vargs = true;
		} else {
			std::string errmsg = fmt::format(
				"Unexpected identifier {}",
				it->text());
			throw std::runtime_error(errmsg);
		}
		lastTokenType = it->type();
	}
	if (it == tokens.end()) {
		std::string errmsg = fmt::format(
			"Unterminated '(', expexted ')' before end of line",
			tokens.front().text());
		throw std::runtime_error(errmsg);
	}
	++it;
	d.contents.resize(std::ranges::distance(it, tokens.end()));
	std::copy(it, tokens.end(), d.contents.begin());

	return d;
}

static Preprocessor::Include
	CreateInclude(const std::vector<Token>& tokens)
{
	Preprocessor::Include d;

	auto it = tokens.begin();
	
	// Skip "include" directive, expect '<'
	it = std::ranges::next(it, 1, tokens.end());

	if (it == tokens.end()) {
		std::string errmsg = fmt::format(
			"Invalid use of {}, expected a string literal",
			tokens.front().text());
		throw std::runtime_error(errmsg);
	}
	if (it->type() != TTPunctuator::OPERATOR) {
		std::string errmsg = fmt::format(
			"Invalid use of {}, expected a string literal",
			tokens.front().text());
		throw std::runtime_error(errmsg);
	}
	if (it->text().compare("<")) {
		std::string errmsg = fmt::format(
			"Unexpected identitfier: {}, expected '<' before end of line",
			tokens.front().text());
		throw std::runtime_error(errmsg);
	}
	Token t(TTLiteral::STRING, "");
	auto closeIt = it;
	for (; closeIt != tokens.end(); ++closeIt) {
		if (closeIt->text().compare(">"))
			break;
		t.text().append(closeIt->text());
	}
	if (closeIt == tokens.end()) {
		std::string errmsg = fmt::format(
			"Unterminated '<', expexted '>'",
			tokens.front().text());
		throw std::runtime_error(errmsg);
	}
	t.value() = std::make_any<std::string>(t.text());
	d.file = t.text();
	d.type = Preprocessor::DirectiveType::INCLUDE;

	return d;
}

static Preprocessor::DirectiveVariant
	CreateMacro(const std::vector<Token>& tokens) {
	if (tokens.size() < 3)
		return Preprocessor::DirectiveVariant();
	if (tokens[2].text()[0] == '(')
		return CreateFunctionMacro(tokens);
	else
		return CreateObjectMacro(tokens);
}

static Preprocessor::Check
	CreateCheck(const std::vector<Token>& tokens)
{
	Preprocessor::Check d;

	return d;
}

static Preprocessor::Message
	CreateMessage(const std::vector<Token>& tokens)
{
	Preprocessor::Message d;
	auto it = tokens.begin();
	
	// Skip "warn" or "error" directive
	it = std::ranges::next(it, 1, tokens.end());

	if (it == tokens.end()) {
		std::string errmsg = fmt::format(
			"Invalid use of {}, Expected a string literal",
			tokens.front().text());
		throw std::runtime_error(errmsg);
	}
	if (it->type() != TTLiteral::STRING) {
		std::string errmsg = fmt::format(
			"Invalid use of {}, Expected a string literal",
			tokens.front().text());
		throw std::runtime_error(errmsg);
	}
	d.contents = std::any_cast<std::string>(it->value());
	if (std::any_cast<std::size_t>(tokens.front().value()) == Hash<const char *>{}("warn"))
		d.type = Preprocessor::DirectiveType::WARN_MESSAGE;
	else
		d.type = Preprocessor::DirectiveType::ERROR_MESSAGE;

	return d;
}

Preprocessor& Preprocessor::Reset()
{
	m_Code.clear();
	m_Directives.clear();
	m_Diagnostics.clear();
	return *this;
}

Preprocessor& Preprocessor::Preprocess() {
	using std::ranges::next;
	int adv = 1;
	for (auto it = m_Code.begin();
		it != m_Code.end();
		it = next(it, adv, m_Code.end()))
	{
		adv = 1;
		if (it->type() == TokenCategory::PREPROCESSOR_DIRECTIVE) {
			DirectiveVariant d;
			if (it->text().size() < 1) {
				std::string msg = fmt::format(
					"Failed to parse preprocessor directive: \n\t{}",
					it->text());
				m_Diagnostics.emplace_back(std::move(msg));
				continue;
			}
			std::string_view line(it->text().begin(), it->text().end());
			d = ParseDirective(line);
			it = m_Code.erase(it);
			adv = 0;
			if (std::holds_alternative<std::monostate>(d)) {
				std::string msg = fmt::format(
					"Failed to parse preprocessor directive: \n\t{}",
					it->text());
				m_Diagnostics.emplace_back(std::move(msg));
				continue;
			}
			m_Directives.emplace_back(std::move(d));
		} else if (it->type() == TTIdentifier::SYMBOL) {
			for (const auto& directive : m_Directives) {
				if (std::holds_alternative<ObjectMacro>(directive)) {
					auto& macro = std::get<ObjectMacro>(directive);
					if (macro.nameHash == it->value<std::size_t>()) {
						it = ReplaceObjectMacro(macro, it, m_Code);
						break;
					}
				} else if (std::holds_alternative<FunctionMacro>(directive)) {
					auto& macro = std::get<FunctionMacro>(directive);
					if (macro.nameHash == it->value<std::size_t>()) {
						it = ReplaceFunctionMacro(macro, it, m_Code);
						break;
					}
				}
			}
		}
	}
	return *this;
}

static Preprocessor::DirectiveVariant
	CreateDirective(const common::HashSet<std::string_view>& keywords,
				 const std::vector<Token>& tokens)
{
	Preprocessor::DirectiveVariant d;

	auto it = keywords.Find(tokens.front().value<std::size_t>());
	if (it == keywords.end())
		return d;

	switch (it->second) {
		case Hash<const char *>{}("define"):
			return CreateMacro(tokens);
			break;

		case Hash<const char *>{}("if"):
		case Hash<const char *>{}("elif"):
		case Hash<const char *>{}("else"):
		case Hash<const char *>{}("endif"):
			return CreateCheck(tokens);
			break;

		case Hash<const char *>{}("include"):
			return CreateInclude(tokens);
			break;

		case Hash<const char *>{}("error"):
		case Hash<const char *>{}("warn"):
			return CreateMessage(tokens);
			break;

		default:
			throw std::runtime_error("Unknown preprocessor directive");
	}

	return d;
}

// TODO: recurse and resovle macros before paring
Preprocessor::DirectiveVariant
	Preprocessor::ParseDirective(const std::string_view line)
{
	using std::ranges::next;
	DirectiveVariant d;
	Lexer l(g_PreprocDirectives, std::string(line.substr(1)));
	std::vector<Token>& tokens = l.Tokenize().GetTokens();

	try {
		if (tokens.front().type() != TTIdentifier::KEYWORD) {
			throw std::logic_error("Not a keyword");
			return d;
		}
		d = CreateDirective(l.GetKeywords(), l.GetTokens());
	} catch (const std::logic_error& e) {
		m_Diagnostics.push_back(fmt::format("Invalid preprocessor directive {} ()",
									tokens.front().text(), e.what()));
		return d;
	}

	return d;
}
}


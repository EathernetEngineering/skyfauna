// SPDX-License-Identifier: MIT
// Copyright (C) 2025 Chloe Eather

#include <iterator>
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
	"if",
	"elif",
	"else",
	"endif",
	"include",
	"error",
	"warn",
	"vargs",
	"vlist"
});

template<typename InputIt, typename End>
requires std::input_iterator<InputIt> && std::sentinel_for<End, InputIt> &&
	std::same_as<std::iter_value_t<InputIt>, Token>
static auto ReplaceObjectMacro(const Preprocessor::ObjectMacro& to,
						 InputIt tokenIt, End last,
						 std::vector<Token>& tokens) -> decltype(tokenIt)
{
	tokens.insert(tokens.end(), to.contents.begin(), to.contents.end());
	return std::ranges::next(tokenIt,
						  std::ranges::distance(to.contents.begin(), to.contents.end()),
						  last);
}

template<typename InputIt, typename End>
requires std::input_iterator<InputIt> && std::sentinel_for<End, InputIt> &&
	std::same_as<std::iter_value_t<InputIt>, Token>
static auto ReplaceFunctionMacro(const Preprocessor::FunctionMacro& to,
						 InputIt tokenIt, End last,
						 std::vector<Token>& tokens) -> decltype(tokenIt)
{
	// Search the code tokens to find the bounds of the function call-like use
	auto toReplaceBegin = tokenIt;
	auto toReplaceEnd = std::ranges::next(tokenIt, 2, last);
	char pairedDelimiterCount = 0;
	char lastPairedDelimiter;
	std::vector<Token> vargs;
	int argIndex = 0;
	std::map<std::size_t, std::vector<Token>> argsMap;
	TokenType lastTokenType = CastTokenType<TokenType>(TTPunctuator::DELIMITER);
	for (; toReplaceEnd != last; ++toReplaceEnd) {
		if (toReplaceEnd->type() == TTPunctuator::DELIMITER &&
			lastTokenType == TTIdentifier::SYMBOL)
		{
			auto pairedDelimiterIdx = toReplaceEnd->text().npos;
			const std::string& tokenText = toReplaceEnd->text();
			if (tokenText.compare(",") == 0) {
				++argIndex;
				if (argIndex > to.paramCount) { 
					vargs.push_back(*toReplaceEnd);
				}
				continue;
			} else if (pairedDelimiterCount < 0) {
				throw std::runtime_error("");
			} else if (tokenText.compare(")") == 0 &&
				pairedDelimiterCount == 0)
			{
				break;
			} else if ((pairedDelimiterIdx = tokenText.find_first_of("[{(<"))
				!= tokenText.npos)
			{
				lastPairedDelimiter = tokenText.at(pairedDelimiterIdx);
			} else if ((pairedDelimiterIdx = tokenText.find_first_of("]})>"))
				!= tokenText.npos)
			{
				char expectedDelim = tokenText.at(pairedDelimiterIdx);
				if (lastPairedDelimiter != GetMatchingPairedDelimiter(
					expectedDelim))
				{
					std::string errmsg = fmt::format(
						"Unexpected delimiter \"{}\", expected \"{}\"",
						expectedDelim, GetMatchingPairedDelimiter(expectedDelim));
					throw std::runtime_error(errmsg);
				}
			} else {
				std::string errmsg = fmt::format(
					"Unexpected token \"{}\", expected \")\"",
					tokenText);
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
			"Non-variadic macro \"{}\" expects {} arguments, {} provided",
			toReplaceBegin->text(), to.paramCount, argIndex);
		throw std::runtime_error(errmsg);
	}
	if (static_cast<int>(argsMap.size()) != to.paramCount) {
		std::string errmsg = fmt::format(
			"\"{}\" expects {} arguments, {} provided",
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
	tokens.insert(tokens.end(), replacement.begin(), replacement.end());
	return toReplaceEnd;
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
			"Expected identifier after \"{}\"",
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
			"Expected identifier after \"{}\"",
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
			if (it->text().compare(",") != 0) {
				std::string errmsg = fmt::format(
					"Unexpected identifier \"{}\"",
					it->text());
			}
			// Do nothing
		} else if (lastTokenType == TTPunctuator::DELIMITER &&
			it->type() == TTPunctuator::OPERATOR) {
			if (it->text().compare("...") == 0) {
				d.vargs = true;
			} else {
				std::string errmsg = fmt::format(
					"Unexpected identifier \"{}\"",
					it->text());
				throw std::runtime_error(errmsg);
			}
		} else {
			std::string errmsg = fmt::format(
				"Unexpected identifier \"{}\"",
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
			"Invalid use of \"{}\", expected a file name",
			tokens.front().text());
		throw std::runtime_error(errmsg);
	}
	if (it->type() != TTPunctuator::OPERATOR) {
		std::string errmsg = fmt::format(
			"Invalid use of \"{}\", expected '<'",
			tokens.front().text());
		throw std::runtime_error(errmsg);
	}
	if (it->text().compare("<")) {
		std::string errmsg = fmt::format(
			"Unexpected token: \"{}\", expected '<' before end of line",
			tokens.front().text());
		throw std::runtime_error(errmsg);
	}
	std::string filename;
	while (it != tokens.end()) {
		if (it->text().compare(">") == 0)
			break;
		filename.append(it->text());
		++it;
	}
	if (it == tokens.end()) {
		throw std::runtime_error("Unterminated '<', expexted '>'");
	}
	++it; // This should be at the end of line now in a well formed program
	d.file = filename;
	d.type = Preprocessor::DirectiveType::INCLUDE;

	// This will never actually loop? Is that okay? Should this be an if?
	for (; it != tokens.end(); ++it) {
		throw std::runtime_error(fmt::format("Unexpected unquialified id: {}",
						   it->text()));
	}

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
	m_Preprocessed.reserve(m_Code.size());
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
			bool replaced = false;
			for (const auto& directive : m_Directives) {
				try {
					if (std::holds_alternative<ObjectMacro>(directive)) {
						auto& macro = std::get<ObjectMacro>(directive);
						if (macro.nameHash == it->value<std::size_t>()) {
							it = ReplaceObjectMacro(macro, it, m_Code.end(), m_Preprocessed);
							replaced = true;
							break;
						}
					} else if (std::holds_alternative<FunctionMacro>(directive)) {
						auto& macro = std::get<FunctionMacro>(directive);
						if (macro.nameHash == it->value<std::size_t>()) {
							it = ReplaceFunctionMacro(macro, it, m_Code.end(), m_Preprocessed);
							replaced = true;
							break;
						}
					}
				} catch (const std::runtime_error& e) {
					m_Diagnostics.push_back(fmt::format("Failed to replace macro {} ({})",
												it->text(), e.what()));
				}
			}
			if (!replaced) {
				m_Preprocessed.push_back(*it);
			}
		} else {
			m_Preprocessed.push_back(*it);
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
			throw std::logic_error(fmt::format("\"{}\" Not a keyword",
				tokens.front().text()));
		}
		d = CreateDirective(l.GetKeywords(), l.GetTokens());
	} catch (const std::logic_error& e) {
		m_Diagnostics.push_back(fmt::format(
			"Invalid preprocessor directive \"{}\" ({})",
			tokens.front().text(), e.what()));
	} catch (const std::runtime_error& e) {
		m_Diagnostics.push_back(fmt::format(
			"Invalid preprocessor directive \"{}\" ({})",
			tokens.front().text(), e.what()));
	}

	return d;
}
}


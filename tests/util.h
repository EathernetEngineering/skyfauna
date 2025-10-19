// SPDX-License-Identifier: MIT
// Copyright (C) 2025 Chloe Eather

#ifndef SKYFAUNA_TEST_UTIL_H_
#define SKYFAUNA_TEST_UTIL_H_

#include <skyfauna/lex.h>

#include <iterator>
#include <ranges>
#include <concepts>
#include <vector>

#include <gtest/gtest-assertion-result.h>

namespace skyfauna::tests {
template<typename InputIt1, typename InputIt2>
requires std::input_iterator<InputIt1> && std::input_iterator<InputIt2> &&
	std::same_as<std::iter_value_t<InputIt1>, ::skyfauna::Token> &&
	std::same_as<std::iter_value_t<InputIt2>, ::skyfauna::Token>
testing::AssertionResult CompareTokens(InputIt1 lhsFirst, InputIt1 lhsLast,
										 InputIt2 rhsFirst, InputIt2 rhsLast)
{
	auto lhsIt = lhsFirst;
	auto rhsIt = rhsFirst;
	bool eq = true, sizeEq = true;
	if (std::distance(lhsIt, lhsLast) != std::distance(rhsIt, rhsLast))
		eq = false, sizeEq = false;
	std::vector<std::string> errors;
	for (; lhsIt != lhsLast && rhsIt != rhsLast; ++lhsIt, ++rhsIt)
	{
		TokenType lhsCategoryBase = CastTokenType<TokenType>(lhsIt->type());
		TokenCategory lhsCategory = 
			CastTokenType<TokenCategory>(lhsCategoryBase & AnyTokenCategoryMask());
		TokenType rhsCategoryBase = CastTokenType<TokenType>(rhsIt->type());
		TokenCategory rhsCategory = 
			CastTokenType<TokenCategory>(rhsCategoryBase & AnyTokenCategoryMask());
		TokenType lhsType = lhsIt->type();
		TokenType rhsType = rhsIt->type();

		if (lhsCategory != rhsCategory) {
			eq = false;
			errors.push_back(fmt::format("Token \"{}\" category not eq to token \"{}\""
								" ({} vs {})",
								lhsIt->text(), rhsIt->text(),
								lhsCategory, rhsCategory));
		}
		if (lhsType != rhsType) {
			eq = false;
			lhsType &= AnySpecificTokenTypeMask();
			rhsType &= AnySpecificTokenTypeMask();
			errors.push_back(fmt::format("Token \"{}\" type not eq to token \"{}\""
								" ({}:{:08b} vs {}:{:08b})",
								lhsIt->text(), rhsIt->text(),
								lhsCategory, lhsType,
								rhsCategory, rhsType));
		}
		if (lhsIt->text().compare(rhsIt->text())) {
			eq = false;
			errors.push_back(fmt::format("Token \"{}\"text not eq to token \"{}\"",
								lhsIt->text(), rhsIt->text()));
		}
	}
	if (eq) {
		return ::testing::AssertionSuccess();
	} else {
		auto result = ::testing::AssertionFailure();
		if (!sizeEq)
			result << "Input token counts unequal (" << std::distance(lhsFirst, lhsLast)
				<< " vs "<< std::distance(rhsFirst, rhsLast) <<")";
		if (errors.size() > 0) {
			if (!sizeEq) {
				result << std::endl;
			}
			auto errorsIt = errors.begin();
			do {
				result << *errorsIt << std::endl;
				++errorsIt;
			} while (errorsIt != errors.end());
		}
		return result;
	}
}

template<typename Range1, typename Range2>
requires std::ranges::range<Range1> && std::ranges::range<Range2>
inline testing::AssertionResult CompareTokens(const Range1& lhs,
											  const Range2& rhs)
{
	using std::ranges::begin, std::ranges::end;
	return CompareTokens(begin(lhs), end(lhs), begin(rhs), end(rhs));
}
}

#endif


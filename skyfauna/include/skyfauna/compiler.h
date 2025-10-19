// SPDX-License-Identifier: MIT
// Copyright (C) 2025 Chloe Eather

#ifndef SKYFAUNA_COMPILER_H_
#define SKYFAUNA_COMPILER_H_

#include <skyfauna/lex.h>

#include <filesystem>
#include <iterator>
#include <vector>

namespace skyfauna {
class Compiler {
public:
	Compiler() = default;
	~Compiler() noexcept {};

	void AddFile(std::filesystem::path path);

	template<typename InputIt, typename End>
	requires std::input_iterator<InputIt> &&
		std::sentinel_for<End, InputIt> &&
		std::constructible_from<std::filesystem::path, std::iter_reference_t<InputIt>>
	void AddFile(InputIt begin, End end) {
		for (InputIt it = begin; it != end; ++it)
		AddFile(*it);
	}

	void Compile();
private:
	void CompileFile(const std::filesystem::path& path);
	std::vector<Token> LexPreprocessorDirective(const Token& t);
	Compiler& Preprocess();
	Compiler& Parse();

private:
	std::vector<std::filesystem::path> m_FilePaths;
	std::vector<Token> m_Tokens;
};
}

#endif


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

	template<typename It>
	requires std::input_iterator<It>
	void AddFile(It begin, It end) {
		for (It it = begin; it != end; ++it)
		AddFile(*it);
	}

	void Compile();
private:
	void CompileFile(const std::filesystem::path& path);

private:
	std::vector<std::filesystem::path> m_FilePaths;
	Lexer m_Lex;
};
}

#endif


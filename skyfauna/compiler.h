// SPDX-License-Identifier: MIT
// Copyright (C) 2025 Chloe Eather

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

private:
	std::vector<std::filesystem::path> m_FilePaths;
	Lexer m_Lex;
};
}

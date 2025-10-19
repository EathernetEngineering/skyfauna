// SPDX-License-Identifier: MIT
// Copyright (C) 2025 Chloe Eather

#include <skyfauna/compiler.h>
#include <skyfauna/preproc.h>
#include <skyfauna/common/log.h>
#include <skyfauna/common/util.h>

#include <filesystem>
#include <fstream>
#include <system_error>
#include <cerrno>

namespace skyfauna {
static constexpr std::array g_Keywords = std::to_array<std::string_view>({
	"int",
	"float",
	"char",
	"string",
	"void",
	"ptr",
	"return",
	"static",
	"class",
	"public",
	"private",
	"protected",
	"null",
});

void Compiler::AddFile(std::filesystem::path path) {
	if (!std::filesystem::exists(path)) {
		throw std::runtime_error(fmt::format("File {} does not exist",
									   path.filename().string()));
	}
	if (!std::filesystem::is_regular_file(path)) {
		throw std::runtime_error(fmt::format("File {} is not a valid file",
									   path.filename().string()));
	}

	m_FilePaths.emplace_back(std::move(path));
}

void Compiler::Compile() {
	for (auto& path : m_FilePaths) {
		CompileFile(path);
		SF_INFO("Compiled file: {}", path.string());
	}
}

void Compiler::CompileFile(const std::filesystem::path& path) {
	std::string codeStr;
	std::ifstream file(path);
	if (!file.is_open()) {
		throw std::system_error(std::error_code(errno, std::system_category()));
	}
	auto size = file.seekg(0, file.end).tellg();
	file.seekg(0, file.beg);
	codeStr.resize(size);
	file.read(codeStr.data(), size);
	if (file.fail()) {
		throw std::runtime_error(fmt::format("Failed to read file {}",
									   path.filename().string()));
	}
	SF_DEBUG("Code to compile: \n{}\n", codeStr);
	Lexer lex(std::vector<std::string_view>(g_Keywords.begin(), g_Keywords.end()),
			 std::move(codeStr));
	m_Tokens = std::move(lex.Tokenize().GetTokens());
	Preprocess();

	for (auto& token : m_Tokens) {
		SF_TRACE("{}", token);
	}
	Parse();

}

Compiler& Compiler::Preprocess()
{
	Preprocessor pre(m_Tokens);
	pre.Preprocess();
	m_Tokens = std::move(pre.GetOutput());
	for (const auto& msg : pre.GetOutput()) {
		SF_ERROR("PREPROCESSOR: {}", msg);
	}

	return *this;
}

Compiler& Compiler::Parse()
{
	return *this;
}
}


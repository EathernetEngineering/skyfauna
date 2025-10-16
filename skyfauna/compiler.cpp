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
	m_Lex.Reset();
}

void Compiler::CompileFile(const std::filesystem::path& path) {
	std::string m_Code;
	std::ifstream file(path);
	if (!file.is_open()) {
		throw std::system_error(std::error_code(errno, std::system_category()));
	}
	auto size = file.seekg(0, file.end).tellg();
	file.seekg(0, file.beg);
	m_Code.resize(size);
	file.read(m_Code.data(), size);
	if (file.fail()) {
		throw std::runtime_error(fmt::format("Failed to read file {}",
									   path.filename().string()));
	}
	SF_DEBUG("Code to compile: \n{}\n", m_Code);
	m_Lex.Reset(std::vector<std::string_view>(g_Keywords.begin(), g_Keywords.end()),
			 std::move(m_Code));
	m_Tokens = std::move(m_Lex.Tokenize().GetTokens());
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


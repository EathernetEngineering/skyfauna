// SPDX-License-Identifier: MIT
// Copyright (C) 2025 Chloe Eather

#include <skyfauna/compiler.h>
#include <skyfauna/common/log.h>
#include <skyfauna/common/util.h>

#include <filesystem>
#include <fstream>
#include <system_error>
#include <cerrno>

namespace skyfauna {
void Compiler::AddFile(std::filesystem::path path) {
	if (!std::filesystem::exists(path)) {
		throw std::runtime_error(std::format("File {} does not exist",
									   path.filename().string()));
	}
	if (!std::filesystem::is_regular_file(path)) {
		throw std::runtime_error(std::format("File {} is not a valid file",
									   path.filename().string()));
	}

	m_FilePaths.emplace_back(std::move(path));
}

void Compiler::Compile() {
	for (auto& path : m_FilePaths) {
		CompileFile(path);
	}
	m_Lex.Reset();
}

void Compiler::CompileFile(const std::filesystem::path& path) {
	std::string code;
	std::ifstream file(path);
	if (!file.is_open()) {
		throw std::system_error(std::error_code(errno, std::system_category()));
	}
	auto size = file.seekg(0, file.end).tellg();
	file.seekg(0, file.beg);
	code.resize(size);
	file.read(code.data(), size);
	if (file.fail()) {
		throw std::runtime_error(fmt::format("Failed to read file {}",
									   path.filename().string()));
	}
	SF_TRACE("Raw code: \n{}\n", code);
	m_Lex.Reset(std::move(code));
	const auto& tokens = m_Lex.Tokenize();

	for (auto& token : tokens) {
		SF_TRACE("{}", token);
	}
}
}


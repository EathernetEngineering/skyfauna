// SPDX-License-Identifier: MIT
// Copyright (C) 2025 Chloe Eather

#include <filesystem>
#include <skyfauna/compiler.h>
#include <skyfauna/common/config.h>
#include <skyfauna/common/log.h>
#include <skyfauna/common/commandLine.h>

#include <array>
#include <iostream>
#include <string>
#include <vector>
#include <getopt.h>


enum class Arguments : uint32_t {
	HELP,
	OUTPUT,
	ARG_VERSION,
	ARG_LOG_LEVEL
};

static std::array longOpts = std::to_array({
	skyfauna::common::CmdLineOption<Arguments>{ "help", 'h',
		skyfauna::common::CmdLineArgRequired::NO_ARG, Arguments::HELP },
	skyfauna::common::CmdLineOption<Arguments>{ "usage", 'h',
		skyfauna::common::CmdLineArgRequired::NO_ARG, Arguments::HELP },
	skyfauna::common::CmdLineOption<Arguments>{ "output", 'o',
		skyfauna::common::CmdLineArgRequired::REQUIRED_ARG, Arguments::OUTPUT },
	skyfauna::common::CmdLineOption<Arguments>{ "version", 'v',
		skyfauna::common::CmdLineArgRequired::NO_ARG, Arguments::ARG_VERSION },
	skyfauna::common::CmdLineOption<Arguments>{ "loglevel", 0,
		skyfauna::common::CmdLineArgRequired::REQUIRED_ARG, Arguments::ARG_LOG_LEVEL }
});
// static const char shortOpts[] = "?o:";
// static const option longOpts[] = {
// 	{ "help", no_argument, 0, '?' },
// 	{ "usage", no_argument, 0, '?' },
// 	{ "output", required_argument, 0, 'o' },
// 	{ "version", no_argument, 0, ARG_VERSION },
// 	{ 0, 0, 0, 0 }
// };

static void printUsage(const std::string_view command) {
	std::cout << "Usage: " << command << " [OPTION...] INPUT..." << '\n';
	std::cout << "\t-o, --output          \tFile to write output to" << '\n';
	std::cout << "\t    --loglevel=<level>\tSet the logging level (default: trace)" << '\n';
	std::cout << "\t                      \tlevels: trace, debug, info, warn, error, crit" << '\n';
	std::cout << "\t                      \t(In the future the default will be warn)" << '\n';
	std::cout << "\t-v, --version         \tPrint program version" << '\n';
	std::cout << "\t-h, --help            \tPrint this message" << '\n';
	std::cout << "\t    --usage" << std::endl;
}

static void printVersion(const std::string_view command) {
	auto i = command.find_last_of('/');
	std::string_view name;
	if (i != command.npos) {
		name = command.substr(++i);
	} else {
		name = command.substr();
	}
	std::cout << name.data() << ' ' << SKYFAUNA_VERSION_MAJOR << '.' <<
		SKYFAUNA_VERSION_MINOR << std::endl;
	std::cout << '\n' <<
	"Copyright (C) 2024 2025 Chloe Eather" << "\n\n" <<
	"Permission is hereby granted, free of charge, to any person obtaining a copy"
	<< '\n' <<
	"of this software and associated documentation files (the “Software”), to deal"
	<< '\n' <<
	"in the Software without restriction, including without limitation the rights to"
	<< '\n' <<
	"use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies"
	<< '\n' <<
	"of the Software, and to permit persons to whom the Software is furnished to do"
	<< '\n' << "so, subject to the following conditions:" << '\n' << "" << '\n' <<
	"The above copyright notice and this permission notice shall be included in all"
	<< '\n' << "copies or substantial portions of the Software." << "\n\n" <<
	"THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR"
	<< '\n' <<
	"IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,"
	<< '\n' <<
	"FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE"
	<< '\n' <<
	"AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER"
	<< '\n' <<
	"LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,"
	<< '\n' <<
	"OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE"
	<< '\n' << "SOFTWARE." << std::endl;
}

int main(int argc, char **argv) {
	std::vector<std::string> inputFilenames;
	std::string outputFilename = "out.txt";

	skyfauna::common::Logger::Init();

	if (argc < 2) {
		std::cerr << "No sources provided!" << std::endl;
		printUsage(argv[0]);
	}

	skyfauna::common::CommandLineParser<Arguments> cmdLine(argc, argv);
	cmdLine.SetLongOpts(longOpts);
	cmdLine.Parse();
	const auto& args = cmdLine.GetFlags();
	for (const auto& arg : args) {
		switch (arg.identifier) {
			case Arguments::HELP:
				printUsage(cmdLine.GetCommand());
				return 0;

			case Arguments::ARG_VERSION:
				printVersion(cmdLine.GetCommand());
				return 0;

			case Arguments::OUTPUT:
				if (arg.type == skyfauna::common::CmdLineArgType::STRING) {
					outputFilename = std::any_cast<std::string>(arg.arg);
				}
				break;

			case Arguments::ARG_LOG_LEVEL:
				if (arg.type == skyfauna::common::CmdLineArgType::STRING) {
					const std::string& level = std::any_cast<const std::string&>(arg.arg);
					if (level.compare("critical") == 0) {
						skyfauna::common::Logger::SetLogLevel(spdlog::level::critical);
					} else if (level.compare("error") == 0) {
						skyfauna::common::Logger::SetLogLevel(spdlog::level::err);
					} else if (level.compare("warn") == 0) {
						skyfauna::common::Logger::SetLogLevel(spdlog::level::warn);
					} else if (level.compare("info") == 0) {
						skyfauna::common::Logger::SetLogLevel(spdlog::level::info);
					} else if (level.compare("debug") == 0) {
						skyfauna::common::Logger::SetLogLevel(spdlog::level::debug);
					} else if (level.compare("trace") == 0) {
						skyfauna::common::Logger::SetLogLevel(spdlog::level::trace);
					} else {
						SF_ERROR("Unknown log level: \"{}\".", level);
						SF_ERROR("To see usage use {} --help", cmdLine.GetCommand());
						return 1;
					}
				}
				break;

			default:
				printUsage(cmdLine.GetCommand());
				return 1;

		}
	}
	
	for (auto f : cmdLine.GetUnhandledArgs()) {
		if (!std::filesystem::exists(f)) {
			SF_ERROR("Provided source \"{}\" file does not exist", f);
			return 1;
		}
		inputFilenames.emplace_back(f);
	}
	if (inputFilenames.size() < 1) {
		SF_ERROR("No source files proviced!");
		return 1;
	}

	skyfauna::Compiler c;
	for (auto inFile : inputFilenames) {
		c.AddFile(inFile);
	}
	c.Compile();
	SF_WARN("[NOT YET IMPLEMENTED] Output file: {}", outputFilename);

	return 0;
}


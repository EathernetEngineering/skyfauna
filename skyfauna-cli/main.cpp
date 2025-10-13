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
};

static std::array longOpts = std::to_array({
	skyfauna::common::CmdLineOption<Arguments>{ "help", 'h',
		skyfauna::common::CmdLineArgRequired::NO_ARG, Arguments::HELP },
	skyfauna::common::CmdLineOption<Arguments>{ "usage", 'h',
		skyfauna::common::CmdLineArgRequired::NO_ARG, Arguments::HELP },
	skyfauna::common::CmdLineOption<Arguments>{ "output", 'o',
		skyfauna::common::CmdLineArgRequired::REQUIRED_ARG, Arguments::OUTPUT },
	skyfauna::common::CmdLineOption<Arguments>{ "version", 'v',
		skyfauna::common::CmdLineArgRequired::NO_ARG, Arguments::ARG_VERSION }
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
	std::cout << "	-o, --output     File to write output to" << '\n';
	std::cout << "	-h, --help       Print this message" << '\n';
	std::cout << "	    --usage" << '\n';
	std::cout << "	-v, --version    Print program version" << std::endl;
}

static void printVersion(const std::string_view command) {
	auto i = command.find_last_of('/');
	std::string_view name;
	if (i != command.npos) {
		name = command.substr(i);
	} else {
		name = command.substr();
	}
	std::cout << name.data() << ' ' << SKYFAUNA_VERSION_MAJOR <<
		SKYFAUNA_VERSION_MINOR << std::endl;
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
	SF_INFO("[NOT YET IMPLEMENTED] Output file: {}", outputFilename);

	return 0;
}


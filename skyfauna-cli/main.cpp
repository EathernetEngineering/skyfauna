// SPDX-License-Identifier: MIT
// Copyright (C) 2025 Chloe Eather

#include <skyfauna/compiler.h>
#include <skyfauna/common/config.h>
#include <skyfauna/common/log.h>

#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <cstdio>
#include <getopt.h>


enum Arguments : uint32_t {
	ARG_VERSION
};

static const char shortOpts[] = "?o:";
static const option longOpts[] = {
	{ "help", no_argument, 0, '?' },
	{ "usage", no_argument, 0, '?' },
	{ "output", required_argument, 0, 'o' },
	{ "version", no_argument, 0, ARG_VERSION },
	{ 0, 0, 0, 0 }
};

static void printUsage(const char* progName) {
	fprintf(stdout, "Usage: %s [OPTION...] INPUT...\n"
	                "\t-o, --output     File to write output to\n"
	                "\t-?, --help       Print this message\n"
	                "\t    --usage\n"
	                "\t    --version    Print program version\n",
	        progName);
}

static void printVersion(const std::string& progName) {
	auto i = progName.find_last_of('/');
	std::string name;
	if (i != progName.npos) {
		name = std::string(progName.c_str() + i);
	} else {
		name = std::string(progName);
	}
	fprintf(stdout, "%s %d.%d\n",
		 name.c_str(),
		 SKYFAUNA_VERSION_MAJOR, SKYFAUNA_VERSION_MINOR);
}

int main(int argc, char **argv) {
	std::vector<std::string> inputFilenames;
	std::string outputFilename = "out.txt";

	skyfauna::common::Logger::Init();

	if (argc < 2) {
		std::cerr << "No sources provided!" << std::endl;
		printUsage(argv[0]);
	}

	int optionIndex = 0;
	for (;;) {
		int c = getopt_long(argc, argv, shortOpts, longOpts, &optionIndex);
		if (c == -1)
			break;

		switch (c) {
		case 'o':	
			outputFilename = optarg;
			break;

		case ARG_VERSION:	
			printVersion(argv[0]);
			exit(EXIT_SUCCESS);

		case '?':
		default:
			printUsage(argv[0]);
			exit(EXIT_SUCCESS);
		}
	}
	
	for (uint32_t i = optind; i < argc; i++) {
		inputFilenames.push_back(argv[i]);
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

	return 0;
}


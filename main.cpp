#include <cstdlib>
#include <cstdio>
#include <vector>
#include <string>

#include "config.h"
#include "solver.h"
#include "parser.h"

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

static void printVersion(const char* progName) {
	fprintf(stdout, "%s %s\n", progName, SKYFAUNA_VERSION);
}

int main(int argc, char **argv) {
	char* eq = nullptr;
	size_t eqSize;

	std::vector<std::string> inputFilenames;
	std::string outputFilename = "out.txt";

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

	fprintf(stdout, "Enter equation to be solved: ");
	fflush(stdout);
	
	getdelim(&eq, &eqSize, '\n', stdin);

	auto tokens = Tokenize(eq);
	free(eq);
	auto tokensrpn = GetReversePolishNotation(tokens);
	int64_t solution = SolveReversePolishNotation(tokensrpn);

	fprintf(stdout, "Solution: %li\n", solution);

	return 0;
}


// SPDX-License-Identifier: MIT
// Copyright (C) 2025 Chloe Eather

#include <skyfauna/config.h>
#include <skyfauna/solver.h>
#include <skyfauna/lex.h>

#include <exception>
#include <iostream>
#include <string>
#include <unordered_map>
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

static void printVersion(const char* progName) {
	fprintf(stdout, "%s %d.%d\n", progName, SKYFAUNA_VERSION_MAJOR, SKYFAUNA_VERSION_MINOR);
}

std::unordered_map<TokenType, std::string> g_TokenNames = {
	{ TokenType::INVALID,         "INVALID TOKEN" },
	{ TokenType::KEYWORD,         "KEYWORD" },
	{ TokenType::DECIMAL_LITERAL, "DECIMAL LITERAL" },
	{ TokenType::INT_LITERAL,     "INT LITERAL" },
	{ TokenType::CHAR_LITERAL,    "CHAR LITERAL" },
	{ TokenType::STRING_LITERAL,  "STRING LITERAL" },
	{ TokenType::SYMBOL,          "SYMBOL" },
	{ TokenType::ADD,             "ADD OPERATOR" },
	{ TokenType::INC,             "INC OPERATOR" },
	{ TokenType::SUB,             "SUB OPERATOR" },
	{ TokenType::DEC,             "DEC OPERATOR" },
	{ TokenType::MUL,             "MUL OPERATOR" },
	{ TokenType::DIV,             "DIV OPERATOR" },
	{ TokenType::MOD,             "MOD OPERATOR" },
	{ TokenType::XOR,             "XOR OPERATOR" },
	{ TokenType::AND,             "AND" },
	{ TokenType::OR,              "OR" },
	{ TokenType::BITAND,          "BITWISE AND" },
	{ TokenType::BITOR,           "BITWISE OR" },
	{ TokenType::BITSL,           "BIT SHIFT LEFT" },
	{ TokenType::BITSR,           "BIT SHIFT RIGHT" },
	{ TokenType::LESSTHAN,        "LESS THAN OPERATOR" },
	{ TokenType::GREATERTHAN,     "GREATER THAN OPERATOR" },
	{ TokenType::LESSTHANEQ,      "LESS THAN OR EQUAL OPERATOR" },
	{ TokenType::GREATERTHANEQ,   "GREATER THAN OR EQUAL OPERATOR" },
	{ TokenType::EQUAL,           "EQUAL OPERATOR" },
	{ TokenType::ASSIGN,          "ASSIGNMENT OPERATOR" },
	{ TokenType::SEMICOLON,       "SEMICOLON" },
	{ TokenType::COLON,           "COLON" },
	{ TokenType::PAREN_OPEN,      "PARENTHESES OPEN" },
	{ TokenType::PAREN_CLOSE,     "PARENTHESES CLOSE" },
	{ TokenType::BRACKET_OPEN,    "BRACKET OPEN" },
	{ TokenType::BRACKET_CLOSE,   "BRACKET CLOSE" },
	{ TokenType::BRACE_OPEN,      "BRACE OPEN" },
	{ TokenType::BRACE_CLOSE,     "BRACE CLOSE" }
};

int main(int argc, char **argv) {
	std::vector<std::string> inputFilenames;
	std::string outputFilename = "out.txt";

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

	std::vector<Token> tokens;
	for (auto inFile : inputFilenames) {
		try {
			tokens = Tokenize(inFile);
		} catch (std::exception& e) {
			fprintf(stderr, "%s\n", e.what());
		}
		for (auto token : tokens) {
			std::cout << token.text << "\t\t" << g_TokenNames[token.type] << "\n";
		}
	}

	//auto tokensrpn = GetReversePolishNotation(tokens);
	//int64_t solution = SolveReversePolishNotation(tokensrpn);

	//fprintf(stdout, "Solution: %li\n", solution);

	return 0;
}


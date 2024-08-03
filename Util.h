#ifndef CEE_UTIL_H_
#define CEE_UTIL_H_

#include <string>
#include <string_view>
#include <vector>
#include <cctype>

#include "Tokenizer.h"

inline std::string stripWhitespace(std::string_view str) {
	std::string stripped;

	for (auto c : str) {
		if (!std::isspace(c))
			stripped.push_back(c);
	}

	return stripped;
}

inline void printTokens(const std::vector<Token>& tokens) {
	for (auto token : tokens) {
		switch (token.first) {
			case OperatorType::ADD:
				fprintf(stderr, " %c ", '+');
				break;
			case OperatorType::SUB:
				fprintf(stderr, " %c ", '-');
				break;
			case OperatorType::DIV:
				fprintf(stderr, " %c ", '/');
				break;
			case OperatorType::MUL:
				fprintf(stderr, " %c ", '*');
				break;
			case OperatorType::MOD:
				fprintf(stderr, " %c ", '%');
				break;
			case OperatorType::POW:
				fprintf(stderr, " %c ", '^');
				break;
			case OperatorType::PAREN_OPEN:
				fprintf(stderr, " %c ", '(');
				break;
			case OperatorType::PAREN_CLOSE:
				fprintf(stderr, " %c ", ')');
				break;
			case OperatorType::LITERAL:
				fprintf(stderr, "%li", std::get<int64_t>(token.second));
				break;
		}
	}
}

#endif


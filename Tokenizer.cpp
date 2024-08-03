#include "Tokenizer.h"

std::vector<Token> Tokenize(std::string_view eq) {
	std::vector<Token> tokens;

	std::string buffer;
	OperatorType mcOp;

	auto flushBuffer =  [&tokens, &buffer, &mcOp](){
		if (buffer.length() > 0) {
			tokens.emplace_back(mcOp, std::stoi(buffer));
			buffer.clear();
		}
	};

	for (auto c : eq) {
		OperatorType t = OperatorType::LITERAL;
		switch (c) {
			case '+':
				t = OperatorType::ADD;
				break;

			case '-':
				t = OperatorType::SUB;
				break;

			case '*':
				t = OperatorType::MUL;
				break;

			case '/':
				t = OperatorType::DIV;
				break;

			case '%':
				t = OperatorType::MOD;
				break;

			case '^':
				t = OperatorType::POW;
				break;

			case '(':
				t = OperatorType::PAREN_OPEN;
				break;

			case ')':
				t = OperatorType::PAREN_CLOSE;
				break;

			default:
				if (std::isdigit(c)) {
					mcOp = OperatorType::LITERAL;
					buffer.push_back(c);
					
				}
				continue;
		}
		flushBuffer();
		tokens.emplace_back(t, std::monostate{});
	}
	flushBuffer();

	return tokens;
}


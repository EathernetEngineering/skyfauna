#include "Solver.h"
#include "Tokenizer.h"

#include <stack>
#include <unordered_map>
#include <cmath>

std::unordered_map<OperatorType, uint8_t> g_PrecedenceTable = {
	{ OperatorType::ADD, 1 },
	{ OperatorType::SUB, 1 },
	{ OperatorType::MUL, 2 },
	{ OperatorType::DIV, 2 },
	{ OperatorType::MOD, 2 },
	{ OperatorType::POW, 3 }
};

std::unordered_map<OperatorType, uint8_t> g_OperandCountTable = {
	{ OperatorType::ADD, 2 },
	{ OperatorType::SUB, 2 },
	{ OperatorType::MUL, 2 },
	{ OperatorType::DIV, 2 },
	{ OperatorType::MOD, 2 },
	{ OperatorType::POW, 2 }
};

static void ExecuteUnaryOperator(std::stack<Token>& solve) {
	Token op;
	Token rhs;
	Token lhs;

	op = solve.top();
	solve.pop();
	rhs = solve.top();
	solve.pop();

	switch (op.first) {
		default:
			fprintf(stderr, "Failed to execute unary operation.\n");
	}
}

static void ExecuteBinaryOperator(std::stack<Token>& solve) {
	Token op;
	Token rhs;
	Token lhs;

	op = solve.top();
	solve.pop();
	rhs = solve.top();
	solve.pop();
	lhs = solve.top();
	solve.pop();

	switch (op.first) {
		case OperatorType::ADD:
			solve.emplace(OperatorType::LITERAL, std::get<int64_t>(lhs.second) + std::get<int64_t>(rhs.second));
			break;
		
		case OperatorType::SUB:
			solve.emplace(OperatorType::LITERAL, std::get<int64_t>(lhs.second) - std::get<int64_t>(rhs.second));
			break;
		
		case OperatorType::MUL:
			solve.emplace(OperatorType::LITERAL, std::get<int64_t>(lhs.second) * std::get<int64_t>(rhs.second));
			break;
		
		case OperatorType::DIV:
			solve.emplace(OperatorType::LITERAL, std::get<int64_t>(lhs.second) / std::get<int64_t>(rhs.second));
			break;
		
		case OperatorType::MOD:
			solve.emplace(OperatorType::LITERAL, std::get<int64_t>(lhs.second) % std::get<int64_t>(rhs.second));
			break;
		
		case OperatorType::POW:
			solve.emplace(OperatorType::LITERAL, (int64_t)std::pow(std::get<int64_t>(lhs.second), std::get<int64_t>(rhs.second)));
			break;
		
		default:
			fprintf(stderr, "Failed to execute binary operation.\n");
	}
}

static void ExecuteTernaryOperator(std::stack<Token>& solve) {
	Token op;
	Token rhs;
	Token mhs;
	Token lhs;

	op = solve.top();
	solve.pop();
	rhs = solve.top();
	solve.pop();
	mhs = solve.top();
	solve.pop();
	lhs = solve.top();
	solve.pop();

	switch (op.first) {
		default:
			fprintf(stderr, "Failed to execute ternary operation.\n");
	}
}

std::vector<Token> GetReversePolishNotation(const std::vector<Token>& tokens) {
	std::stack<Token> holding;
	std::vector<Token> output;

	auto flushHolding = [&output, &holding](const Token& tryPush) {
		while (!holding.empty()) {
			output.push_back(holding.top());
			holding.pop();
			if (g_PrecedenceTable[tryPush.first] > g_PrecedenceTable[holding.top().first]) {
				break;
			}
		}
	};
	auto clearHolding = [&output, &holding]() {
		while (!holding.empty()) {
			output.push_back(holding.top());
			holding.pop();
		}
	};

	for (auto token : tokens) {
		if (token.first == OperatorType::LITERAL) {
			output.push_back(token);
			continue;
		}
		if (holding.empty()) {
			holding.push(token);
			continue;
		}
		Token stackTop = holding.top();
		if (g_PrecedenceTable[token.first] < g_PrecedenceTable[stackTop.first]) {
			flushHolding(token);
		}
		holding.push(token);
	}

	clearHolding();

	return output;
}

int64_t SolveReversePolishNotation(const std::vector<Token>& tokens) {
	std::stack<Token> solve;

	for (auto token : tokens) {
		solve.push(token);
		if (token.first == OperatorType::LITERAL) {
			continue;
		}
		switch (g_OperandCountTable[token.first]) {
			case 1:
				ExecuteUnaryOperator(solve);
				break;

			case 2:
				ExecuteBinaryOperator(solve);
				break;

			case 3:
				ExecuteTernaryOperator(solve);
				break;

			default:
				fprintf(stderr, "Failed to get operator operand count.\n");
		}
	}

	return std::get<int64_t>(solve.top().second);
}


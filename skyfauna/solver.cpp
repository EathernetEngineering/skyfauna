// SPDX-License-Identifier: MIT
// Copyright (C) 2025 Chloe Eather
//
#include <skyfauna/solver.h>

#include <stack>
#include <unordered_map>
#include <cmath>

std::unordered_map<OperatorType, uint8_t> g_PrecedenceTable = {
	{ TokenType::ADD, 1 },
	{ TokenType::SUB, 1 },
	{ TokenType::MUL, 2 },
	{ TokenType::DIV, 2 },
	{ TokenType::MOD, 2 },
	{ TokenType::POW, 3 },
	{ TokenType::FAC, 4 },
	{ TokenType::PAREN_OPEN, 0 },
	{ TokenType::PAREN_CLOSE, 0 }
};

std::unordered_map<OperatorType, uint8_t> g_OperandCountTable = {
	{ TokenType::ADD, 2 },
	{ TokenType::SUB, 2 },
	{ TokenType::MUL, 2 },
	{ TokenType::DIV, 2 },
	{ TokenType::MOD, 2 },
	{ TokenType::POW, 2 },
	{ TokenType::FAC, 1 }
};

static void ExecuteUnaryOperator(std::stack<Token>& solve) {
	Token op;
	Token rhs;

	op = solve.top();
	solve.pop();
	rhs = solve.top();
	solve.pop();

	switch (op.type) {
		case TokenType::FAC:
			solve.emplace(TokenType::LITERAL, [](int64_t n){ for (int64_t i = n - 1; i > 1; i--) { n *= i; } return n; }(std::get<int64_t>(rhs.value)));
			break;
		
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

	switch (op.type) {
		case TokenType::ADD:
			solve.emplace(TokenType::LITERAL, std::get<int64_t>(lhs.value) + std::get<int64_t>(rhs.value));
			break;
		
		case TokenType::SUB:
			solve.emplace(TokenType::LITERAL, std::get<int64_t>(lhs.value) - std::get<int64_t>(rhs.value));
			break;
		
		case TokenType::MUL:
			solve.emplace(TokenType::LITERAL, std::get<int64_t>(lhs.value) * std::get<int64_t>(rhs.value));
			break;
		
		case TokenType::DIV:
			solve.emplace(TokenType::LITERAL, std::get<int64_t>(lhs.value) / std::get<int64_t>(rhs.value));
			break;
		
		case TokenType::MOD:
			solve.emplace(TokenType::LITERAL, std::get<int64_t>(lhs.value) % std::get<int64_t>(rhs.value));
			break;
		
		case TokenType::POW:
			solve.emplace(TokenType::LITERAL, (int64_t)std::pow(std::get<int64_t>(lhs.value), std::get<int64_t>(rhs.value)));
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

	switch (op.type) {
		default:
			fprintf(stderr, "Failed to execute ternary operation.\n");
	}
}

std::vector<Token> GetReversePolishNotation(const std::vector<Token>& tokens) {
	std::stack<Token> holding;
	std::vector<Token> output;

	auto flushHolding = [&output, &holding](const Token& tryPush) {
		if (tryPush.type == TokenType::PAREN_CLOSE) {
			while (!holding.empty()) {
				if (holding.top().type == TokenType::PAREN_OPEN) {
					holding.pop();
					break;
				}
				output.push_back(holding.top());
				holding.pop();
			}
		} else {
			while (!holding.empty()) {
				if (g_PrecedenceTable[tryPush.type] > g_PrecedenceTable[holding.top().type]) {
					break;
				}
				output.push_back(holding.top());
				holding.pop();
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
		if (token.type == TokenType::LITERAL) {
			output.push_back(token);
			continue;
		}
		if (holding.empty()) {
			holding.push(token);
			continue;
		}
		if (token.type == TokenType::PAREN_OPEN) {
			holding.push(token);
			continue;
		}
		if (token.type == TokenType::PAREN_CLOSE) {
			flushHolding(token);
			continue;
		}
		Token stackTop = holding.top();
		if (g_PrecedenceTable[token.type] < g_PrecedenceTable[stackTop.type]) {
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
		if (token.type == TokenType::LITERAL) {
			continue;
		}
		switch (g_OperandCountTable[token.type]) {
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

	return std::get<int64_t>(solve.top().value);
}


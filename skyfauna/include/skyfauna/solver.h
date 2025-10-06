// SPDX-License-Identifier: MIT
// Copyright (C) 2025 Chloe Eather

#ifndef SKYFAUNA_SOLVER_H_
#define SKYFAUNA_SOLVER_H_

#include <skyfauna/lex.h>

#include <vector>

std::vector<Token> GetReversePolishNotation(const std::vector<Token>& tokens);
int64_t SolveReversePolishNotation(const std::vector<Token>& tokens);

#endif

#ifndef CEE_SOLVER_H_
#define CEE_SOLVER_H_

#include <vector>

#include "Tokenizer.h"

std::vector<Token> GetReversePolishNotation(const std::vector<Token>& tokens);
int64_t SolveReversePolishNotation(const std::vector<Token>& tokens);

#endif

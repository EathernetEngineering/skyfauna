#include <cstdlib>
#include <cstdio>

#include "Solver.h"
#include "Tokenizer.h"

#include "Util.h"

int main(int argc, char **argv) {
	char* eq = nullptr;
	size_t eqSize;

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


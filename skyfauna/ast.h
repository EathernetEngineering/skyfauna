// SPDX-License-Identifier: MIT
// Copyright (C) 2025 Chloe Eather

#ifndef SKYFAUNA_AST_H_
#define SKYFAUNA_AST_H_

#include <lex.h>

#include <memory>
#include <vector>

struct ASTNode {
	std::shared_ptr<ASTNode> parent;
	std::vector<ASTNode> children;

	Token token;
};

class AbstactSyntaxTree {
	
};


#endif


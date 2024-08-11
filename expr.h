#ifndef CEE_AST_H_
#define CEE_AST_H_

#include "Tokenizer.h"
#include <memory>

struct ASTNode {
	std::shared_ptr<ASTNode> parent;
	std::vector<ASTNode> children;

	Token token;
};

class AbstactSyntaxTree {
	
};

#endif


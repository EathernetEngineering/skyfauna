#ifndef CEE_TOKENIZER_H_
#define CEE_TOKENIZER_H_

#include <string>
#include <string_view>
#include <variant>
#include <utility>
#include <vector>
#include <cstdint>

enum class OperatorType : uint16_t {
	LITERAL,
	ADD,
	SUB,
	MUL,
	DIV,
	MOD,
	POW,
	FAC,
	PAREN_OPEN,
	PAREN_CLOSE
};

using Token = std::pair<OperatorType, std::variant<std::monostate, int64_t>>;

std::vector<Token> Tokenize(std::string_view eq);

#endif

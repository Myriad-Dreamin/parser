
#include "pch.h"
#include <iostream>
#include <vector>
#include <map>
#include <string>

#ifdef VS_COMPILE
#include <lexer-result.h>
#include <parser-common.h>
#else
#include "../LexerMock/lexer-result.h"
#include "../ParserCommon/parser-common.h"
#endif // VS_COMPILE


// ( num )
// ( num - num * num / num )
int main() {
	auto lexer_result = LexerResult(fromIstream(std::cin));
	for (auto x :lexer_result.tokens) {
	    std::cout << std::underlying_type_t<Token>(x) << " ";
	}
	std::cout << std::endl;
	// auto parser = Parser<Token, LexerResult, token_adapt_example>(lexer_result);
	// auto result = parser.parse();
	// std::cout << *result << std::endl;
	// delete result;
}

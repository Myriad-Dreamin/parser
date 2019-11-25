
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

namespace parse {
	using token_tt = std::underlying_type_t<Token>;
	template<typename token_t, class Source, class TokenTable>
	class RecursiveAParser {
		using istream = Source;
		using result_t = Result<token_t, UTerm>;
		using node_t = ASTNode<token_t, UTerm>;
		istream &ref;
		token_t token;

		result_t *result;
	public:
		RecursiveAParser(istream &ref): ref(ref) {}
		result_t* parse() {
			result = new result_t();
			result->code = ResultCode::Ok;
			auto hdl = result;
			read();
			parseE(result->rt = result->alloc(UTerm::E, true));
			result = nullptr;
			return hdl;
		}
	private:
		void read() {
			ref >> token;
		}
		void error() {
			result->code = ResultCode::Error;
			std::cout << "error " << token << std::endl;
		}
		void parseE(node_t* &rt) {
			parseT(rt->insert(result->alloc(UTerm::T, true)));
			if (token != TokenTable::eof) {
				parseED(rt->insert(result->alloc(UTerm::ED, true)));
			}
		}
		void parseED(node_t* &rt) {
			if (token == TokenTable::add || token == TokenTable::sub) {
				rt->insert(result->alloc(token));
				read();
				parseT(rt->insert(result->alloc(UTerm::T, true)));
				parseED(rt->insert(result->alloc(UTerm::ED, true)));
			}
		}
		void parseT(node_t* &rt) {
			parseF(rt->insert(result->alloc(UTerm::F, true)));
			if (token != TokenTable::eof) {
				parseTD(rt->insert(result->alloc(UTerm::TD, true)));
			}
		}
		void parseTD(node_t* &rt) {
			if (token == TokenTable::mul || token == TokenTable::div) {
				rt->insert(result->alloc(token));
				read();
				parseF(rt->insert(result->alloc(UTerm::F, true)));
				parseTD(rt->insert(result->alloc(UTerm::TD, true)));
			}
		}
		void parseF(node_t* &rt) {
			if (token == TokenTable::lbr) {
				rt->insert(result->alloc(token));
				read();
				parseE(rt->insert(result->alloc(UTerm::E, true)));
				if (token == TokenTable::rbr) {
					rt->insert(result->alloc(token));
					read();
				} else {
					error();
				}
			} else if (token == TokenTable::num) {
				rt->insert(result->alloc(token));
				read();
			} else {
				error();
			}
		}
	};
}
struct token_adapt_example {
	static const Token add = Token::Add;
	static const Token sub = Token::Sub;
	static const Token mul = Token::Mul;
	static const Token div = Token::Div;
	static const Token lbr = Token::Lbr;
	static const Token rbr = Token::Rbr;
	static const Token num = Token::Num;
	static const Token eof = Token::Eof;
};


// ( num )
// ( num - num * num / num )
int main() {
	auto lexer_result = LexerResult(fromIstream(std::cin));
	auto parser = parse::RecursiveAParser<Token, LexerResult, token_adapt_example>(lexer_result);
	auto result = parser.parse();
	std::cout << *result << std::endl;
	delete result;
}

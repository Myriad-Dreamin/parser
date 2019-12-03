
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

#ifdef DEBUG
		int32_t tab_cnt;
#endif

		result_t *result;
	public:
		RecursiveAParser(istream &ref): ref(ref) {
#ifdef DEBUG
			tab_cnt = 0;
#endif
		}
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
#ifdef DEBUG
			for (int i = 0; i < tab_cnt; i++)std::cout << "  ";
			std::cout << "parsing E" << std::endl;
			tab_cnt++;
#endif

			parseT(rt->insert(result->alloc(UTerm::T, true)));
#ifdef DEBUG
			tab_cnt--;
#endif
			if (token != TokenTable::eof) {
#ifdef DEBUG
				tab_cnt++;
#endif
				parseED(rt->insert(result->alloc(UTerm::ED, true)));
#ifdef DEBUG
				tab_cnt--;
#endif
			}
		}
		void parseED(node_t* &rt) {
#ifdef DEBUG
			for (int i = 0; i < tab_cnt; i++)std::cout << "  ";
			std::cout << "parsing ED" << std::endl;
#endif
			if (token == TokenTable::add || token == TokenTable::sub) {
#ifdef DEBUG
				for (int i = 0; i < tab_cnt; i++)std::cout << "  ";
				print::print("matched "); print::print(token, true);
#endif
				rt->insert(result->alloc(token));
				read();
#ifdef DEBUG
				tab_cnt++;
#endif
				parseT(rt->insert(result->alloc(UTerm::T, true)));
				parseED(rt->insert(result->alloc(UTerm::ED, true)));
#ifdef DEBUG
				tab_cnt--;
#endif
			}
		}
		void parseT(node_t* &rt) {
#ifdef DEBUG
			for (int i = 0; i < tab_cnt; i++)std::cout << "  ";
			std::cout << "parsing T" << std::endl;
#endif
#ifdef DEBUG
			tab_cnt++;
#endif
			parseF(rt->insert(result->alloc(UTerm::F, true)));
#ifdef DEBUG
			tab_cnt--;
#endif
			if (token != TokenTable::eof) {
#ifdef DEBUG
				tab_cnt++;
#endif
				parseTD(rt->insert(result->alloc(UTerm::TD, true)));
#ifdef DEBUG
				tab_cnt--;
#endif
			}
		}
		void parseTD(node_t* &rt) {
#ifdef DEBUG
			for (int i = 0; i < tab_cnt; i++)std::cout << "  ";
			std::cout << "parsing TD" << std::endl;
#endif
			if (token == TokenTable::mul || token == TokenTable::div) {
#ifdef DEBUG
				for (int i = 0; i < tab_cnt; i++)std::cout << "  ";
				print::print("matched "); print::print(token, true);
#endif
				rt->insert(result->alloc(token));
				read();
#ifdef DEBUG
				tab_cnt++;
#endif
				parseF(rt->insert(result->alloc(UTerm::F, true)));
				parseTD(rt->insert(result->alloc(UTerm::TD, true)));
#ifdef DEBUG
				tab_cnt--;
#endif
			}
		}
		void parseF(node_t* &rt) {
#ifdef DEBUG
			for (int i = 0; i < tab_cnt; i++)std::cout << "  ";
			std::cout << "parsing F" << std::endl;
#endif
			if (token == TokenTable::lbr) {
#ifdef DEBUG
				for (int i = 0; i < tab_cnt; i++)std::cout << "  ";
				print::print("matched "); print::print(token, true);
#endif
				rt->insert(result->alloc(token));
				read();
#ifdef DEBUG
				tab_cnt++;
#endif
				parseE(rt->insert(result->alloc(UTerm::E, true)));
#ifdef DEBUG
				tab_cnt--;
#endif
				if (token == TokenTable::rbr) {
#ifdef DEBUG
					for (int i = 0; i < tab_cnt; i++)std::cout << "  ";
					print::print("matched "); print::print(token, true);
#endif
					rt->insert(result->alloc(token));
					read();
				} else {
					error();
				}
			} else if (token == TokenTable::num) {
#ifdef DEBUG
				for (int i = 0; i < tab_cnt; i++)std::cout << "  ";
				print::print("matched "); print::print(token, true);
#endif
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
// ( num + num / ( num + num * num ) )
int main() {
	auto lexer_result = LexerResult(fromIstream(std::cin));
	auto parser = parse::RecursiveAParser<Token, LexerResult, token_adapt_example>(lexer_result);
	auto result = parser.parse();
	std::cout << *result << std::endl;
	delete result;
}


#include "pch.h"
#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <string>
#include <cstring>
#include <fstream>
#include <type_traits>
#include <regex>

#ifdef VS_COMPILE
#include <lexer-result.h>
#include <parser-common.h>
#include <functional.h>
#include <string-algorithms.h>
#include <print.h>
#else
#include "../LexerMock/lexer-result.h"
#include "../ParserCommon/parser-common.h"
#include "../ParserCommon/functional.h"
#include "../ParserCommon/string-algorithms.h"
#include "../ParserCommon/print.h"
#endif // VS_COMPILE


namespace parse {
	
template<typename term_t, typename uterm_t, class grammar_traits>
class LL1Grammar {
	using model_t = Model<term_t, uterm_t>;
	using string = typename model_t::string;
	using strvec = typename model_t::strvec;
	using symbol = typename model_t::symbol;
	using grammar_t = LL1Grammar<term_t, uterm_t, grammar_traits>;
	
	std::map<string, symbol> &sym_table;
	std::map<symbol, std::set<symbol>* > first;
	std::vector<Production<symbol>> &prods;

public:
	LL1Grammar(model_t &model) :
		sym_table(model.sym_table),
		first(),
		prods(model.prods) { init(); }

	LL1Grammar(model_t *model) :
		sym_table(model->sym_table),
		first(),
		prods(model->prods) { init(); }

	LL1Grammar(std::map<string, symbol> &sym_table,
		std::vector<Production<symbol>> &prods) :
		sym_table(sym_table),
		first(),
		prods(prods) { init(); }

	virtual ~LL1Grammar() {
		for (auto &x: first) {
			delete x.second;
			x.second = nullptr;
		}
	}
private:
	void init() {
		calculate_first_fixed_point<grammar_t, grammar_traits>(*this);
		for (auto &c : first) {
			print::print(c.first);
			print::print(' ');
			print::print(*c.second, true);
		}
	}

	template<class u, class v>
	friend void calculate_first_fixed_point(u &g);
};

}


struct grammar_traits_example {
	static const auto epsilon = Token::Eps;
	static const auto dollar = Token::Dol;
};


int main() {

	auto model = parse::G<Token, parse::UTerm>("grammar.gr");
	if (model->is_error()) {
		std::cout << model->error() << std::endl;
	}

	using LL1 = parse::LL1Grammar<Token, parse::UTerm, grammar_traits_example>;
	auto grammar = LL1(model);

	delete model;

}

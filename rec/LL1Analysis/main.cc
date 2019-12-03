
#include "pch.h"
#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <string>
#include <cstring>
#include <fstream>
#include <sstream>
#include <type_traits>
#include <regex>
#include <queue>
#include <cassert>
#include <stack>
#include <algorithm>
#ifdef VS_COMPILE
#include <lexer-result.h>
#include <parser-common.h>
#else
#include "../LexerMock/lexer-result.h"
#include "../ParserCommon/parser-common.h"
#endif // VS_COMPILE


namespace parse {


template<class grammar_traits, class Policy=BasicLLGrammar<grammar_traits>>
class LL1Grammar : public Policy {
public:
	using model_t = typename grammar_traits::model_t;
	using string = typename grammar_traits::string;
	using strvec = typename grammar_traits::strvec;
	using symbol_t = typename grammar_traits::symbol_t;
	using production_t = typename grammar_traits::production_t;
	
	using grammar_t = LL1Grammar<grammar_traits>;
private:
	std::map<string, symbol_t> &sym_table;
	std::vector<production_t> &prods;
	symbol_t begin_symbol;
	
	std::map<symbol_t, std::set<symbol_t>* > first;
	std::map<symbol_t, std::set<symbol_t>* > follow;

	using action_map = std::map<symbol_t, action_space::action*>;
	std::map<symbol_t, action_map*> table;
public:
	LL1Grammar(model_t &model) :
		first(),
		follow(),
		table(),
		sym_table(model.sym_table),
		prods(model.prods),
		begin_symbol(model.begin_symbol) { init(); }

	LL1Grammar(model_t *model) :
		first(),
		follow(),
		table(),
		sym_table(model->sym_table),
		prods(model->prods),
		begin_symbol(model->begin_symbol) { init(); }

	LL1Grammar(std::map<string, symbol_t> &sym_table,
		std::vector<production_t> &prods,
		const symbol_t &begin_symbol) :
		first(),
		follow(),
		table(),
		sym_table(sym_table),
		prods(prods),
		begin_symbol(begin_symbol) { init(); }
	
	virtual ~LL1Grammar() {
		delete_first_symbols(*this);
		delete_follow_symbols(*this);
		for (auto &x: table) {
			for (auto &y : *x.second) {
				delete y.second;
				y.second = nullptr;
			}
			delete x.second;
			x.second = nullptr;
		}
		// std::cout << "deconstruct" << std::endl;
	}

	// LL1Grammar(const LL1Grammar) {

	// } 

private:
	void init() {
		calculate_first_fixed_point<grammar_t, grammar_traits>(*this);
		calculate_follow_fixed_point<grammar_t, grammar_traits>(*this);

		for (int i = prods.size() - 1; i >= 0; i--) {
			auto &prod = prods[i];
			for (int j = 0; j < prod.rhs.size(); j++) {
				if (prod.rhs[j] == grammar_traits::epsilon) {
					for (int k = j + 1; k < prod.rhs.size(); k++) {
						prod.rhs[k - 1] = prod.rhs[k];
					}
					prod.rhs.pop_back();
				}
			}
		}
		/*
		for (auto &c : first) {
			print::print(c.first);
			print::print(' ');
			print::print(c.second);
			print::print(' ');
			print::print(*c.second, true);
		}
		std::cout << "fist end" << std::endl;
		for (auto &c : epsable) {
			print::print(c.first);
			print::print(' ');
			print::print(bool(c.second), true);
		}
		std::cout << std::endl;
		for (auto &c : follow) {
			print::print(c.first);
			print::print(' ');
			print::print(*c.second, true);
		}
		print::print("begin symbol ");
		print::print(this->begin_symbol, true);
		*/
		build();

		Policy::table = &table;
		Policy::begin_symbol = begin_symbol;
		Policy::follow = follow;
	}

	void build() {
		for (auto &c : sym_table) {
			table[c.second] = new action_map();
		}
		// check()
		std::set<symbol_t> mset;
		for (auto &prod : prods) {
			get_first1<grammar_t, grammar_traits>(*this, prod, mset);
			auto &acmp = *table[prod.lhs];
			for (auto &sym : mset) {
				if (acmp.count(sym)) {
					std::stringstream s("conflict ");
					print::print("prod:", false, s);
					print::print(prod, false, s);
					print::print("mset:", false, s);
					print::print(mset, false, s);
					print::print("symbol:", false, s);
					print::print(sym, false, s);
					throw std::logic_error(s.str());
				}
				action_space::action *a = new action_space::replace_action1<symbol_t>(prod.lhs, prod.rhs);
				acmp[sym] = a;
			}
		}
	}

	template<class u, class v>
	friend void calculate_first_fixed_point(u &g);

	template<class u, class v>
	friend void calculate_follow_fixed_point(u &g);


	template<class u, class v>
	friend void get_first1(u &g, typename v::production_t &prod,
		std::set<typename v::symbol_t> &res);

	template<class u, class v>
	friend void get_first1_nc(u &g, typename v::production_t &prod,
		std::set<typename v::symbol_t> &res);

	template<class u>
	friend void delete_first_symbols(u &g);
	template<class u>
	friend void delete_follow_symbols(u &g);
};


}


struct grammar_traits_example {
	static const auto eof = Token::Eof;
	static const auto epsilon = Token::Eps;
	static const auto dollar = Token::Dol;
};


int main() {

	auto model = parse::G<Token, parse::UTerm>("grammar.gr");
	if (model->is_error()) {
		std::cout << model->error() << std::endl;
	}

	using gt = parse::basic_grammar_traits<Token, parse::UTerm, grammar_traits_example>;
	using LL1 = parse::LL1Grammar<gt>;
	auto grammar = LL1(model);
	auto f = std::fstream("test0.dat", std::ios::in);


	auto flow = LexerResult(fromIstream(f));

	f.close();

	auto result = grammar.work(flow);

	std::cout << *result << std::endl;
	delete result;
	delete model;

}

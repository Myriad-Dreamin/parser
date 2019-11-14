
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
#include <queue>
#include <cassert>
#include <algorithm>
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
namespace action_space {
	struct action {};
	template<typename symbol_t>
	struct replace_action: public action {
		const std::vector<symbol_t> &reduce;
		const std::vector<symbol_t> &produce;
	};
	template<typename symbol_t>
	struct replace_action1: public action {
		const symbol_t &reduce;
		const std::vector<symbol_t> &produce;
	};
	struct shift_action: public action { int shift_number; };
	struct error_action: public action { const std::string error_info; };
}

struct InitFailed: std::exception {

};


template<class grammar_traits,
	class container=std::map<grammar_traits, action_space::action*>>
class BasicLLGrammar {
public:
	using model_t = typename grammar_traits::model_t;
	using string = typename grammar_traits::string;
	using strvec = typename grammar_traits::strvec;
	using symbol_t = typename grammar_traits::symbol_t;
	using production_t = typename grammar_traits::production_t;
	
	using grammar_t = BasicLLGrammar<grammar_traits>;

protected:
	container &table;
	BasicLLGrammar(container &table) : table(table) {}
public:
	void reset(){};
	action_space::action act(const symbol_t &s){};
};

template<class Grammar, typename grammar_traits>
void get_first1_nc(
	Grammar &g,
	typename grammar_traits::production_t &prod,
	std::set<typename grammar_traits::symbol_t> &res) {
	for(auto &sym :prod.rhs) {
		res.merge(*first[sym]);
		if (!epsable(sym)) {
			return;
		}
	}
	res.merge(*follow[prod.lhs]);
}

template<class Grammar, typename grammar_traits>
void get_first1(
	Grammar &g,
	typename grammar_traits::production_t &prod,
	std::set<typename grammar_traits::symbol_t> &res) {
	res.clear();
	get_first1_nc(first, sequence, res);
}


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
	std::map<symbol_t, uint8_t> epsable;

	std::map<symbol_t, action_space::action*> table;
public:
	LL1Grammar(model_t &model) :
		first(),
		follow(),
		epsable(),
		table(),
		sym_table(model.sym_table),
		prods(model.prods),
		begin_symbol(model.begin_symbol) { init(); }

	LL1Grammar(model_t *model) :
		first(),
		follow(),
		epsable(),
		table(),
		sym_table(model->sym_table),
		prods(model->prods),
		begin_symbol(model->begin_symbol) { init(); }

	LL1Grammar(std::map<string, symbol_t> &sym_table,
		std::vector<production_t> &prods,
		const symbol_t &begin_symbol) :
		first(),
		follow(),
		epsable(),
		table(),
		sym_table(sym_table),
		prods(prods),
		begin_symbol(begin_symbol) { init(); }

	virtual ~LL1Grammar() {
		for (auto &x: first) {
			delete x.second;
			x.second = nullptr;
		}
		for (auto &x: follow) {
			delete x.second;
			x.second = nullptr;
		}
		for (auto &x: epsable) {
			delete x.second;
			x.second = nullptr;
		}
		for (auto &x: table) {
			delete x.second;
			x.second = nullptr;
		}
	}
private:
	void init() {
		calculate_first_fixed_point<grammar_t, grammar_traits>(*this);
		calculate_epsilonable<grammar_t, grammar_traits>(*this);
		calculate_follow_fixed_point<grammar_t, grammar_traits>(*this);
		for (auto &c : first) {
			print::print(c.first);
			print::print(' ');
			print::print(*c.second, true);
		}
		std::cout << std::endl;
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

		// check()
		std::set<symbol_t> mset;
		for (auto prod: prods) {
			get_first1(*this, prod, mset);
			print::print(prod);
			print::print(' ');
			print::print(mset);
		}

	}

	template<class u, class v>
	friend void calculate_first_fixed_point(u &g);
	
	template<class u, class w>
	friend void calculate_epsilonable(u &g);

	template<class u, class v>
	friend void calculate_follow_fixed_point(u &g);
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

	using gt = parse::basic_grammar_traits<Token, parse::UTerm, grammar_traits_example>;
	using LL1 = parse::LL1Grammar<gt>;
	auto grammar = LL1(model);

	delete model;

}

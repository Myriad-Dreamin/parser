
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

template<typename term_t, typename uterm_t, class grammar_traits>
class LL1Grammar;

template<typename term_t, typename uterm_t, class grammar_traits>
void calculate_epsilonable(LL1Grammar<term_t, uterm_t, grammar_traits> &g);

template<typename term_t, typename uterm_t, class grammar_traits>
class LL1Grammar {
public:
	using model_t = Model<term_t, uterm_t>;
	using string = typename model_t::string;
	using strvec = typename model_t::strvec;
	using symbol_t = typename model_t::symbol_t;
	using production_t = Production<symbol_t>;
	using grammar_t = LL1Grammar<term_t, uterm_t, grammar_traits>;
private:
	std::map<string, symbol_t> &sym_table;
	std::vector<production_t> &prods;
	symbol_t begin_symbol;
	
	std::map<symbol_t, std::set<symbol_t>* > first;
	std::map<symbol_t, std::set<symbol_t>* > follow;
	std::map<symbol_t, uint8_t> epsable;
public:
	LL1Grammar(model_t &model) :
		sym_table(model.sym_table),
		first(),
		prods(model.prods),
		begin_symbol(model.begin_symbol) { init(); }

	LL1Grammar(model_t *model) :
		sym_table(model->sym_table),
		first(),
		prods(model->prods),
		begin_symbol(model->begin_symbol) { init(); }

	LL1Grammar(std::map<string, symbol_t> &sym_table,
		std::vector<production_t> &prods,
		const symbol_t &begin_symbol) :
		sym_table(sym_table),
		first(),
		prods(prods),
		begin_symbol(begin_symbol) { init(); }

	virtual ~LL1Grammar() {
		for (auto &x: first) {
			delete x.second;
			x.second = nullptr;
		}
	}
private:
	void init() {
		calculate_first_fixed_point<grammar_t, grammar_traits>(*this);
		calculate_epsilonable<term_t, uterm_t, grammar_traits>(*this);
		calculate_follow_fixed_point<term_t, uterm_t, grammar_traits>(*this);
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
	}

	template<class u, class v>
	friend void calculate_first_fixed_point(u &g);
	
	template<typename u, typename v, class w>
	friend void calculate_epsilonable(LL1Grammar<u,v,w> &g);

	template<class u, class v>
	friend void calculate_follow_fixed_point(u &g);

};


namespace epsilonable {
	enum ExploreState{
		No,
		Yes,
		Unknown,
	};
}

template<typename term_t, typename uterm_t, class grammar_traits>
void calculate_epsilonable(LL1Grammar<term_t, uterm_t, grammar_traits> &g) {
	using namespace epsilonable;
	using Grammar = LL1Grammar<term_t, uterm_t, grammar_traits>;
	using production_t = typename Grammar::production_t;
	std::vector<production_t*> conts;
	
	for(auto &ss: g.sym_table) {
		g.epsable[ss.second] = ss.second.is_unterm() ? ExploreState::Unknown: ExploreState::No;
	}
	for (auto &prod: g.prods) {
		auto &sym = prod.lhs;
		if (g.epsable[sym] != ExploreState::Unknown) {
			continue;
		}
		if (prod.rhs.size() == 1 && prod.rhs[0] == grammar_traits::epsilon) {
			g.epsable[sym] = ExploreState::Yes;
			continue;
		}
		bool cont = true;
		for (auto &rsym:prod.rhs) {
			auto &rst = g.epsable[rsym];
			if (rst == ExploreState::No) {
				cont = false;
			}
		}
		if (cont) {
			conts.push_back(&prod);
		}
	}
	bool changed, prod_eps;
	do {
		changed = false;
		for (typename std::make_signed<size_t>::type
			i = conts.size() - 1; i >= 0; i--) {
			auto &prod = *(conts[i]);
			if (g.epsable[prod.lhs] != ExploreState::Unknown) {
				std::swap(conts[i], conts.back());
				conts.pop_back();
				continue;
			}
			prod_eps = true;
			for (auto &rsym:prod.rhs) {
				auto &rst = g.epsable[rsym];
				if (rst == ExploreState::No) {
					prod_eps = false;
					g.epsable[prod.lhs] = ExploreState::No;
					changed = true;
					std::swap(conts[i], conts.back());
					conts.pop_back();
					break;
				}
				if (rst == ExploreState::Unknown) {
					prod_eps = false;
					break;
				}
			}
			if (prod_eps) {
				g.epsable[prod.lhs] = ExploreState::Yes;
				changed = true;
			}
		}
	} while(changed);
	for (auto &ss : g.sym_table) {
		if (g.epsable[ss.second] == ExploreState::Unknown) {
			g.epsable[ss.second] = ExploreState::No;
		}
	}
}

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

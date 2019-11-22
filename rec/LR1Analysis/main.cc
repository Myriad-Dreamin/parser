

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
#include <chrono>
#include <algorithm>
#ifdef VS_COMPILE
#include <lexer-result.h>
#include <parser-common.h>
#else
#include "../LexerMock/lexer-result.h"
#include "../ParserCommon/parser-common.h"
#endif // VS_COMPILE

namespace parse {


template<class grammar_traits, class Policy=BasicLRGrammar<grammar_traits> >
class LR1Grammar : public Policy {
public:
	using model_t = typename grammar_traits::model_t;
	using string = typename grammar_traits::string;
	using strvec = typename grammar_traits::strvec;
	using symbol_t = typename grammar_traits::symbol_t;
	using production_t = typename grammar_traits::production_t;
	using state_id_t = typename grammar_traits::state_id_t;

	using grammar_t = LR1Grammar<grammar_traits>;
private:
	std::map<string, symbol_t> &sym_table;
	std::vector<production_t> &prods;
	symbol_t begin_symbol;

	std::map<symbol_t, std::set<symbol_t>* > first;
	std::map<symbol_t, uint8_t> epsable;

	using action_map = std::map<symbol_t, action_space::action*>;
	std::map<state_id_t, action_map*> table;
public:
	LR1Grammar(model_t &model):
		first(),
		epsable(),
		table(),
		sym_table(model.sym_table),
		prods(model.prods),
		begin_symbol(model.begin_symbol) {
		init();
	}

	LR1Grammar(model_t *model):
		first(),
		epsable(),
		table(),
		sym_table(model->sym_table),
		prods(model->prods),
		begin_symbol(model->begin_symbol) {
		init();
	}

	LR1Grammar(std::map<string, symbol_t> &sym_table,
		std::vector<production_t> &prods,
		const symbol_t &begin_symbol):
		first(),
		epsable(),
		table(),
		sym_table(sym_table),
		prods(prods),
		begin_symbol(begin_symbol) {
		init();
	}

	virtual ~LR1Grammar() {
		delete_first_symbols(*this);
		for (auto &x : table) {
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
		calculate_epsilonable<grammar_t, grammar_traits>(*this);
		using context_t = LR1ActionCalculationContext<grammar_traits>;
		calculate_LR_1_items<grammar_traits>(prods, first, epsable, begin_symbol, [&](
			context_t &context) {
			/*std::cout << state.size() << std::endl;
			for (int i = 0; i < state.size(); i++) {
				for (auto j : automa.at_e(i)) {
					std::cout << "(" << i << ", " << j.to << ", " << j.w << ")" << std::endl;
				}
			}
			for (auto &st : state) {
				print::print(*st, true);
			}*/

			for (int i = 0; i < context.state.size(); i++) {
				auto acmp = new action_map();
				table[i] = acmp;
				for (auto j : context.automa.at_e(i)) {
					auto &sym = j.w;
					if (sym.is_unterm()) {
#ifdef DEBUG
						if ((*acmp).count(sym)) {
							std::cout << "conflict item"; print::print(sym, true);
						}
#endif
						(*acmp)[sym] = new action_space::goto_action<state_id_t>(j.to);
					} else {
#ifdef DEBUG
						if ((*acmp).count(sym)) {
							std::cout << "conflict item"; print::print(sym, true);
						}
#endif
						(*acmp)[sym] = new action_space::shift_action<state_id_t>(j.to);
					}
				}

				for (auto &item : *context.state[i]) {
					if (item.first.second == prods[item.first.first].rhs.size()) {
#ifdef DEBUG
						if ((*acmp).count(item.second)) {
							std::cout << "conflict item"; print::print(item.second, true);
						}
#endif
						if (prods[item.first.first].lhs == begin_symbol) {
							(*acmp)[item.second] = new action_space::accept_action();
						} else {
							(*acmp)[item.second] = new action_space::replace_action1<symbol_t>(prods[item.first.first].lhs, prods[item.first.first].rhs);
						}
					}
				}
			}

			for (const auto &acmp : table) {
				print::print(acmp.first);
				print::print(' ');
				print::print(*acmp.second, true);
			}
		});
		Policy::init(&table, begin_symbol);
		// Policy::table = &table;
		// Policy::begin_symbol = begin_symbol;
		// Policy::follow = follow;
	}


	template<class u, class v>
	friend void calculate_first_fixed_point(u &g);

	template<class u, class w>
	friend void calculate_epsilonable(u &g);

	template<class u, class v>
	friend void get_first1(u &g, typename v::production_t &prod,
		std::set<typename v::symbol_t> &res);

	template<class u, class v>
	friend void get_first1_nc(u &g, typename v::production_t &prod,
		std::set<typename v::symbol_t> &res);


	template<class u, class v, class Iterable>
	friend typename std::enable_if<is_iterable::is_iterable<Iterable>::value>::type get_first1(u &g, Iterable &prod,
		std::set<typename v::symbol_t> &res);

	template<class u, class v, class Iterable>
	friend typename std::enable_if<is_iterable::is_iterable<Iterable>::value>::type get_first1_nc(u &g, Iterable &prod,
		std::set<typename v::symbol_t> &res);

	template<class u>
	friend void delete_first_symbols(u &g);
	template<class u>
	friend void delete_follow_symbols(u &g);
	template<class u>
	friend void delete_epsable_symbols(u &g);
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

	auto lr1 = parse::LR1Grammar<gt>(model);

	
	auto f = std::fstream("test0.dat", std::ios::in);
	auto flow = LexerResult(fromIstream(f));

	auto result = lr1.work(flow);

	f.close();

	if (result != nullptr) {
		std::cout << *result << std::endl;
		delete result;
	}

	delete model;

}

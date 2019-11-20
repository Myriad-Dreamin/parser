
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


	template<class grammar_traits,
		class container = std::map<
		typename grammar_traits::symbol_t,
		std::map<typename grammar_traits::symbol_t, action_space::action*>*>>
		class BasicLLGrammar {
		public:
			using model_t = typename grammar_traits::model_t;
			using string = typename grammar_traits::string;
			using strvec = typename grammar_traits::strvec;
			using symbol_t = typename grammar_traits::symbol_t;
			using term_t = typename grammar_traits::term_t;
			using production_t = typename grammar_traits::production_t;
			using node_t = typename grammar_traits::node_t;
			using grammar_t = BasicLLGrammar<grammar_traits, container>;
			using result_t = Result<typename grammar_traits::term_t,
				typename grammar_traits::uterm_t>;

		protected:
			container *table;
			std::stack<node_t*> stack;
			symbol_t next_symbol, begin_symbol;
			std::map<symbol_t, std::set<symbol_t>*> follow;
			int error_count;
		public:

			template<typename IStream>
			void read(IStream &in, symbol_t &sym) {
				term_t token;
				in >> token;
				sym = symbol_t(token, symbol_t::term_v);
			}

			action_space::action *act(const symbol_t &s) {};
			void reset() {
				error_count = 0;
				while (stack.size()) {
					stack.pop();
				}
				next_symbol = grammar_traits::epsilon;
			}
			template<class IStream>
			result_t *work(IStream &is) {
				reset();
				auto result = new result_t();
				auto rt = result->alloc(begin_symbol);
				result->rt = rt;
				stack.push(rt);
				read(is, next_symbol);
				while (stack.size()) {
					if (next_symbol == grammar_traits::eof) {
						return result;
					}
					auto state = stack.top();
					if (state->symbol.is_unterm()) {
						auto &acmp = *(*table)[state->symbol];
						if (acmp.count(next_symbol)) {
							auto d0 = dynamic_cast<action_space::replace_action1<symbol_t>*>(acmp[next_symbol]);
							if (d0 != nullptr) {
								auto &prod = *d0;
								if (state->symbol != prod.reduce) {
									if (follow[state->symbol]->count(next_symbol)) {
										stack.pop();
									} else {
										read(is, next_symbol);
									}
									error_count++;
								} else {
									stack.pop();
									for (auto &sym : prod.produce) {
										state->insert(result->alloc(sym));
									}
									for (auto iter = state->ch.rbegin(); iter != state->ch.rend(); iter++) {
										stack.push(*iter);
									}
								}
							} else {
								auto d1 = dynamic_cast<action_space::error_action*>(acmp[next_symbol]);
								if (d1 != nullptr) {
									auto &info = *d1;
									error_count++;
								} else {
									auto d2 = dynamic_cast<action_space::synch_action*>(acmp[next_symbol]);
									if (d2 != nullptr) {
										auto &info = *d2;
										stack.pop();
										error_count++;
									}
								}
							}
						} else {
							read(is, next_symbol);
							error_count++;
						}
					} else {
						if (next_symbol != state->symbol) {
							error_count++;
						} else {
							stack.pop();
						}
						read(is, next_symbol);
					}
				}
				return result;
			}

			template<class IStream>
			result_t *work2(IStream &is) {
				reset();
				auto result = new result_t();
				auto rt = result->alloc(begin_symbol);
				result->rt = rt;
				stack.push(rt);
				read(is, next_symbol);
				while (stack.size()) {
					if (next_symbol == grammar_traits::eof) {
						return result;
					}
					auto state = stack.top();
					if (state->symbol.is_unterm()) {
						auto &acmp = *(*table)[state->symbol];
						if (acmp.count(next_symbol)) {
							auto d0 = dynamic_cast<action_space::replace_action1<symbol_t>*>(acmp[next_symbol]);
							if (d0 != nullptr) {
								auto &prod = *d0;
								if (state->symbol != prod.reduce) {
									if (follow[state->symbol]->count(next_symbol)) {
										stack.pop();
									} else {
										read(is, next_symbol);
									}
									error_count++;
								} else {
									stack.pop();
									for (auto &sym : prod.produce) {
										state->insert(result->alloc(sym));
									}
									for (auto iter = state->ch.rbegin(); iter != state->ch.rend(); iter++) {
										stack.push(*iter);
									}
								}
							} else {
								auto d1 = dynamic_cast<action_space::error_action*>(acmp[next_symbol]);
								if (d1 != nullptr) {
									auto &info = *d1;
									error_count++;
								}
							}
						} else {
							read(is, next_symbol);
							error_count++;
						}
					} else {
						if (next_symbol != state->symbol) {
							error_count++;
						} else {
							stack.pop();
						}
						read(is, next_symbol);
					}
				}
				return result;
			}
	};


	template<class grammar_traits, class Policy = BasicLLGrammar<grammar_traits>>
	class LL1Grammar: public Policy {
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

		using action_map = std::map<symbol_t, action_space::action*>;
		std::map<symbol_t, action_map*> table;
	public:
		LL1Grammar(model_t &model):
			first(),
			follow(),
			epsable(),
			table(),
			sym_table(model.sym_table),
			prods(model.prods),
			begin_symbol(model.begin_symbol) {
			init();
		}

		LL1Grammar(model_t *model):
			first(),
			follow(),
			epsable(),
			table(),
			sym_table(model->sym_table),
			prods(model->prods),
			begin_symbol(model->begin_symbol) {
			init();
		}

		LL1Grammar(std::map<string, symbol_t> &sym_table,
			std::vector<production_t> &prods,
			const symbol_t &begin_symbol):
			first(),
			follow(),
			epsable(),
			table(),
			sym_table(sym_table),
			prods(prods),
			begin_symbol(begin_symbol) {
			init();
		}

		virtual ~LL1Grammar() {
			for (auto &x : first) {
				delete x.second;
				x.second = nullptr;
			}
			for (auto &x : follow) {
				delete x.second;
				x.second = nullptr;
			}
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
			calculate_follow_fixed_point<grammar_t, grammar_traits>(*this);
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

			for (auto &c : sym_table) {
				table[c.second] = new action_map();
			}
			// check()
			std::set<symbol_t> mset;
			for (auto &prod : prods) {
				get_first1<grammar_t, grammar_traits>(*this, prod, mset);
				print::print(prod);
				print::print(' ');
				print::print(mset, true);
				auto &acmp = *table[prod.lhs];
				for (auto &sym : mset) {
					if (acmp.count(sym)) {
						std::stringstream s("conflct ");
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
			for (auto &c : table) {
				print::print(c.first);
				print::print(' ');
				print::print(*c.second, true);
			}

			Policy::table = &table;
			Policy::begin_symbol = begin_symbol;
			Policy::follow = follow;
		}

		template<class u, class v>
		friend void calculate_first_fixed_point(u &g);

		template<class u, class w>
		friend void calculate_epsilonable(u &g);

		template<class u, class v>
		friend void calculate_follow_fixed_point(u &g);

		template<class u, class v>
		friend void get_first1(u &g, typename v::production_t &prod,
			std::set<typename v::symbol_t> &res);

		template<class u, class v>
		friend void get_first1_nc(u &g, typename v::production_t &prod,
			std::set<typename v::symbol_t> &res);
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

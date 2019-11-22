
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
#define DEBUG

namespace parse {


template<class grammar_traits,
	class container = std::map<
	typename grammar_traits::state_id_t,
	std::map<typename grammar_traits::symbol_t, action_space::action*>*>>
	class BasicLRGrammar {
	public:
		using model_t = typename grammar_traits::model_t;
		using string = typename grammar_traits::string;
		using strvec = typename grammar_traits::strvec;
		using symbol_t = typename grammar_traits::symbol_t;
		using term_t = typename grammar_traits::term_t;
		using production_t = typename grammar_traits::production_t;
		using node_t = typename grammar_traits::node_t;
		using state_id_t = typename grammar_traits::state_id_t;
		using grammar_t = BasicLRGrammar<grammar_traits, container>;
		using result_t = Result<typename grammar_traits::term_t,
			typename grammar_traits::uterm_t>;

	protected:
		container *table;
		std::stack<std::pair<node_t*, state_id_t>> stack;
		symbol_t next_symbol, begin_symbol;
		int error_count;

		void init(container *m_table, const symbol_t &bsym) {
			this->table = m_table;
			this->begin_symbol = bsym;
		}
	public:

		template<typename IStream>
		void read(IStream &in, symbol_t &sym) {
			term_t token;
			in >> token;
			sym = symbol_t(token, symbol_t::term_v);
			if (sym == grammar_traits::eof) {
				sym = grammar_traits::dollar;
			}
		}

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
			stack.push(std::make_pair(rt, 0));
			read(is, next_symbol);
			while (stack.size()) {
#ifdef DEBUG
				std::cout << "state size " << stack.size() << " ";
#endif // DEBUG

				// print::print(stack.top(), true);
				auto acmp = (*table)[stack.top().second];
				if (!acmp->count(next_symbol)) {
#ifdef DEBUG
					std::cout << "state " << stack.top().second << " find action failed ";
					print::print(next_symbol, true);
#endif // DEBUG
					error_count++;
					read(is, next_symbol);
					continue;
				}
				auto norm_action = (*acmp)[next_symbol];
				if (auto action = dynamic_cast<action_space::shift_action<state_id_t>*>(norm_action)) {
#ifdef DEBUG
					std::cout << "state " << stack.top().second << " shift in " << next_symbol << std::endl;
#endif // DEBUG
					stack.push({result->alloc(next_symbol), action->to_state});
					read(is, next_symbol);

				} else if (auto action = dynamic_cast<action_space::replace_action1<symbol_t>*>(norm_action)) {
					auto mrt = result->alloc(action->reduce);
#ifdef DEBUG
					std::cout << "state " << stack.top().second << " reduced ";
#endif // DEBUG
					for (typename std::make_signed<size_t>::type i = action->produce.size() - 1; i >= 0; i--) {
#ifdef DEBUG
						if (stack.top().first->symbol != action->produce[i]) {
							std::cout << "unmatched reduction " << i << " ";
							print::print(action->produce, true);
						}
#endif
						mrt->ch.push_back(stack.top().first);
						stack.pop();
					}
					std::reverse(mrt->ch.begin(), mrt->ch.end());
					auto acmp2 = (*table)[stack.top().second];
#ifdef DEBUG
					if (!acmp2->count(action->reduce)) {
						std::cout << "unmatched reduction " << stack.top().second << " "; print::print(action->reduce, true);
					}
#endif
#ifdef DEBUG
					std::cout << *mrt << " to state " << (dynamic_cast<action_space::goto_action<state_id_t>*>(
						(*acmp2)[action->reduce]))->to_state << std::endl;
#endif // DEBUG
					stack.push({mrt, (dynamic_cast<action_space::goto_action<state_id_t>*>(
						(*acmp2)[action->reduce]))->to_state});
				} else if (auto action = dynamic_cast<action_space::accept_action*>(norm_action)) {
					result->rt->insert(stack.top().first);
#ifdef DEBUG
					assert(("stack.size() == 2", stack.size() == 2));
#endif
					// std::cout << "stack size " << stack.size() << std::endl;
					if (error_count != 0) {
						result->code = ResultCode::Error;
					}
					return result;
				}
#ifdef DEBUG
				else {
					std::cout << "action (" << stack.top().second << ", ";
					print::print(next_symbol);
					std::cout << ") is not determinted" << std::endl;
				}
#endif // DEBUG

			}
			if (error_count != 0) {
				result->code = ResultCode::Error;
			}
			return result;
		}
};


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
		
		build();
		Policy::init(&table, begin_symbol);
		// Policy::table = &table;
		// Policy::begin_symbol = begin_symbol;
		// Policy::follow = follow;
	}

	using item_t = std::pair<std::pair<int, int>, symbol_t>;
	std::vector<item_t> items;

	using hashed_item_t = int64_t;
	using state_set = std::set<item_t>;
	std::vector<state_set*> state;
	std::map<hashed_item_t, state_id_t> hash_set;
	graph::WeightedGraph<state_id_t, symbol_t, 500, 500 * 10> automa;
	int64_t seed, seed2;
	const int64_t mod = 1000000000 + 9;
	
	state_set &alloc(state_id_t &idx) {
		state.push_back(new state_set());
		idx = state.size() - 1;
		return *state.back();
	}

	

	void build() {
		automa.init(499);
		seed = std::chrono::system_clock::now().time_since_epoch().count() % mod;
		state_id_t idx; state_set &mset = alloc(idx);
		bool proceed = false;
		for (typename std::make_signed<size_t>::type i = static_cast<typename std::make_signed<size_t>::type>(prods.size()) - 1; i >= 0;i--) {
			auto &prod = prods[i];
			for (size_t j = 0; j < prod.rhs.size(); j++) {
				if (prod.rhs[j] == grammar_traits::epsilon) {
					for (size_t k = j + 1; k < prod.rhs.size(); k++) {
						prod.rhs[k - 1] = prod.rhs[k];
					}
					prod.rhs.pop_back();
				}
			}
			if (prod.lhs == begin_symbol) {
				if (proceed) {
					throw std::invalid_argument("must with only one production reduce to begin symbol");
				}
				proceed = true;
				mset.insert(item_t{{i, 0}, grammar_traits::dollar});
			}
		}

		if (mset.size() == 0) {
			throw std::invalid_argument("must with only one production reduce to begin symbol");
		}
		walk(extend(idx));
		/*std::cout << state.size() << std::endl;
		for (int i = 0; i < state.size(); i++) {
			for (auto j : automa.at_e(i)) {
				std::cout << "(" << i << ", " << j.to << ", " << j.w << ")" << std::endl;
			}
		}
		for (auto &st : state) {
			print::print(*st, true);
		}*/

		for (int i = 0; i < state.size(); i++) {
			auto acmp = new action_map();
			table[i] = acmp;
			for (auto j : automa.at_e(i)) {
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

			for (auto &item : *state[i]) {
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

		for (auto &st : state) {
			delete st;
		}

		for (const auto &acmp : table) {
			print::print(acmp.first);
			print::print(' ');
			print::print(*acmp.second, true);
		}
	}
	
	void walk(state_id_t idx) {
		if (idx < 0) {
			return;
		}
		// std::cout << "walking " << idx; print::print(*state[idx], true);
		std::set<symbol_t> sym_set;
		for (auto &item : *state[idx]) {
			if (prods[item.first.first].rhs.size() == item.first.second) {
				continue;
			}
			sym_set.insert(prods[item.first.first].rhs[item.first.second]);
		}
		for (auto &sym: sym_set) {
			state_id_t next_state; state_set &mset = alloc(next_state);
			for (auto &item : *state[idx]) {
				if (prods[item.first.first].rhs.size() == item.first.second) {
					continue;
				}
				auto &sym0 = prods[item.first.first].rhs[item.first.second];
				if (sym0 == sym) {
					mset.insert(item_t{{item.first.first, item.first.second + 1}, item.second});
				}
			}
			// std::cout << "walking " << idx; print::print(sym); print::print(*state[idx]); print::print("->"); print::print(mset, true);
			auto true_next_state = extend(next_state);
			automa.addedge(idx, true_next_state, sym);
			if (true_next_state == next_state) walk(true_next_state);
		}
	}
	
	state_id_t extend(state_id_t idx) {
		auto &mset = state[idx];
		for (auto &item : *mset) {
			if (prods[item.first.first].rhs.size() == item.first.second) {
				continue;
			}
			auto &sym = prods[item.first.first].rhs[item.first.second];
			if (sym.is_unterm()) {
				std::set<symbol_t> fset;
				std::vector<symbol_t> atom_v; atom_v.push_back(item.second);
				auto combine_v = combine(slice(prods[item.first.first].rhs, item.first.second + 1), slice(atom_v));
				get_first1<grammar_t, grammar_traits, decltype(combine_v)>(*this, combine_v, fset);
				fset.erase(grammar_traits::epsilon);
				extend_to(*mset, fset, sym);
			}
		}
		auto hashed_item = calculate_hash(*mset);
		if (hash_set.count(hashed_item)) {
			swap(state[idx], state.back());
			state.pop_back();
			delete mset;
			return hash_set[hashed_item];
		}
		hash_set[hashed_item] = idx;
		return idx;
	}

	void extend_to(state_set &mset, std::set<symbol_t> lookahead, const symbol_t &next_sym) {
		for (int i = prods.size() - 1; i >= 0; i--) {
			auto &prod = prods[i];
			if (prod.lhs == next_sym) {
				auto item = std::make_pair(i, 0);
				for (auto &asym : lookahead) {
					auto ritem = std::make_pair(item, asym);
					if (!mset.count(ritem)) {
						mset.insert(ritem);
						if (prod.rhs.size()) {
							auto &sym = prod.rhs[0];
							if (sym.is_unterm()) {
								std::set<symbol_t> fset;
								std::vector<symbol_t> atom_v; atom_v.push_back(asym);
								auto combine_v = combine(slice(prod.rhs, 1), slice(atom_v));
								get_first1<grammar_t, grammar_traits, decltype(combine_v)>(*this, combine_v, fset);
								fset.erase(grammar_traits::epsilon);
								extend_to(mset, fset, sym);
							}
						}
					}
				}
			}
		}
	}


	hashed_item_t calculate_hash(state_set &mset) {
		hashed_item_t result = 0;
		static const int64_t _233_2 = (2333333LL * 2333333LL) % mod;
		for (auto &item : mset) {
			result = (seed * result + item.first.first * _233_2 % mod + item.first.second * 2333333LL % mod + item.second.hash()) % mod;
		}
		return result;
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

	using state_id_t = int32_t;
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

#pragma once

#include "graph.h"
#include "action.h"
#include "calculation.h"
#include <utility>
#include <functional>
#include <chrono>

namespace parse {

	template<class grammar_traits>
	struct LR1ActionCalculationContext {
		using context_t = LR1ActionCalculationContext<grammar_traits>;
		using symbol_t = typename grammar_traits::symbol_t;
		using state_id_t = typename grammar_traits::state_id_t;
		using production_t = typename grammar_traits::production_t;
		using action_map = std::map<symbol_t, action_space::action*>;
		using item_t = std::pair<std::pair<int, int>, symbol_t>;
		using hashed_item_t = int64_t;
		using state_set = std::set<item_t>;

		std::vector<item_t> items;
		std::vector<state_set*> state;
		std::map<hashed_item_t, state_id_t> hash_set;
		graph::WeightedGraph<state_id_t, symbol_t, 500, 500 * 10> automa;
		int64_t seed, seed2;
		const int64_t mod = 1000000000 + 9;

		std::vector<production_t> &prods;
		std::map<symbol_t, std::set<symbol_t>* > first;
		symbol_t &begin_symbol;

		LR1ActionCalculationContext(
			std::vector<production_t> &prods, std::map<symbol_t, std::set<symbol_t>* > &first, symbol_t &begin_symbol):
			prods(prods),
			first(first),
			begin_symbol(begin_symbol)
		{}

		~LR1ActionCalculationContext() {
			for (auto &st : state) {
				delete st;
			}
		}

		state_set &alloc(state_id_t &idx) {
			state.push_back(new state_set());
			idx = state.size() - 1;
			return *state.back();
		}

		context_t &build() {
			automa.init(499);
			seed = std::chrono::system_clock::now().time_since_epoch().count() % mod;
			state_id_t idx; state_set &mset = alloc(idx);
			bool proceed = false;
			for (typename std::make_signed<size_t>::type i = static_cast<typename std::make_signed<size_t>::type>(prods.size()) - 1; i >= 0; i--) {
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
			return *this;
		}

		context_t &callback(const std::function<void(context_t &)> &cb) {
			cb(*this);
			return *this;
		}

	private:


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
			for (auto &sym : sym_set) {
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
					get_first1<context_t, grammar_traits, decltype(combine_v)>(*this, combine_v, fset);
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
									get_first1<context_t, grammar_traits, decltype(combine_v)>(*this, combine_v, fset);
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
	};

	template<typename grammar_traits>
	void calculate_LR_1_items(
		std::vector<typename grammar_traits::production_t> &prods,
		std::map<typename grammar_traits::symbol_t,
		std::set<typename grammar_traits::symbol_t>* > &first,
		typename grammar_traits::symbol_t &begin_symbol,
		const std::function<void(LR1ActionCalculationContext<grammar_traits>&)> &callback) {
		LR1ActionCalculationContext<grammar_traits>(prods, first, begin_symbol).build().callback(callback);
	}


}


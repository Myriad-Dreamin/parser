#pragma once

#include <set>
#include "iterable.h"

namespace parse {

	template<class Grammar, class grammar_traits>
	void calculate_first_fixed_point(Grammar &g) {

		using symbol_t = typename Grammar::symbol_t;
		for (auto &x : g.sym_table) {
			auto &sym = x.second;
			auto s = new std::set<symbol_t>();
			if (!sym.is_unterm()) {
				s->insert(sym);
			}
			g.first[sym] = s;
		}

		bool changed;
		do {
			changed = false;
			for (auto &symset : g.first) {
				if (!symset.first.is_unterm()) {
					continue;
				}
				auto &sym = symset.first;
				auto set = symset.second;
				size_t ls = set->size();
				// print::print("testing ");
				// print::print(set); print::print(" ");
				// print::print(symset.second); print::print(" "); print::print(sym); print::print(" ");
				// print::print(*set, true);
				// lower_bound or sort() && iter will be better
				for (auto &prod : g.prods) {
					if (prod.lhs == sym) {
						// print::print("using ");
						// print::print(prod, true);

						for (auto &rsym : prod.rhs) {
							auto &rset = *g.first[rsym];
							// print::print("merge ");
							// print::print(*set);
							// print::print(" ");
							// print::print(rset, true);

							set->insert(rset.begin(), rset.end());
							if (!rset.count(grammar_traits::epsilon)) {
								break;
							}
						}
					}
				}
				if (set->size() != ls) {
					changed = true;
				}
			}
		} while (changed);
		// for (auto &c : g.first) {
		// 	print::print(c.first);
		// 	print::print(' ');
		// 	print::print(c.second);
		// 	print::print(' ');
		// 	print::print(*c.second, true);
		// }
	}

	namespace epsilonable {
		enum ExploreState {
			No,
			Yes,
			Unknown,
		};
	}

	template<class Grammar, class grammar_traits>
	void calculate_epsilonable(Grammar &g) {
		using namespace epsilonable;
		using production_t = typename Grammar::production_t;
		std::vector<production_t*> conts;

		for (auto &ss : g.sym_table) {
			g.epsable[ss.second] = ss.second.is_unterm() ? ExploreState::Unknown : ExploreState::No;
		}
		g.epsable[grammar_traits::epsilon] = ExploreState::Yes;
		for (auto &prod : g.prods) {
			auto &sym = prod.lhs;
			if (g.epsable[sym] != ExploreState::Unknown) {
				continue;
			}
			if (prod.rhs.size() == 1 && prod.rhs[0] == grammar_traits::epsilon) {
				g.epsable[sym] = ExploreState::Yes;
				continue;
			}
			bool cont = true;
			for (auto &rsym : prod.rhs) {
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
				for (auto &rsym : prod.rhs) {
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
		} while (changed);
		for (auto &ss : g.sym_table) {
			if (g.epsable[ss.second] == ExploreState::Unknown) {
				g.epsable[ss.second] = ExploreState::No;
			}
		}
	}

	template<class Grammar, class grammar_traits>
	void calculate_follow_fixed_point(Grammar &g) {
		using symbol_t = typename Grammar::symbol_t;
		auto &beg = g.begin_symbol;
		for (auto &x : g.sym_table) {
			auto &sym = x.second;
			auto s = new std::set<symbol_t>();
			if (sym == beg) {
				s->insert(grammar_traits::dollar);
			}
			g.follow[sym] = s;
		}

		bool changed;
		do {
			changed = false;
			for (auto &prod : g.prods) {
				auto &lhs = prod.lhs;
				auto &rhs = prod.rhs;
				std::set<symbol_t> mset(*g.follow[lhs]);
				mset.erase(grammar_traits::epsilon);
				for (typename std::make_signed<size_t>::type
					i = rhs.size() - 1; i >= 0; i--) {
					auto &rsym = rhs[i];
					if (rsym.is_unterm()) {
						auto sz = g.follow[rsym]->size();
						g.follow[rsym]->insert(mset.begin(), mset.end());
						if (g.follow[rsym]->size() - sz) {
							changed = true;
						}
					}
					if (!g.epsable[rsym]) {
						mset.clear();
					}
					mset.insert(g.first[rsym]->begin(), g.first[rsym]->end());
					mset.erase(grammar_traits::epsilon);
				}
			}
		} while (changed);
	}

	template<class Grammar, typename grammar_traits, class Iterable>
	typename std::enable_if<is_iterable::is_iterable<Iterable>::value>::type get_first1_nc(
		Grammar &g,
		Iterable &sequence,
		std::set<typename grammar_traits::symbol_t> &res) {
		for (auto &sym : sequence) {
			res.insert(g.first[sym]->begin(), g.first[sym]->end());
			if (!g.epsable[sym]) {
				return;
			}
		}
	}

	template<class Grammar, typename grammar_traits, class Iterable>
	typename std::enable_if<is_iterable::is_iterable<Iterable>::value>::type get_first1(
		Grammar &g,
		Iterable &sequence,
		std::set<typename grammar_traits::symbol_t> &res) {
		res.clear();
		get_first1_nc<Grammar, grammar_traits>(g, sequence, res);
	}


	template<class Grammar, typename grammar_traits>
	void get_first1_nc(
		Grammar &g,
		typename grammar_traits::production_t &prod,
		std::set<typename grammar_traits::symbol_t> &res) {
		for (auto &sym : prod.rhs) {
			res.insert(g.first[sym]->begin(), g.first[sym]->end());
			if (!g.epsable[sym]) {
				return;
			}
		}
		res.insert(g.follow[prod.lhs]->begin(), g.follow[prod.lhs]->end());
	}

	template<class Grammar, typename grammar_traits>
	void get_first1(
		Grammar &g,
		typename grammar_traits::production_t &prod,
		std::set<typename grammar_traits::symbol_t> &res) {
		res.clear();
		get_first1_nc<Grammar, grammar_traits>(g, prod, res);
	}

	
	template<class Grammar>
	void delete_first_symbols(Grammar &g) {
		for (auto &x : g.first) {
			delete x.second;
			x.second = nullptr;
		}
	}


	template<class Grammar>
	void delete_follow_symbols(Grammar &g) {
		for (auto &x : g.follow) {
			delete x.second;
			x.second = nullptr;
		}
	}


	template<class Grammar>
	void delete_epsable_symbols(Grammar &g) {
	}


}

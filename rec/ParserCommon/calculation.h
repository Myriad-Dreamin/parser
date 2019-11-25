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
				for (auto &prod : g.prods) {
					if (prod.lhs == sym) {
                        bool flag = true;
						for (auto &rsym : prod.rhs) {
							auto &rset = *g.first[rsym];
							if (rset.count(grammar_traits::epsilon) && !set->count(grammar_traits::epsilon)) {
                                set->insert(rset.begin(), rset.end());
                                set->erase(grammar_traits::epsilon);
                            } else {
                                set->insert(rset.begin(), rset.end());
                            }
							if (!rset.count(grammar_traits::epsilon)) {
                                flag = false;
								break;
							}
						}
                        if (flag) {
                            set->insert(grammar_traits::epsilon);
                        }
					}
				}
				if (set->size() != ls) {
					changed = true;
				}
			}
		} while (changed);
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
					if (!g.first[rsym]->count(grammar_traits::epsilon)) {
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
			if (!g.first[sym]->count(grammar_traits::epsilon)) {
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
			if (!g.first[sym]->count(grammar_traits::epsilon)) {
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

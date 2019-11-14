
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
void calculate_follow_fixed_point(LL1Grammar<term_t, uterm_t, grammar_traits> &g);

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
	friend class CalculateEpsilonableContext;
	template<typename u, typename v, class w>
	friend void calculate_epsilonable(LL1Grammar<u,v,w> &g);

	template<typename u, typename v, class w>
	friend void calculate_follow_fixed_point(LL1Grammar<u,v,w> &g);

};


template<typename term_t, typename uterm_t, class grammar_traits>
class CalculateEpsilonableContext {
	enum ExploreState: uint8_t {
		No,
		Yes,
		Unknown,
		Exploring,
	};
	using Grammar = LL1Grammar<term_t, uterm_t, grammar_traits>;
	using symbol_t = typename Grammar::symbol_t;
	using production_t = typename Grammar::production_t;
	Grammar &g;

	std::vector<production_t*> conts;
	// template<typename node_type, int64_t VSize, int64_t ESize, bool undirected=true, typename id_type=uint32_t>
	// struct Graph {
	// 	using graph_t = Graph<node_type, VSize, ESize, undirected, id_type>;
	// 	struct edge {
	// 		id_type nx;
	// 		node_type to;
	// 	} e[ESize<<1];
	// 	std::map<node_type, id_type> head;
	// 	id_type mal;
	// 	Graph() { init(); }
	// 	Graph(int n) { init(n+1); }

	// 	void addedge(const node_type &u, const node_type &v) {
	// 		auto &edge = e[mal], &hu = head[u];
	// 		edge.to = v;
	// 		edge.nx = hu; hu = mal++;
	// 	}
	// 	struct partial_iterator {
	// 		const graph_t &view;
	// 		node_type u;
	// 		partial_iterator(const graph_t &g, const node_type &u) :view(g), u(u) {};
			
	// 		struct iterator {
	// 			const graph_t &view;
	// 			id_type edge_id;
	// 			iterator(const graph_t &g, id_type edge_id) :view(g), edge_id(edge_id) {}
	// 			iterator &operator++() {
	// 				edge_id = view.e[edge_id].nx;
	// 				return *this;
	// 			}
	// 			iterator &operator++(int) {
	// 				return operator++();
	// 			}
	// 			bool operator!=(const iterator &i) const {
	// 				return edge_id != i;
	// 			}
	// 			bool operator==(const iterator &i) const {
	// 				return edge_id == i;
	// 			}
	// 			const node_type &operator*() const {
	// 				return view.e[edge_id].to;
	// 			}
	// 		};
			
	// 		iterator begin() {
	// 			return iterator(view, view.head[u]);
	// 		}
	// 		iterator end() {
	// 			return iterator(view, 0);
	// 		}
	// 	};
	// 	partial_iterator at(const node_type &u) {
	// 		return partial_iterator(g, u);
	// 	}
	// private:
	// 	void init(int n = VSize) {
	// 		#if DEBUG
	// 		assert(("n < VSize", n < VSize));
	// 		#endif
	// 		head.clear();
	// 		mal = 1;
	// 	}
	// };

	// Graph<symbol_t, 200, 200 * 200> dep_graph;
	// std::map<symbol, int> deg;
	// std::queue<symbol> Q;

	// void addedge(const production_t &prod) {
	// 	deg[prod.lhs] += prod.rhs.size();
	// 	for (auto &rsym: prod.rhs) {
	// 		dep_graph.addedge(rsym, prod.lhs);
	// 	}
	// }
public:
	CalculateEpsilonableContext(Grammar &g) :g(g) /*, dep_graph()*/ {}
	void work() {
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
		for (auto &c : g.epsable) {
			print::print(c.first);
			print::print(' ');
			print::print(int(c.second), true);
		}
	}
private:
	// uint8_t check(symbol_t &sym) {
	// 	g.epsable[sym] = ExploreState::Exploring;
	// 	print::print("next");
	// 	print::print(sym, true);
	// 	if (g.prods.size() == 0) {
	// 		g.epsable[sym] = ExploreState::No;
	// 		return;
	// 	}
	// 	for (auto &prod: g.prods) {
	// 		if (prod.lhs != sym) {
	// 			continue;
	// 		}
	// 		if (prod.rhs.size() == 1 && prod.rhs[0] == grammar_traits::epsilon) {
	// 			g.epsable[sym] = ExploreState::Yes;
	// 			return;
	// 		} else {
	// 			bool cont = true;
	// 			for (auto &rsym:prod.rhs) {
	// 				auto rst = g.epsable[rsym];
	// 				if (rst == ExploreState::Unknown) {
	// 					rst = check(rsym);
	// 				}
	// 				if (rst == ExploreState::No) {
	// 					cont = false;
	// 				}
	// 			}
	// 			if (cont) {
	// 				addedge(rsym.prod);
	// 			}
	// 		}
	// 	}
	// 	g.epsable[sym] = ExploreState::Unknown;
		
	// 	// for (auto &prod: g.prods) {
	// 	// 	if (prod.lhs != sym) {
	// 	// 		continue;
	// 	// 	}
	// 	// 	for (auto &sym: prod.rhs) {
	// 	// 		if (g.epsable[ss.second] == ExploreState::Unknown) {
	// 	// 			dfs(ss.second);
	// 	// 		}
	// 	// 		// if (g.epsable[])
	// 	// 	}
	// 	// 	print::print("\n");
	// 	// 	// if (!symset.first.is_unterm()) {
	// 	// 	// 	continue;
	// 	// 	// }
	// 	// 	// auto &sym = symset.first;
	// 	// 	// auto &set = *symset.second;
	// 	// 	// size_t ls = set.size();
	// 	// 	// for (auto) {
				
	// 	// 	// }
	// 	// }
	// }
};



template<typename term_t, typename uterm_t, class grammar_traits>
void calculate_epsilonable(LL1Grammar<term_t, uterm_t, grammar_traits> &g) {
	CalculateEpsilonableContext(g).work();
}


template<typename term_t, typename uterm_t, class grammar_traits>
void calculate_follow_fixed_point(LL1Grammar<term_t, uterm_t, grammar_traits> &g) {
	using Grammar = LL1Grammar<term_t, uterm_t, grammar_traits>;
	using symbol_t = typename Grammar::symbol_t;
	auto &beg = g.begin_symbol;
	for (auto &x: g.sym_table) {
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
		for (auto &prod: g.prods) {
			auto &lhs = prod.lhs;
			auto &rhs = prod.rhs;
			std::set<symbol_t> mset(*g.follow[lhs]);
			mset.erase(grammar_traits::epsilon);
			for (typename std::make_signed<size_t>::type
				i = rhs.size() - 1; i >= 0; i--) {
				auto &rsym = rhs[i];
				if (rsym.is_unterm()) {
					auto sz = g.follow[rsym]->size();
					g.follow[rsym]->merge(mset);
					if (g.follow[rsym]->size() - sz) {
						changed = true;
					}
				}
				if (!g.epsable[rsym]) {
					mset.clear();
				}
				mset.merge(*g.first[rsym]);
				mset.erase(grammar_traits::epsilon);
			}
		}
	} while(changed);
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

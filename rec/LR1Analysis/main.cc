
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


namespace graph {
	template<typename id_type, typename node_type>
	struct Edge {
		id_type nx;
		node_type to;
		Edge() {
			nx = 0;
		};
		Edge(id_type nx, node_type to): nx(nx), to(to) {};
	};
	
	template<typename id_type, typename node_type>
	std::ostream &operator<< (std::ostream &os, Edge<id_type, node_type> &e) {
		os << "(" << e.nx << ", " << e.to << ")";
		return os;
	}


	template<typename id_type, typename node_type, typename weighter>
	struct WeightedEdge: public Edge<id_type, node_type> {
		weighter w;

		WeightedEdge() :Edge<id_type, node_type>() {};
		WeightedEdge(const id_type &nx, const node_type &to, const weighter &w): Edge<id_type, node_type>(nx, to), w(w) {};
	};

	template<typename id_type, typename node_type, typename weighter>
	std::ostream &operator<< (std::ostream &os, WeightedEdge<id_type, node_type, weighter> &e) {
		os << "(" << e.nx << ", " << e.to << ", " << e.w << ")";
		return os;
	}

	template<typename node_type, int64_t VSize, int64_t ESize,
		bool undirected=false, typename id_type=uint32_t, typename edge_t=Edge<id_type, node_type> >
	struct Graph {
	 	using graph_t = Graph<node_type, VSize, ESize, undirected, id_type, edge_t>;
		static constexpr int64_t esize_v = undirected ? (ESize << 1) : ESize;
		edge_t e[esize_v];
	 	std::map<node_type, id_type> head;
	 	id_type mal;
	 	Graph() { init(); }
	 	Graph(int n) { init(n+1); }

		void addedge(const node_type &u, const node_type &v) {
			if CXX17_STATIC_CONDITION (undirected) {
				_addedge(u, v);
				_addedge(v, u);
			} else {
				_addedge(u, v);
			}
		}


		struct __base_edge_iterator {
			const graph_t &view;
			id_type edge_id;
			__base_edge_iterator(const graph_t &g, id_type edge_id):view(g), edge_id(edge_id) {}
			__base_edge_iterator &operator++() {
				edge_id = view.e[edge_id].nx;
				return *this;
			}
			__base_edge_iterator &operator++(int) {
				return operator++();
			}
			bool operator!=(const __base_edge_iterator &i) const {
				return edge_id != i.edge_id;
			}
			bool operator==(const __base_edge_iterator &i) const {
				return edge_id == i.edge_id;
			}
		};

		struct node_iterator: public __base_edge_iterator {
			using f_class = __base_edge_iterator;
			node_iterator(const graph_t &g, id_type edge_id): f_class(g, edge_id) {}
			const node_type &operator*() const {
				return f_class::view.e[f_class::edge_id].to;
			}
		};

		struct edge_iterator: public __base_edge_iterator {
			using f_class = __base_edge_iterator;
			edge_iterator(const graph_t &g, id_type edge_id): f_class(g, edge_id) {}
			const edge_t &operator*() const {
				return f_class::view.e[f_class::edge_id];
			}
		};

		template<class iterator>
	 	struct partial_iterator {
	 		const graph_t &view;
	 		const node_type &u;
	 		partial_iterator(const graph_t &g, const node_type &u) :view(g), u(u) {};

	 		iterator begin() {
				if (!view.head.count(u)) {
					return end();
				}
	 			return iterator(view, view.head.at(u));
	 		}
	 		iterator end() {
	 			return iterator(view, 0);
	 		}
	 	};

		using partial_node_iterator = partial_iterator<node_iterator>;
		partial_node_iterator at(const node_type &u) {
			return partial_node_iterator(*this, u);
		}

		using partial_edge_iterator = partial_iterator<edge_iterator>;
		partial_edge_iterator at_e(const node_type &u) {
			return partial_edge_iterator(*this, u);
		}

		void init(int n = VSize) {
#ifdef DEBUG
			assert(("n < VSize", n < VSize));
#endif
			head.clear();
			mal = 1;
		}

	private:

		void _addedge(const node_type &u, const node_type &v) {
			auto &edge = e[mal]; auto &hu = head[u];
			edge.to = v;
			edge.nx = hu; hu = mal++;
		}
	};


	template<typename node_type, typename weighter, int64_t VSize, int64_t ESize,
		bool undirected = false, typename id_type = uint32_t, typename edge_t = WeightedEdge<id_type, node_type, weighter> >
	struct WeightedGraph: public Graph<node_type, VSize, ESize, undirected, id_type, edge_t> {
		using f_class = Graph<node_type, VSize, ESize, undirected, id_type, edge_t>;
		WeightedGraph() : f_class() {}
		WeightedGraph(int n) : f_class(n) {}

		void addedge(const node_type &u, const node_type &v, const weighter &w) {
			if CXX17_STATIC_CONDITION (undirected) {
				_addedge(u, v, w);
				_addedge(v, u, w);
			} else {
				_addedge(u, v, w);
			}
		}

		void addedge(const node_type &u, const node_type &v) {
			f_class::addedge(u, v);
		}

	private:
		void _addedge(const node_type &u, const node_type &v, const weighter &w) {
			auto &edge = f_class::e[f_class::mal]; auto &hu = f_class::head[u];
			edge.to = v;
			edge.w = w;
			edge.nx = hu; hu = f_class::mal++;
		}
	};
}

template<typename I>
struct __slice {
	I l, r;

	__slice(I l, I r) : l(l), r(r) {}

	I begin() {
		return l;
	}
	I end() {
		return r;
	}
};

template<typename I>
struct __combine {
	__slice<I> ls, rs;

	__combine(const __slice<I> &ls, const __slice<I> &rs):ls(ls), rs(rs) {}

	struct iterator {
		__combine<I> &c;
		I cur; bool ls;
		iterator(__combine<I> &c, I cur, bool ls): c(c), cur(cur), ls(ls) {}
		iterator &operator++() {
			cur++;
			if (ls == false && cur == std::end(c.ls)) {
				cur = std::begin(c.rs);
				ls = true;
			}
			return *this;
		}
		iterator &operator++(int) {
			return operator++();
		}
		bool operator!=(const iterator &i) const {
			return ls != i.ls || cur != i.cur;
		}
		bool operator==(const iterator &i) const {
			return ls == i.ls && cur == i.cur;
		}
		auto operator*() -> decltype(*cur) {
			return *cur;
		}
	};

	iterator begin() {
		return iterator(*this, ls.begin(), false);
	}

	iterator end() {
		return iterator(*this, rs.end(), true);
	}
};

template<typename Iterator>
__slice<Iterator> slice(Iterator _begin, Iterator _end) {
	return __slice<Iterator>(_begin, _end);
}

template<typename Container>
auto slice(const Container &c, int l=-1, int r=-1) -> __slice<decltype(std::begin(c))> {
	auto x = std::begin(c), y = std::end(c);
	if (l != -1) {
		if(r!=-1) r -= l;
		while (l) {
			x++; l--;
		}
	}
	if (r != -1) {
		y = x;
		while (r) {
			y++; r--;
		}
	}
	return __slice<decltype(std::begin(c))>(x, y);
}

template<typename Iterator>
__combine<Iterator> combine(const __slice<Iterator> &_left, const __slice<Iterator> &_right) {
	return __combine<Iterator>(_left, _right);
}



namespace parse {
template<class grammar_traits>
class LR1Grammar {
public:
	using model_t = typename grammar_traits::model_t;
	using string = typename grammar_traits::string;
	using strvec = typename grammar_traits::strvec;
	using symbol_t = typename grammar_traits::symbol_t;
	using production_t = typename grammar_traits::production_t;

	using grammar_t = LR1Grammar<grammar_traits>;
private:
	std::map<string, symbol_t> &sym_table;
	std::vector<production_t> &prods;
	symbol_t begin_symbol;

	std::map<symbol_t, std::set<symbol_t>* > first;
	std::map<symbol_t, uint8_t> epsable;

	using action_map = std::map<symbol_t, action_space::action*>;
	std::map<symbol_t, action_map*> table;
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

		// Policy::table = &table;
		// Policy::begin_symbol = begin_symbol;
		// Policy::follow = follow;
	}

	using item_t = std::pair<std::pair<int, int>, symbol_t>;
	std::vector<item_t> items;

	using state_id = int32_t;
	using hashed_item_t = int64_t;
	using state_set = std::set<item_t>;
	std::vector<state_set*> state;
	std::map<hashed_item_t, state_id> hash_set;
	graph::WeightedGraph<state_id, symbol_t, 500, 500 * 10> automa;
	int64_t seed, seed2;
	const int64_t mod = 1e9 + 9;
	
	state_set &alloc(state_id &idx) {
		state.push_back(new state_set());
		idx = state.size() - 1;
		return *state.back();
	}

	void build() {
		automa.init(499);
		seed = std::chrono::system_clock::now().time_since_epoch().count() % mod;
		state_id idx; state_set &mset = alloc(idx);
		bool proceed = false;
		for (int i = prods.size() - 1; i >= 0;i--) {
			auto &prod = prods[i];
			if (prod.lhs == begin_symbol) {
				if (proceed) {
					throw std::invalid_argument("must with only one production reduce to begin symbol");
				}
				proceed = true;
				auto base_item = std::make_pair(i, 0);
				std::set<symbol_t> fset;
				get_first1<grammar_t, grammar_traits, decltype(prod.rhs)>(*this, prod.rhs, fset);
				for (auto &sym: fset) {
					mset.insert(std::make_pair(base_item, sym));
				}
			}
		}

		if (mset.size() == 0) {
			throw std::invalid_argument("must with only one production reduce to begin symbol");
		}
		print::print(prods[mset.begin()->first.first], true);
		walk(extend(idx));
		std::cout << state.size() << std::endl;
		for (int i = 0; i < state.size(); i++) {
			for (auto j : automa.at_e(i)) {
				std::cout << "(" << i << ", " << j.to << ", " << j.w << ")" << std::endl;
			}
		}
		for (auto &st : state) {
			print::print(*st, true);
			delete st;
		}
	}
	
	void walk(state_id idx) {
		if (idx < 0) {
			return;
		}
		std::cout << "walking " << idx; print::print(*state[idx], true);
		std::set<symbol_t> sym_set;
		for (auto &item : *state[idx]) {
			if (prods[item.first.first].rhs.size() == item.first.second) {
				continue;
			}
			sym_set.insert(prods[item.first.first].rhs[item.first.second]);
		}
		for (auto &sym: sym_set) {
			state_id next_state; state_set &mset = alloc(next_state);
			for (auto &item : *state[idx]) {
				if (prods[item.first.first].rhs.size() == item.first.second) {
					continue;
				}
				auto &sym0 = prods[item.first.first].rhs[item.first.second];
				if (sym0 == sym) {
					mset.insert(item_t{{item.first.first, item.first.second + 1}, item.second});
				}
			}
			std::cout << "walking " << idx; print::print(sym); print::print(*state[idx]); print::print("->"); print::print(mset, true);
			auto true_next_state = extend(next_state);
			automa.addedge(idx, true_next_state, sym);
			if (true_next_state == next_state) walk(true_next_state);
		}
	}
	
	state_id extend(state_id idx) {
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

	void extend_to(state_set &mset, std::set<symbol_t> lookahead, const symbol_t &sym) {
		for (int i = prods.size() - 1; i >= 0; i--) {
			auto &prod = prods[i];
			if (prod.lhs == sym) {
				auto item = std::make_pair(i, 0);
				for (auto &asym : lookahead) {
					auto ritem = std::make_pair(item, asym);
					if (!mset.count(ritem)) {
						mset.insert(ritem);
						auto &sym = prod.rhs[0];
						if (sym.is_unterm()) {
							extend_to(mset, lookahead, sym);
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
			result = (seed * result + item.first.first * _233_2 + item.first.second * 2333333LL + int64_t(item.second.data.uterm)) % mod;
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
/*

	using item_t = std::pair<int, int>;
	std::vector<item_t> items;

	using state_id = int32_t;
	using hashed_item_t = int64_t;
	using state_set = std::set<item_t>;
	std::vector<state_set*> state;
	std::map<hashed_item_t, state_id> hash_set;
	graph::WeightedGraph<state_id, symbol_t, 500, 500 * 10> automa;
	int64_t seed, seed2;
	const int64_t mod = 1e9 + 9;

	state_set &alloc(state_id &idx) {
		state.push_back(new state_set());
		idx = state.size() - 1;
		return *state.back();
	}

	void build() {
		automa.init(499);
		seed = std::chrono::system_clock::now().time_since_epoch().count() % mod;
		state_id idx; state_set &mset = alloc(idx);

		for (int i = prods.size() - 1; i >= 0;i--) {
			auto &prod = prods[i];
			if (prod.lhs == begin_symbol) {
				mset.insert(item_t{i, 0});
			}
		}
		if (mset.size() != 1) {
			throw std::invalid_argument("must with only one production reduce to begin symbol");
		}

		print::print(prods[mset.begin()->first], true);
		walk(extend(idx));
		std::cout << state.size() << std::endl;
		for (int i = 0; i < state.size(); i++) {
			for (auto j : automa.at_e(i)) {
				std::cout << "(" << i << "," << j.to << ", " << j.w << ")" << std::endl;
			}
		}
		for (auto &st : state) {
			print::print(*st, true);
			delete st;
		}
	}

	void walk(state_id idx) {
		if (idx < 0) {
			return;
		}
		std::cout << "walking " << idx; print::print(*state[idx], true);
		std::set<symbol_t> sym_set;
		for (auto &item : *state[idx]) {
			if (prods[item.first].rhs.size() == item.second) {
				continue;
			}
			sym_set.insert(prods[item.first].rhs[item.second]);
		}
		for (auto &sym: sym_set) {
			state_id next_state; state_set &mset = alloc(next_state);
			for (auto &item : *state[idx]) {
				if (prods[item.first].rhs.size() == item.second) {
					continue;
				}
				auto &sym0 = prods[item.first].rhs[item.second];
				if (sym0 == sym) {
					mset.insert(item_t{item.first, item.second + 1});
				}
			}
			std::cout << "walking " << idx; print::print(sym); print::print(*state[idx]); print::print("->"); print::print(mset, true);
			auto true_next_state = extend(next_state);
			automa.addedge(idx, true_next_state, sym);
			if (true_next_state == next_state) walk(true_next_state);
		}
	}

	state_id extend(state_id idx) {
		auto &mset = state[idx];
		for (auto &item : *mset) {
			if (prods[item.first].rhs.size() == item.second) {
				continue;
			}
			auto &sym = prods[item.first].rhs[item.second];
			if (sym.is_unterm()) {
				extend_to(*mset, sym);
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

	void extend_to(state_set &mset, const symbol_t &sym) {
		for (int i = prods.size() - 1; i >= 0; i--) {
			auto &prod = prods[i];
			if (prod.lhs == sym) {
				auto item = item_t{i, 0};
				if (!mset.count(item)) {
					mset.insert(item);
					auto &sym = prod.rhs[0];
					if (sym.is_unterm()) {
						extend_to(mset, sym);
					}
				}
			}
		}
	}

	hashed_item_t calculate_hash(state_set &mset) {
		hashed_item_t result = 0;
		for (auto &item : mset) {
			result = (seed * result + item.first * 233333333LL + item.second) % mod;
		}
		return result;
	}

*/
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

	f.close();

	delete model;

}

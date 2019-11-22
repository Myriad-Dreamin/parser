#pragma once

#include "cxx17.h"
#include <iostream>

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

		WeightedEdge():Edge<id_type, node_type>() {};
		WeightedEdge(const id_type &nx, const node_type &to, const weighter &w): Edge<id_type, node_type>(nx, to), w(w) {};
	};

	template<typename id_type, typename node_type, typename weighter>
	std::ostream &operator<< (std::ostream &os, WeightedEdge<id_type, node_type, weighter> &e) {
		os << "(" << e.nx << ", " << e.to << ", " << e.w << ")";
		return os;
	}

	template<typename node_type, int64_t VSize, int64_t ESize,
		bool undirected = false, typename id_type = uint32_t, typename edge_t = Edge<id_type, node_type> >
		struct Graph {
		using graph_t = Graph<node_type, VSize, ESize, undirected, id_type, edge_t>;
		static constexpr int64_t esize_v = undirected ? (ESize << 1) : ESize;
		edge_t e[esize_v];
		std::map<node_type, id_type> head;
		id_type mal;
		Graph() {
			init();
		}
		Graph(int n) {
			init(n + 1);
		}

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
			partial_iterator(const graph_t &g, const node_type &u):view(g), u(u) {};

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
			//#ifdef DEBUG
			//			assert(("n < VSize", n < VSize));
			//#endif
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
		WeightedGraph(): f_class() {}
		WeightedGraph(int n): f_class(n) {}

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

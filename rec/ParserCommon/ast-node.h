#pragma once

#include <iostream>

namespace parse {
	template<typename term_t, typename uterm_t>
	struct ASTNode {
		using symbol_t = Symbol<term_t, uterm_t>;
		symbol_t symbol;
		ASTNode(term_t x):symbol(x) {}
		ASTNode(uterm_t x, bool y):symbol(x, y) {}
		ASTNode(const symbol_t &r): symbol(r) {}
		ASTNode *&insert(ASTNode * chx) {
			ch.push_back(chx);
			return ch.back();
		}
		std::vector<ASTNode*> ch;
		template<typename u, typename v>
		friend std::ostream &operator<<(std::ostream & os, ASTNode<u, v> &res);
	};


	template<typename term_t, typename uterm_t>
	std::ostream &operator<<(std::ostream & os, ASTNode<term_t, uterm_t> &res) {
		os << "{" << (Symbol<term_t, uterm_t>&)(res) << ", {";
		for (auto &chx : res.ch) {
			os << *chx << ", ";
		}
		os << "}}";

		return os;
	}

}

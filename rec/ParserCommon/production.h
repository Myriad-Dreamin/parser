#pragma once

#include <iostream>

namespace parse {
	template<typename token_t>
	struct Production {
		token_t lhs;
		std::vector<token_t> rhs;

		Production(token_t & lhs, std::vector<token_t> & rhs): lhs(lhs), rhs(rhs) {}
		Production(token_t & lhs, std::vector<token_t> &&rhs): lhs(lhs), rhs(rhs) {}
		Production(token_t &&lhs, std::vector<token_t> & rhs): lhs(lhs), rhs(rhs) {}
		Production(token_t &&lhs, std::vector<token_t> &&rhs): lhs(lhs), rhs(rhs) {}
		Production(const Production &rp) {
			lhs = rp.lhs;
			rhs.clear(); rhs.assign(rp.rhs.begin(), rp.rhs.end());
		}
		Production(Production &&rp) {
			lhs = std::move(rp.lhs);
			rhs.swap(rp.rhs);
		}
	};

	template<typename token_t>
	std::ostream &operator<<(std::ostream &os, Production<token_t> &x) {
		os << x.lhs << " -> ";
		print::print(x.rhs);
		return os;
	}
}


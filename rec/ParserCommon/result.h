#pragma once

#include <iostream>
#include "ast-node.h"


namespace parse {
	enum class ResultCode: uint16_t {
		Ok,
		Error,
	};


	std::ostream &operator<<(std::ostream &os, ResultCode x) {
		switch (x) {
			case ResultCode::Ok:
				os << "Ok";
				break;
			case ResultCode::Error:
				os << "Error";
				break;
			default:
				os << "ResultCode<NotFound>";
				break;
		}
		return os;
	}

	// template<typename term_t, class Source, class TokenTable>
	// class Parser;

	template<typename term_t, typename uterm_t>
	struct Result {
		using node_t = ASTNode<term_t, uterm_t>;
		node_t *rt;
		ResultCode code;
		virtual ~Result() {
			release();
		}
		void release() {
			for (auto &node : nodes) {
				delete node;
			}
			nodes.clear();
			rt = nullptr;
		}

		template<typename T>
		node_t *alloc(T x) {
			node_t *n = new node_t(x);
			nodes.push_back(n);
			return n;
		}

		template<typename T>
		node_t *alloc(T x, bool y) {
			node_t *n = new node_t(x, y);
			nodes.push_back(n);
			return n;
		}


	protected:
		std::vector<node_t*> nodes;

		template<typename u, typename v>
		friend std::ostream &operator<<(std::ostream & os, Result<u, v> &res);
	};

	template<typename term_t, typename uterm_t>
	std::ostream &operator<<(std::ostream & os, Result<term_t, uterm_t> &res) {
		os << "{ code: " << res.code << ", node: ";
		os << *res.rt;
		os << "}";
		return os;
	}
}


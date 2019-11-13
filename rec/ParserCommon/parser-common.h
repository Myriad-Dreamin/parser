#pragma once

#include <iostream>
#include <type_traits>

namespace parse {


template<typename term_t, typename uterm_t>
struct Symbol {
	static const bool uterm_v = true;
	static constexpr bool term_v = !uterm_v;
	static const uterm_t undecided = static_cast<uterm_t>(-1);

	union Data {
		term_t term;
		uterm_t uterm;
	} data;
	bool ut;
	
	Symbol() {
		data.uterm = undecided;
		ut = uterm_v;
	}

	Symbol(term_t x) {
		data.term = x;
		ut = term_v;
	}
	template<typename T>
	Symbol(T x, bool utrm) {
		if (utrm) {
			data.uterm = static_cast<uterm_t>(x);
		} else {
			data.term = static_cast<term_t>(x);
		}
		ut = utrm;
	}

	template<typename u, typename v>
	friend std::ostream &operator<<(std::ostream & os, Symbol<u, v> &res);
};

template<typename term_t, typename uterm_t>
struct Node: public Symbol<term_t, uterm_t> {
	using symbol = Symbol<term_t, uterm_t>;
	Node(term_t x) :symbol(x) {}
	Node(uterm_t x, bool y) :symbol(x, y) {}
	
	Node *&insert(Node * chx) {
		ch.push_back(chx);
		return ch.back();
	}
	std::vector<Node*> ch;
	template<typename u, typename v>
	friend std::ostream &operator<<(std::ostream & os, Node<u, v> &res);
};

template<typename term_t, typename uterm_t>
std::ostream &operator<<(std::ostream & os, Symbol<term_t, uterm_t> &res) {
	if (res.ut) {
		os << "{ut," << res.data.uterm << "}";
	} else {
		os << "{t," << res.data.term << "}";
	}
	return os;
}

template<typename term_t, typename uterm_t>
std::ostream &operator<<(std::ostream & os, Node<term_t, uterm_t> &res) {
	os << "{" << (Symbol<term_t, uterm_t>&)(res) << ", {";
	for (auto &chx : res.ch) {
		os << *chx << ", ";
	}
	os << "}}";

	return os;
}

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
	template<typename parser_term_t, class Source, class TokenTable>
	friend class Parser;
	using node_t = Node<term_t, uterm_t>;
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
protected:
	std::vector<node_t*> nodes;
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


enum class UTerm: uint16_t {
	E,
	ED,
	T,
	TD,
	F,
};


std::ostream &operator<<(std::ostream &os, UTerm x) {
	switch (x) {
		case UTerm::E:
			os << "E";
			break;
		case UTerm::ED:
			os << "ED";
			break;
		case UTerm::T:
			os << "T";
			break;
		case UTerm::TD:
			os << "TD";
			break;
		case UTerm::F:
			os << "F";
			break;
		default:
			os << "UTerm<NotFound>";
			break;
	}
	return os;
}

}


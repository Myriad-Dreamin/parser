#pragma once

#include <iostream>


template<typename T>
const char *stringify(T v);

template<typename token_t, typename uterm_t>
struct Node {
	union Data {
		token_t token;
		uterm_t uterm;
	} data;
	bool ut;

	Node(token_t x) {
		data.token = x;
		ut = false;
	}
	Node(uterm_t x, bool) {
		data.uterm = x;
		ut = true;
	}

	Node *&insert(Node * chx) {
		ch.push_back(chx);
		return ch.back();
	}

	std::vector<Node*> ch;
	template<typename u, typename v>
	friend std::ostream &operator<<(std::ostream & os, Node<u, v> &res);
};

template<typename token_t, typename uterm_t>
std::ostream &operator<<(std::ostream & os, Node<token_t, uterm_t> &res) {
	if (res.ut) {
		os << "{" << stringify(res.data.uterm) << ", {";
	} else {
		os << "{" << stringify(res.data.token) << ", {";
	}
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

template<>
const char *stringify(ResultCode x) {
	switch (x) {
		case ResultCode::Ok:
			return "Ok";
		case ResultCode::Error:
			return "Error";
		default:
			return "ResultCode<NotFound>";
	}
}

// template<typename token_t, class Source, class TokenTable>
// class Parser;

template<typename token_t, typename uterm_t>
struct Result {
	template<typename parser_token_t, class Source, class TokenTable>
	friend class Parser;
	using Node = Node<token_t, uterm_t>;
	Node *rt;
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
	std::vector<Node*> nodes;
	template<typename T>
	Node *alloc(T x) {
		Node *n = new Node(x);
		nodes.push_back(n);
		return n;
	}
	template<typename T>
	Node *alloc(T x, bool y) {
		Node *n = new Node(x, y);
		nodes.push_back(n);
		return n;
	}

	template<typename u, typename v>
	friend std::ostream &operator<<(std::ostream & os, Result<u, v> &res);
};

template<typename token_t, typename uterm_t>
std::ostream &operator<<(std::ostream & os, Result<token_t, uterm_t> &res) {
	os << "{ code: " << stringify(res.code) << ", node: ";
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

template<>
const char *stringify(UTerm x) {
	switch (x) {
		case UTerm::E:
			return "E";
		case UTerm::ED:
			return "ED";
		case UTerm::T:
			return "T";
		case UTerm::TD:
			return "TD";
		case UTerm::F:
			return "F";
		default:
			return "UTerm<NotFound>";
	}
}

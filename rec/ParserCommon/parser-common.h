#pragma once

#include <iostream>
#include "print.h"
#include "string-algorithms.h"
#include "functional.h"

#include <type_traits>

#if __cplusplus >= 201703L
#define CXX17_STATIC_CONDITION constexpr
#else
#define CXX17_STATIC_CONDITION
#endif

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

	bool is_unterm() const {
		return ut;
	}

	template<bool c>
	term_t get_sym() {
		return data.term;
	}

	bool operator < (const Symbol<term_t, uterm_t> &y) const {
		if (ut != y.ut) {
			return ut < y.ut;
		}
		if (ut == term_v) {
			return static_cast<uint64_t>(data.term) < static_cast<uint64_t>(y.data.term);
		}
		return static_cast<uint64_t>(data.uterm) < static_cast<uint64_t>(y.data.uterm);
	}

	bool operator== (const Symbol<term_t, uterm_t> &y) const {
		return ut == y.ut && (
		( ut ==  term_v && static_cast<uint64_t>(data. term) == static_cast<uint64_t>(y.data. term) ) ||
		( ut == uterm_v && static_cast<uint64_t>(data.uterm) == static_cast<uint64_t>(y.data.uterm) ));
	}

	bool operator!= (const Symbol<term_t, uterm_t> &y) const {
		return !operator==(y);
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


template<typename token_t>
struct Production {
	token_t lhs;
	std::vector<token_t> rhs;

	Production(token_t & lhs, std::vector<token_t> & rhs) : lhs(lhs), rhs(rhs) {}
	Production(token_t & lhs, std::vector<token_t> &&rhs) : lhs(lhs), rhs(rhs) {}
	Production(token_t &&lhs, std::vector<token_t> & rhs) : lhs(lhs), rhs(rhs) {}
	Production(token_t &&lhs, std::vector<token_t> &&rhs) : lhs(lhs), rhs(rhs) {}
	Production(const Production &rp) {
		lhs = rp.lhs;
		rhs.clear();rhs.assign(rp.rhs.begin(), rp.rhs.end());
	}
	Production(Production &&rp) {
		lhs = std::move(rp.lhs);
		rhs.swap(rp.rhs);
	}
};

template<typename token_t>
std::ostream &operator<<(std::ostream &os, Production<token_t> &x) {
	os << x.lhs << " -> ";
	print::print<decltype(x.rhs)>(x.rhs);
	return os;
}


template<typename term_t=int32_t, typename uterm_t=int32_t>
class Model {
	friend class Processor;
	public:
	using string = std::string;
	using strvec = std::vector<string>;
	using symbol = Symbol<term_t, uterm_t>;
	using model = Model<term_t, uterm_t>;
	std::map<string, symbol> sym_table;
	std::vector<Production<symbol>> prods;
	Model() {}
	bool is_error() { return err.length() != 0; }
	const string &error() { return err; }
	private:
	string err;
	model *error(const string &info) {
		err = info;
		return this;
	}

	template<bool ut>
	void apply_sym_def(string &s) {
		strvec slice;
		functional::map(salg::split(s, '=', slice), salg::trim_space<char>);
		if (slice.size() != 2) {
			throw std::invalid_argument("must in form of A = B, not " + s);
		}
		sym_table[slice[0]] = symbol(
			static_cast<typename std::conditional<ut, uterm_t, term_t>::type>(
				std::stoi(slice[1])), ut);
	}

	void sym_from_string_true(string &s) {
		strvec slice;
		functional::map(salg::split(s.substr(5), ',', slice), salg::trim_space<char>);
		for (auto &x:slice)
			apply_sym_def<true>(x);
	}

	void sym_from_string_false(string &s) {
		strvec slice;
		functional::map(salg::split(s.substr(5), ',', slice), salg::trim_space<char>);
		for (auto &x:slice)
			apply_sym_def<false>(x);
	}

	void prod_from_string(string &s) {
		strvec slice;
		if (salg::split(s, "->", slice).size() != 2) {
			throw std::invalid_argument("must in form of A -> B, not " + s);
		}
		auto &lhs = slice[0], &rhss = slice[1];
		
		strvec slice2;
		for (auto &rhs:functional::map(salg::split(rhss, '|', slice2), salg::trim_space<char>))
			prod_from_pair(lhs, rhs);
		
	}

	void prod_from_pair(string &raw_lhs, const string &raw_rhs) {
		symbol lhs = sym_table.at(salg::trim_space<char>(raw_lhs));
		strvec slice;
		std::vector<symbol> rhs;
		//todo
		prods.emplace_back(std::move(Production<symbol>(lhs, functional::map_st(
			functional::map(salg::split_space<char>(raw_rhs, slice), salg::trim_space<char>),
			rhs, [&](const string &r) {return sym_table.at(r);}
		))));
	}
	
	void prods_from_string(string &s) {
		strvec slice;
		functional::map(salg::split(s, '\n', slice), salg::trim_space<char>);
		functional::map_void(slice, [&](string &s){
			if (s.empty()) return;
			this->prod_from_string(s);
		});
	}

	template<typename u, typename v>
	friend Model<u, v> *G(std::istream &in);
};

template<typename term_t=int32_t, typename uterm_t=int32_t>
Model<term_t, uterm_t> *G(std::istream &in) {
	using model_t = Model<term_t, uterm_t>;
	std::string line;
	std::vector<std::string> atos;
	auto model = new Model<term_t, uterm_t>();
	while(salg::get_till<char>(in, line)) {
		if (line.empty()) continue;

		if (line[0] == '#') {
			continue;
		} else if (line[0] == 'P') {
			auto pos = line.find('{');
			if (pos == std::string::npos) {
				return model->error("invalid statement: miss {");
			}
			salg::get_till_nc(in, line = line.substr(pos+1), '}');
			model->prods_from_string(line);
		} else if (line.length() > 5) {
			if (!strncmp(line.c_str(), "uter ", 5)) {
				if CXX17_STATIC_CONDITION (model_t::symbol::uterm_v) {
					model->sym_from_string_true(line);
				} else {
					model->sym_from_string_false(line);
				}
			} else if (!strncmp(line.c_str(), "term ", 5)) {
				if CXX17_STATIC_CONDITION (model_t::symbol::term_v) {
					model->sym_from_string_true(line);
				} else {
					model->sym_from_string_false(line);
				}
			} else {
				return model->error("invalid statement: either uter or term at beginning");
			}
		} else {
			return model->error("invalid statement");
		}
	}
	print::print<decltype(model->sym_table)>(model->sym_table, true);
	print::print<decltype(model->prods)>(model->prods, true);
	return model;
}


template<typename term_t=int32_t, typename uterm_t=int32_t>
Model<term_t, uterm_t> *G(const char *file_name){
	auto f = std::fstream(file_name, std::ios::in);
	return G<term_t, uterm_t>(f);
}	


template<class Grammar, class grammar_traits>
void calculate_first_fixed_point(Grammar &g) {

	using symbol = typename Grammar::symbol;
	for (auto &x: g.sym_table) {
		auto &sym = x.second;
		auto s = new std::set<symbol>();
		if (!sym.is_unterm()) {
			s->insert(sym);
		}
		g.first[sym] = s;
	}
	
	bool changed;
	do {
		changed  = false;
		for (auto &x: g.first) {
			auto &sym = x.first;
			auto &set = *x.second;
			size_t ls = set.size();
			// print::print("testing ");
			// print::print(&set); print::print(" ");
			// print::print(x.second); print::print(" "); print::print(sym); print::print(" ");
			// print::print(set, true);
			// lower_bound or sort() && iter will be better
			for (auto &prod: g.prods) {
				if (prod.lhs == sym) {
					// print::print("using ");
					// print::print(prod, true);
						
					for (auto &rsym: prod.rhs) {
						auto &rset = *g.first[rsym];
						// print::print("merge ");
						// print::print(set);
						// print::print(" ");
						// print::print(rset, true);

						set.insert(rset.begin(), rset.end());
						if (!rset.count(grammar_traits::epsilon)) {
							break;
						}
					}
				}
			}
			if (set.size() != ls) {
				changed = true;
			}
		}
	} while(changed);
}

}


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
		// std::cout << "construct" << std::endl;
	}

	Symbol(term_t x) {
		data.term = x;
		ut = term_v;
		// std::cout << "construct" << std::endl;
	}

	template<typename T>
	Symbol(T x, bool utrm) {
		if (utrm) {
			data.uterm = static_cast<uterm_t>(x);
		} else {
			data.term = static_cast<term_t>(x);
		}
		ut = utrm;
		// std::cout << "construct" << std::endl;
	}

	~Symbol() {
		// std::cout << "deconstruct" << std::endl;
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

	template<typename u, typename v>
	friend std::ostream &operator<<(std::ostream & os, Symbol<u, v> &res);
};


template<typename term_t, typename uterm_t>
struct Node: public Symbol<term_t, uterm_t> {
	using symbol_t = Symbol<term_t, uterm_t>;
	Node(term_t x) :symbol_t(x) {}
	Node(uterm_t x, bool y) :symbol_t(x, y) {}
	
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
	S,
	E,
	ED,
	T,
	TD,
	F,
};


std::ostream &operator<<(std::ostream &os, UTerm x) {
	switch (x) {
		case UTerm::S:
			os << "S";
			break;
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
	print::print(x.rhs);
	return os;
}


template<typename term_t=int32_t, typename uterm_t=int32_t>
class Model {
	friend class Processor;
	public:
	using string = std::string;
	using strvec = std::vector<string>;
	using symbol_t = Symbol<term_t, uterm_t>;
	using model = Model<term_t, uterm_t>;
	std::map<string, symbol_t> sym_table;
	std::vector<Production<symbol_t>> prods;
	symbol_t begin_symbol;
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
		sym_table[slice[0]] = symbol_t(
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
		symbol_t lhs = sym_table.at(salg::trim_space<char>(raw_lhs));
		strvec slice;
		std::vector<symbol_t> rhs;
		//todo
		prods.emplace_back(std::move(Production<symbol_t>(lhs, functional::map_st(
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
	try {
		while(salg::get_till<char>(in, line)) {
			if (salg::trim_space<char>(line).empty()) continue;
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
					if CXX17_STATIC_CONDITION (model_t::symbol_t::uterm_v) {
						model->sym_from_string_true(line);
					} else {
						model->sym_from_string_false(line);
					}
				} else if (!strncmp(line.c_str(), "term ", 5)) {
					if CXX17_STATIC_CONDITION (model_t::symbol_t::term_v) {
						model->sym_from_string_true(line);
					} else {
						model->sym_from_string_false(line);
					}
				} else if (!strncmp(line.c_str(), "begi ", 5)) {
					model->begin_symbol = model->sym_table.at(salg::trim_space<char>(line = line.substr(5)));
				} else {
					return model->error("invalid statement: either uter or term at beginning");
				}
			} else {
				return model->error("invalid statement");
			}
			line.clear();
		}
	} catch(std::exception &e) {
		return model->error(e.what());
	}
	print::print(model->sym_table, true);
	print::print(model->prods, true);
	std::cout << "? ";
	print::print(model->begin_symbol, true);
	return model;
}


template<typename term_t=int32_t, typename uterm_t=int32_t>
Model<term_t, uterm_t> *G(const char *file_name){
	auto f = std::fstream(file_name, std::ios::in);
	return G<term_t, uterm_t>(f);
}	


struct __M_TRAITS {
	static const int32_t epsilon = -1;
	static const int32_t dollar = -2;
};

template<typename __term_t=int32_t, typename __uterm_t=int32_t, class m_traits=__M_TRAITS>
struct basic_grammar_traits {
	using term_t = __term_t;
	using uterm_t = __uterm_t;
	using model_t = Model<term_t, uterm_t>;
	using string = typename model_t::string;
	using strvec = typename model_t::strvec;
	using symbol_t = typename model_t::symbol_t;
	using production_t = Production<symbol_t>;
	static constexpr auto epsilon = m_traits::epsilon;
	static constexpr auto dollar = m_traits::dollar;
};


template<class Grammar, class grammar_traits>
void calculate_first_fixed_point(Grammar &g) {

	using symbol_t = typename Grammar::symbol_t;
	for (auto &x: g.sym_table) {
		auto &sym = x.second;
		auto s = new std::set<symbol_t>();
		if (!sym.is_unterm()) {
			s->insert(sym);
		}
		g.first[sym] = s;
	}
	
	bool changed;
	do {
		changed  = false;
		for (auto &symset: g.first) {
			if (!symset.first.is_unterm()) {
				continue;
			}
			auto &sym = symset.first;
			auto set = symset.second;
			size_t ls = set->size();
			// print::print("testing ");
			// print::print(set); print::print(" ");
			// print::print(symset.second); print::print(" "); print::print(sym); print::print(" ");
			// print::print(*set, true);
			// lower_bound or sort() && iter will be better
			for (auto &prod: g.prods) {
				if (prod.lhs == sym) {
					// print::print("using ");
					// print::print(prod, true);
						
					for (auto &rsym: prod.rhs) {
						auto &rset = *g.first[rsym];
						// print::print("merge ");
						// print::print(*set);
						// print::print(" ");
						// print::print(rset, true);

						set->insert(rset.begin(), rset.end());
						if (!rset.count(grammar_traits::epsilon)) {
							break;
						}
					}
				}
			}
			if (set->size() != ls) {
				changed = true;
			}
		}
	} while(changed);
	// for (auto &c : g.first) {
	// 	print::print(c.first);
	// 	print::print(' ');
	// 	print::print(c.second);
	// 	print::print(' ');
	// 	print::print(*c.second, true);
	// }
}

namespace epsilonable {
	enum ExploreState{
		No,
		Yes,
		Unknown,
	};
}

template<class Grammar, class grammar_traits>
void calculate_epsilonable(Grammar &g) {
	using namespace epsilonable;
	using production_t = typename Grammar::production_t;
	std::vector<production_t*> conts;
	
	for(auto &ss: g.sym_table) {
		g.epsable[ss.second] = ss.second.is_unterm() ? ExploreState::Unknown: ExploreState::No;
	}
	g.epsable[grammar_traits::epsilon] = ExploreState::Yes;
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
}

template<class Grammar, class grammar_traits>
void calculate_follow_fixed_point(Grammar &g) {
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
					g.follow[rsym]->insert(mset.begin(), mset.end());
					if (g.follow[rsym]->size() - sz) {
						changed = true;
					}
				}
				if (!g.epsable[rsym]) {
					mset.clear();
				}
				mset.insert(g.first[rsym]->begin(), g.first[rsym]->end());
				mset.erase(grammar_traits::epsilon);
			}
		}
	} while(changed);
}


}


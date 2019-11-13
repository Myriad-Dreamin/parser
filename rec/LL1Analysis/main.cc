
#include "pch.h"
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <cstring>
#include <fstream>
#include <type_traits>
#include <regex>
#ifdef VS_COMPILE
#include <lexer-result.h>
#include <parser-common.h>
#else
#include "../LexerMock/lexer-result.h"
#include "../ParserCommon/parser-common.h"
#endif // VS_COMPILE


namespace functional
{
	template<typename T, typename F>
	T &map(T &vec, const F &f) {
		for (auto &element: vec) {
			element = f(element);
		}
		return vec;
	}

	template<typename S, typename T, typename F>
	T &map_st(S &svec, T &tvec, const F &f) {
		tvec.resize(svec.size());
		for (size_t i = 0; i < svec.size(); i++) {
			tvec[i] = f(svec[i]);
		}
		return tvec;
	}

	template<typename T, typename F>
	T &map_void(T &vec, const F &f) {
		for (auto &element: vec) {
			f(element);
		}
		return vec;
	}
} // namespace functional



namespace salg {


	template<typename stream_t>
	struct traits {};

	template<>
	struct traits<char> {
		using string = std::basic_string<char>;
		using istream = std::basic_istream<char>;
		static const char line_end = '\n';
		static const char eof = EOF;
		static const char stp = '\0';
	};

	template<typename stream_t>
	std::vector<typename traits<stream_t>::string> &split(
		const typename traits<stream_t>::string &s,
		const stream_t sep,
		std::vector<typename traits<stream_t>::string> &res
	) {
		res.clear();
		// traits<stream_t>::string
		size_t ls = 0;
		for (int x = 0; x < s.length() ;x++) {
			if (s[x] == sep) {
				res.push_back(s.substr(ls, x-ls));
				ls = x + 1;
			}
		}
		res.push_back(s.substr(ls));
		return res;
	}

	#if __cplusplus >= 201703L 
	template<typename stream_t>
	std::vector<std::string> &split(
		const std::string_view &s,
		const std::string_view &sep,
		std::vector<std::string> &res
	) {
	#else
	std::vector<std::string> &split(
		const std::string &s,
		const std::string &seps,
		std::vector<std::string> &res
	) {
	#endif
		
		res.clear();
		size_t ls = 0, sl = sep.length();
		if (sl < 5) {
			size_t pos = s.find(sep, 0);
			
			while(pos != std::string::npos)
			{
				res.push_back(std::string(s.substr(ls, pos-ls)));
				ls = pos + sl;
				pos = s.find(sep,ls+1);
			}
			res.push_back(std::string(s.substr(ls)));
		} else {
		}
		return res;
	}

	std::vector<std::string> &split(
		const std::string &s,
		const char *sep,
		std::vector<std::string> &res
	) {
		#if __cplusplus >= 201703L
		std::string_view sw(sep);
		return split<char>(s, sw, res);
		#else
		return split(s, sep, res);
		#endif
	}

	template<typename stream_t>
	bool get_till_nc(
		typename traits<stream_t>::istream &in,
		typename traits<stream_t>::string &res,
		const stream_t line_end=traits<stream_t>::line_end) {
		for (stream_t s;;) {
			in.get(s);
			if (s == traits<stream_t>::stp || s == line_end) {
				return res.size() != 0;
			}
			res.push_back(s);
		}
	}

	template<typename stream_t>
	bool get_till(
		typename traits<stream_t>::istream &in,
		typename traits<stream_t>::string &res,
		const stream_t line_end=traits<stream_t>::line_end) {
		res.clear();
		return get_till_nc(in, res, line_end);
	}

	template<typename stream_t>
	bool till(
		typename traits<stream_t>::istream &in,
		const stream_t line_end=traits<stream_t>::line_end) {
		typename traits<stream_t>::string nil;
		return get_till(in, nil, line_end);
	}

	template<typename stream_t, typename Res>
	using trim_func = Res (*)(stream_t ch);

	template<typename stream_t>
	typename traits<stream_t>::string &trim(
		typename traits<stream_t>::string &str,
		int(*predict)(int)
	) {
		int x = 0,y = str.length() - 1;
		for (; y >= 0 && predict(str[y]); y--);
		for (; x <= y && predict(str[x]); x++);
		str = str.substr(x, y+1);
		return str;
	}

	template<typename stream_t>
	typename traits<stream_t>::string &trim_space(
		typename traits<stream_t>::string &str
	) {
		return trim<stream_t>(str, std::isspace);
	}

	template<typename stream_t>
	std::vector<typename traits<stream_t>::string> &split_space(
		const typename traits<stream_t>::string &s,
		std::vector<typename traits<stream_t>::string> &res
	) {
		res.clear();
		// traits<stream_t>::string
		size_t ls = 0;
		for (int x = 0; x < s.length() ;x++) {
			if (isspace(s[x])) {
				if (x != ls) res.push_back(s.substr(ls, x-ls));
				while(x < s.length() && isspace(s[x])) x++;
				ls = x;
			}
		}
		if (ls != s.length()) res.push_back(s.substr(ls));
		return res;
	}
}

namespace is_iterable
{
	namespace detail
	{
		// To allow ADL with custom begin/end
		using std::begin;
		using std::end;

		template <typename T>
		auto is_iterable_impl(int)
		-> decltype (
			begin(std::declval<T&>()) != end(std::declval<T&>()), // begin/end and operator !=
			void(), // Handle evil operator ,
			++std::declval<decltype(begin(std::declval<T&>()))&>(), // operator ++
			void(*begin(std::declval<T&>())), // operator*
			std::true_type{});

		template <typename T>
		std::false_type is_iterable_impl(...);
	}

	template <typename T>
	using is_iterable = decltype(detail::is_iterable_impl<T>(0));
} // namespace is_iterable


namespace print
{

	template<typename T, typename ...Types>
	typename std::enable_if<!is_iterable::is_iterable<T>::value>::type
	print(T x, bool end = false) {
		std::cout << x;
		if (end) 
			std::cout << std::endl;
	}

	template<typename F, typename S, typename ...Types>
	void print(std::pair<F, S> x, bool end = false) {
		std::cout << "(" << x.first << "," << x.second << ")";
		if (end) 
			std::cout << std::endl;
	}

	template<typename T, typename ...Types>
	void print(typename std::enable_if<is_iterable::is_iterable<T>::value, T>::type xs, bool end = false) {
		std::cout << "{";
		bool vd = false;
		for(auto &x:xs) {
			if (vd) std::cout << ","; else vd = true; 
			print(x);
		}
		std::cout << "}";
		if (end) 
			std::cout << std::endl;
	}
} // namespace printer




// ( num )
// ( num - num * num / num )
// T -> F T


namespace parse {
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
		using string = std::string;
		using strvec = std::vector<string>;
		using symbol = Symbol<term_t, uterm_t>;
		using model = Model<term_t, uterm_t>;
		friend class Processor;
		string err;
		std::map<string, symbol> sym_table;
		std::vector<Production<symbol>> prods;
		public:
		Model() {}
		bool is_error() { return err.length() != 0; }
		const string &error() { return err; }
		private:
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
			prods.emplace_back(std::move(Production(lhs, functional::map_st(
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
					if constexpr(model_t::symbol::uterm_v) {
						model->sym_from_string_true(line);
					} else {
						model->sym_from_string_false(line);
					}
				} else if (!strncmp(line.c_str(), "term ", 5)) {
					if constexpr(model_t::symbol::term_v) {
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
}

int main() {

	auto model = parse::G<Token, parse::UTerm>("grammar.gr");
	if (model->is_error()) {
		std::cout << model->error() << std::endl;
	}
	delete model;
	// T -> F T
	
	// auto lexer_result = LexerResult(fromIstream(std::cin));
	// for (auto x :lexer_result.tokens) {
	//     std::cout << std::underlying_type_t<Token>(x) << " ";
	// }
	// std::cout << std::endl;
	// auto parser = Parser<Token, LexerResult, token_adapt_example>(lexer_result);
	// auto result = parser.parse();
	// std::cout << *result << std::endl;
	// delete result;
}

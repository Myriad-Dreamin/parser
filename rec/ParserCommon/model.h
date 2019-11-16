#pragma once

#include "symbol.h"
#include "production.h"
#include "functional/functional.h"
#include "string-algorithm/string-algorithm.h"
#include "print.h"
#include "cxx17.h"

#include <iostream>
#include <string>
#include <vector>
#include <map>

namespace parse {
	template<typename term_t = int32_t, typename uterm_t = int32_t>
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
		bool is_error() {
			return err.length() != 0;
		}
		const string &error() {
			return err;
		}
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
			for (auto &x : slice)
				apply_sym_def<true>(x);
		}

		void sym_from_string_false(string &s) {
			strvec slice;
			functional::map(salg::split(s.substr(5), ',', slice), salg::trim_space<char>);
			for (auto &x : slice)
				apply_sym_def<false>(x);
		}

		void prod_from_string(string &s) {
			strvec slice;
			if (salg::split(s, "->", slice).size() != 2) {
				throw std::invalid_argument("must in form of A -> B, not " + s);
			}
			auto &lhs = slice[0], &rhss = slice[1];

			strvec slice2;
			for (auto &rhs : functional::map(salg::split(rhss, '|', slice2), salg::trim_space<char>))
				prod_from_pair(lhs, rhs);

		}

		void prod_from_pair(string &raw_lhs, const string &raw_rhs) {
			symbol_t lhs = sym_table.at(salg::trim_space<char>(raw_lhs));
			strvec slice;
			std::vector<symbol_t> rhs;
			//todo
			prods.emplace_back(std::move(Production<symbol_t>(lhs, functional::map_st(
				functional::map(salg::split_space<char>(raw_rhs, slice), salg::trim_space<char>),
				rhs, [&](const string &r) {return sym_table.at(r); }
			))));
		}

		void prods_from_string(string &s) {
			strvec slice;
			functional::map(salg::split(s, '\n', slice), salg::trim_space<char>);
			functional::map_void(slice, [&](string &s) {
				if (s.empty()) return;
				this->prod_from_string(s);
			});
		}

		template<typename u, typename v>
		friend Model<u, v> *G(std::istream &in);
	};

	template<typename term_t = int32_t, typename uterm_t = int32_t>
	Model<term_t, uterm_t> *G(std::istream &in) {
		using model_t = Model<term_t, uterm_t>;
		std::string line;
		std::vector<std::string> atos;
		auto model = new Model<term_t, uterm_t>();
		try {
			while (salg::get_till<char>(in, line)) {
				if (salg::trim_space<char>(line).empty()) continue;
				if (line[0] == '#') {
					continue;
				} else if (line[0] == 'P') {
					auto pos = line.find('{');
					if (pos == std::string::npos) {
						return model->error("invalid statement: miss {");
					}
					salg::get_till_nc(in, line = line.substr(pos + 1), '}');
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
		} catch (std::exception &e) {
			return model->error(e.what());
		}
		print::print(model->sym_table, true);
		print::print(model->prods, true);
		std::cout << "? ";
		print::print(model->begin_symbol, true);
		return model;
	}


	template<typename term_t = int32_t, typename uterm_t = int32_t>
	Model<term_t, uterm_t> *G(const char *file_name) {
		auto f = std::fstream(file_name, std::ios::in);
		auto g = G<term_t, uterm_t>(f);
		f.close();
		return g;
	}
}


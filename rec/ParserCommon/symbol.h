#pragma once

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

		Symbol(const Symbol<term_t, uterm_t> &x) {
			data = x.data;
			ut = x.ut;
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
				(ut == term_v && static_cast<uint64_t>(data.term) == static_cast<uint64_t>(y.data.term)) ||
				(ut == uterm_v && static_cast<uint64_t>(data.uterm) == static_cast<uint64_t>(y.data.uterm)));
		}

		bool operator!= (const Symbol<term_t, uterm_t> &y) const {
			return !operator==(y);
		}

		template<typename u, typename v>
		friend std::ostream &operator<<(std::ostream & os, Symbol<u, v> &res);
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
}

#pragma once

#include <iostream>
#include <string>

namespace parse {
namespace action_space {
	struct action {
		virtual std::ostream &output(std::ostream &os) = 0;
	};

	std::ostream &operator<<(std::ostream &os, action *x) {
		// print::print(x, false, os);
		os << "{";
		x->output(os);
		os << "}";
		return os;
	}

	template<typename symbol_t>
	struct replace_action: public action {
		const std::vector<symbol_t> &reduce;
		const std::vector<symbol_t> &produce;
		replace_action(const std::vector<symbol_t> &reduce,
			const std::vector<symbol_t> &produce): reduce(reduce), produce(produce) {}
		virtual std::ostream &output(std::ostream &os) override {
			return os << *this;
		}
	};


	template<typename symbol_t>
	std::ostream &operator<<(std::ostream &os, replace_action<symbol_t> &x) {
		os << x.reduce << " -> ";
		print::print(x.produce, false, os);
		return os;
	}


	template<typename symbol_t>
	struct replace_action1: public action {
		const symbol_t &reduce;
		const std::vector<symbol_t> &produce;
		replace_action1(const symbol_t &reduce,
			const std::vector<symbol_t> &produce): reduce(reduce), produce(produce) {

		}
		virtual std::ostream &output(std::ostream &os) override {
			return os << *this;
		}
	};

	template<typename symbol_t>
	std::ostream &operator<<(std::ostream &os, replace_action1<symbol_t> &x) {
		print::print(x.reduce, false, os);
		os << " -> ";
		print::print(x.produce, false, os);
		return os;
	}

	struct shift_action: public action {
		int shift_number;
		virtual std::ostream &output(std::ostream &os) override {
			return os << shift_number;
		}
	};
	
	struct error_action: public action {
		const std::string error_info;
		virtual std::ostream &output(std::ostream &os) override {
			return os << error_info;
		}
	};

	struct synch_action: public action {
		const std::string synch_info;
		virtual std::ostream &output(std::ostream &os) override {
			return os << synch_info;
		}
	};
}
}
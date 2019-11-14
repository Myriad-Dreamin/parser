#pragma once

#include "iterable.h"
#include <string>
#include <type_traits>

namespace print
{

	template<typename T, typename ...Types>
	typename std::enable_if<!is_iterable::is_iterable<T>::value>::type
	print(T x, bool end = false, std::ostream & os = std::cout) {
		os << x;
		if (end) 
			os << std::endl;
	}

	template<typename ...Types>
	void print(const std::string &x, bool end = false, std::ostream & os = std::cout) {
		os << x;
		if (end) 
			os << std::endl;
	}

	template<typename ...Types>
	void print(const char *x, bool end = false, std::ostream & os = std::cout) {
		os << x;
		if (end) 
			os << std::endl;
	}

	template<typename F, typename S, typename ...Types>
	void print(std::pair<F, S> x, bool end = false, std::ostream & os = std::cout) {
		os << "(";
		print(x.first, false, os);
		os << ",";
		print(x.second, false, os);
		os << ")";
		if (end) 
			os << std::endl;
	}

	template<typename T, typename ...Types>
	typename std::enable_if<is_iterable::is_iterable<T>::value>::type
	print(const T &xs, bool end = false, std::ostream & os = std::cout) {
		os << "{";
		bool vd = false;
		for(auto &x:xs) {
			if (vd) os << ","; else vd = true; 
			print(x, false, os);
		}
		os << "}";
		if (end) 
			os << std::endl;
	}
} // namespace printer
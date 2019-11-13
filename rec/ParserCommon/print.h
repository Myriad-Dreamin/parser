#pragma once

#include "iterable.h"
#include <string>
#include <type_traits>

namespace print
{

	template<typename T, typename ...Types>
	typename std::enable_if<!is_iterable::is_iterable<T>::value>::type
	print(T x, bool end = false) {
		std::cout << x;
		if (end) 
			std::cout << std::endl;
	}

	template<typename ...Types>
	void print(const std::string &x, bool end = false) {
		std::cout << x;
		if (end) 
			std::cout << std::endl;
	}

	template<typename ...Types>
	void print(const char *x, bool end = false) {
		std::cout << x;
		if (end) 
			std::cout << std::endl;
	}

	template<typename F, typename S, typename ...Types>
	void print(std::pair<F, S> x, bool end = false) {
		std::cout << "(";
		print(x.first);
		std::cout << ",";
		print(x.second);
		std::cout << ")";
		if (end) 
			std::cout << std::endl;
	}

	template<typename T, typename ...Types>
	typename std::enable_if<is_iterable::is_iterable<T>::value>::type
	print(const T &xs, bool end = false) {
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
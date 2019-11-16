#pragma once

#include <iostream>

namespace parse {

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

}
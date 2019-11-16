#pragma once

#include <stdint.h>
#include <vector>

template<typename T>
const char *stringify(T x);


enum class Token: uint16_t {
	Add,
	Sub,
	Mul,
	Div,
	Lbr,
	Rbr,
	Num,
	Eps = 0xfffd,
	Dol = 0xfffe,
	Eof = 0xffff,
};


std::ostream &operator<<(std::ostream &os, Token x) {
	switch (x) {
		case Token::Add:
			os << "Add";
			break;
		case Token::Sub:
			os << "Sub";
			break;
		case Token::Mul:
			os << "Mul";
			break;
		case Token::Div:
			os << "Div";
			break;
		case Token::Lbr:
			os << "Lbr";
			break;
		case Token::Rbr:
			os << "Rbr";
			break;
		case Token::Num:
			os << "Num";
			break;
		case Token::Eps:
			os << "Eps";
			break;
		case Token::Dol:
			os << "Dol";
			break;
		case Token::Eof:
			os << "Eof";
			break;
		default:
			os << "Token<NotFound>";
			break;
	}
	return os;
}

struct LexerResult {
	using vector = std::vector<Token>;
	vector tokens;
	size_t ofs;
	LexerResult() {
		ofs = 0;
	}
	LexerResult(vector &in) {
		tokens.swap(in);
		ofs = 0;
	}
	void reset() {
		ofs = 0;
	}
	LexerResult &operator>>(Token &b) {
		if (ofs == tokens.size()) {
			b = Token::Eof;
		} else {
			b = tokens[ofs++];
		}
		return *this;
	}
};

using ref_pair = std::map<std::string, Token>::value_type;
std::map<std::string, Token> ref_map = {
	ref_pair("+", Token::Add),
	ref_pair("-", Token::Sub),
	ref_pair("*", Token::Mul),
	ref_pair("/", Token::Div),
	ref_pair("(", Token::Lbr),
	ref_pair(")", Token::Rbr),
	ref_pair("num", Token::Num),
	ref_pair("\xff", Token::Eof),
};

std::vector<Token> &fromIstream(std::istream &in) {
	static std::vector<Token> tokens;
	std::string x;
	tokens.clear();
	try {
		while (in >> x) {
			tokens.push_back(ref_map.at(x));
		}
	} catch (std::out_of_range &oc) {
		throw std::runtime_error(std::string("invalid mock flow") + oc.what());
	}
	return tokens;
}




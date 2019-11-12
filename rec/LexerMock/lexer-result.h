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
	Eof = 0xffff,
};

template<>
const char *stringify(Token x) {
	switch (x) {
		case Token::Add:
			return "Add";
		case Token::Sub:
			return "Sub";
		case Token::Mul:
			return "Mul";
		case Token::Div:
			return "Div";
		case Token::Lbr:
			return "Lbr";
		case Token::Rbr:
			return "Rbr";
		case Token::Num:
			return "Num";
		case Token::Eof:
			return "Eof";
		default:
			return "Token<NotFound>";
	}
}

struct LexerResult {
	using vector = std::vector<Token>;
	vector tokens;
	int64_t ofs;
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
		throw std::runtime_error("invalid mock flow");
	}
	return tokens;
}




#pragma once

#include <string>
#include <vector>


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
		const std::string &sep,
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
		std::string sw(sep);
		return split(s, sw, res);
		#endif
	}

	template<typename stream_t>
	bool get_till_nc(
		typename traits<stream_t>::istream &in,
		typename traits<stream_t>::string &res,
		const stream_t line_end=traits<stream_t>::line_end) {
		for (stream_t s;in.get(s);) {
			if (s == line_end) {
				return true;
			}
			res.push_back(s);
		}
		return res.size() != 0;
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
		return trim<stream_t>(str, isspace);
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
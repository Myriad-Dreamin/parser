#pragma once

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

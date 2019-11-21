#pragma once

#include <algorithm>


template<typename I>
struct __slice {
	I l, r;

	__slice(I l, I r): l(l), r(r) {}

	I begin() {
		return l;
	}
	I end() {
		return r;
	}
};

template<typename I>
struct __combine {
	__slice<I> ls, rs;

	__combine(const __slice<I> &ls, const __slice<I> &rs):ls(ls), rs(rs) {}

	struct iterator {
		__combine<I> &c;
		I cur; bool ls;
		iterator(__combine<I> &c, I cur, bool ls): c(c), cur(cur), ls(ls) {}
		iterator &operator++() {
			cur++;
			if (ls == false && cur == std::end(c.ls)) {
				cur = std::begin(c.rs);
				ls = true;
			}
			return *this;
		}
		iterator &operator++(int) {
			return operator++();
		}
		bool operator!=(const iterator &i) const {
			return ls != i.ls || cur != i.cur;
		}
		bool operator==(const iterator &i) const {
			return ls == i.ls && cur == i.cur;
		}
		auto operator*() -> decltype(*cur) {
			return *cur;
		}
	};

	iterator begin() {
		auto iter = std::begin(ls);
		if (iter == std::end(ls)) {
			return iterator(*this, std::begin(rs), true);
		}
		return iterator(*this, iter, false);
	}

	iterator end() {
		return iterator(*this, std::end(rs), true);
	}
};

template<typename Iterator>
__slice<Iterator> slice(Iterator _begin, Iterator _end) {
	return __slice<Iterator>(_begin, _end);
}

template<typename Container>
auto slice(const Container &c, int l = -1, int r = -1) -> __slice<decltype(std::begin(c))> {
	auto x = std::begin(c), y = std::end(c);
	if (l != -1) {
		if (r != -1) r -= l;
		while (l) {
			x++; l--;
		}
	}
	if (r != -1) {
		y = x;
		while (r) {
			y++; r--;
		}
	}
	return __slice<decltype(std::begin(c))>(x, y);
}

template<typename Iterator>
__combine<Iterator> combine(const __slice<Iterator> &_left, const __slice<Iterator> &_right) {
	return __combine<Iterator>(_left, _right);
}


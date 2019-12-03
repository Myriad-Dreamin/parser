#pragma once

#include "result.h"
#include "action.h"
#include <map>
#include <stack>

namespace parse {
	template<class grammar_traits,
		class container = std::map<
		typename grammar_traits::state_id_t,
		std::map<typename grammar_traits::symbol_t, action_space::action*>*>>
		class BasicLRGrammar {
		public:
			using model_t = typename grammar_traits::model_t;
			using string = typename grammar_traits::string;
			using strvec = typename grammar_traits::strvec;
			using symbol_t = typename grammar_traits::symbol_t;
			using term_t = typename grammar_traits::term_t;
			using production_t = typename grammar_traits::production_t;
			using node_t = typename grammar_traits::node_t;
			using state_id_t = typename grammar_traits::state_id_t;
			using grammar_t = BasicLRGrammar<grammar_traits, container>;
			using result_t = Result<typename grammar_traits::term_t,
				typename grammar_traits::uterm_t>;

		protected:
			container *table;
			std::stack<std::pair<node_t*, state_id_t>> stack;
			symbol_t next_symbol, begin_symbol;
			int error_count;

			void init(container *m_table, const symbol_t &bsym) {
				this->table = m_table;
				this->begin_symbol = bsym;
			}
		public:

			template<typename IStream>
			void read(IStream &in, symbol_t &sym) {
				term_t token;
				in >> token;
				sym = symbol_t(token, symbol_t::term_v);
				if (sym == grammar_traits::eof) {
					sym = grammar_traits::dollar;
				}
			}

			void reset() {
				error_count = 0;
				while (stack.size()) {
					stack.pop();
				}
				next_symbol = grammar_traits::epsilon;
			}
			template<class IStream>
			result_t *work(IStream &is) {
				reset();
				auto result = new result_t();
				auto rt = result->alloc(begin_symbol);
				result->rt = rt;
				stack.push(std::make_pair(rt, 0));
				read(is, next_symbol);
				while (stack.size()) {
// #ifdef DEBUG
// 					std::cout << "state size " << stack.size() << " ";
// #endif // DEBUG

					// print::print(stack.top(), true);
					auto acmp = (*table)[stack.top().second];
					if (!acmp->count(next_symbol)) {
#ifdef DEBUG
						std::cout << "state " << stack.top().second << " find action failed ";
						print::print(next_symbol, true);
#endif // DEBUG
						error_count++;
						read(is, next_symbol);
						continue;
			}
					auto norm_action = (*acmp)[next_symbol];
					if (auto action = dynamic_cast<action_space::shift_action<state_id_t>*>(norm_action)) {
#ifdef DEBUG
						std::cout << "state " << stack.top().second << " shift in " << next_symbol << std::endl;
#endif // DEBUG
						stack.push({result->alloc(next_symbol), action->to_state});
						read(is, next_symbol);

					} else if (auto action = dynamic_cast<action_space::replace_action1<symbol_t>*>(norm_action)) {
						auto mrt = result->alloc(action->reduce);
#ifdef DEBUG
						std::cout << "state " << stack.top().second << " reduced ";
#endif // DEBUG
						for (typename std::make_signed<size_t>::type i = action->produce.size() - 1; i >= 0; i--) {
#ifdef DEBUG
							if (stack.top().first->symbol != action->produce[i]) {
								std::cout << "unmatched reduction " << i << " ";
								print::print(action->produce, true);
	}
#endif
							mrt->ch.push_back(stack.top().first);
							stack.pop();
}
						std::reverse(mrt->ch.begin(), mrt->ch.end());
						auto acmp2 = (*table)[stack.top().second];
#ifdef DEBUG
						if (!acmp2->count(action->reduce)) {
							std::cout << "unmatched reduction " << stack.top().second << " "; print::print(action->reduce, true);
						}
#endif
#ifdef DEBUG
						std::cout << *mrt << " to state " << (dynamic_cast<action_space::goto_action<state_id_t>*>(
							(*acmp2)[action->reduce]))->to_state << std::endl;
#endif // DEBUG
						stack.push({mrt, (dynamic_cast<action_space::goto_action<state_id_t>*>(
							(*acmp2)[action->reduce]))->to_state});
					} else if (auto action = dynamic_cast<action_space::accept_action*>(norm_action)) {
						result->rt->insert(stack.top().first);
#ifdef DEBUG
						assert(("stack.size() == 2", stack.size() == 2));
#endif
						// std::cout << "stack size " << stack.size() << std::endl;
						if (error_count != 0) {
							result->code = ResultCode::Error;
						}
						return result;
					}
#ifdef DEBUG
					else {
						std::cout << "action (" << stack.top().second << ", ";
						print::print(next_symbol);
						std::cout << ") is not determinted" << std::endl;
				}
#endif // DEBUG

			}
				if (error_count != 0) {
					result->code = ResultCode::Error;
				}
				return result;
		}
};


template<class grammar_traits,
	class container = std::map<
	typename grammar_traits::symbol_t,
	std::map<typename grammar_traits::symbol_t, action_space::action*>*>>
	class BasicLLGrammar {
	public:
		using model_t = typename grammar_traits::model_t;
		using string = typename grammar_traits::string;
		using strvec = typename grammar_traits::strvec;
		using symbol_t = typename grammar_traits::symbol_t;
		using term_t = typename grammar_traits::term_t;
		using production_t = typename grammar_traits::production_t;
		using node_t = typename grammar_traits::node_t;
		using grammar_t = BasicLLGrammar<grammar_traits, container>;
		using result_t = Result<typename grammar_traits::term_t,
			typename grammar_traits::uterm_t>;

	protected:
		container *table;
		std::stack<node_t*> stack;
		symbol_t next_symbol, begin_symbol;
		std::map<symbol_t, std::set<symbol_t>*> follow;
		int error_count;
	public:

		template<typename IStream>
		void read(IStream &in, symbol_t &sym) {
			term_t token;
			in >> token;
			sym = symbol_t(token, symbol_t::term_v);
		}

		action_space::action *act(const symbol_t &s) {};
		void reset() {
			error_count = 0;
			while (stack.size()) {
				stack.pop();
			}
			next_symbol = grammar_traits::epsilon;
		}
		template<class IStream>
		result_t *work(IStream &is) {
			reset();
			auto result = new result_t();
			auto rt = result->alloc(begin_symbol);
			result->rt = rt;
			stack.push(rt);
			read(is, next_symbol);
			while (stack.size()) {
				if (next_symbol == grammar_traits::eof) {
					return result;
				}
				auto state = stack.top();
#ifdef DEBUG
				std::cout << "next symbol "; print::print(next_symbol);
#endif
				if (state->symbol.is_unterm()) {
					auto &acmp = *(*table)[state->symbol];
					if (acmp.count(next_symbol)) {
						auto d0 = dynamic_cast<action_space::replace_action1<symbol_t>*>(acmp[next_symbol]);
						if (d0 != nullptr) {
							auto &prod = *d0;
							if (state->symbol != prod.reduce) {
#ifdef DEBUG
								std::cout << " mismatch production ";
								print::print(prod.reduce);
								std::cout << " -> ";
								print::print(prod.produce, true);
#endif
								if (follow[state->symbol]->count(next_symbol)) {
									stack.pop();
								} else {
									read(is, next_symbol);
								}
								error_count++;
							} else {
#ifdef DEBUG
								std::cout << " use production ";
								print::print(prod.reduce);
									std::cout << " -> ";
								print::print(prod.produce, true);
#endif
								stack.pop();
								for (auto &sym : prod.produce) {
									state->insert(result->alloc(sym));
								}
								for (auto iter = state->ch.rbegin(); iter != state->ch.rend(); iter++) {
									stack.push(*iter);
								}
							}
						} else {
#ifdef DEBUG
							std::cout << " error " << std::endl;
#endif
							auto d1 = dynamic_cast<action_space::error_action*>(acmp[next_symbol]);
							if (d1 != nullptr) {
								auto &info = *d1;
								read(is, next_symbol);
								error_count++;
							} else {
								auto d2 = dynamic_cast<action_space::synch_action*>(acmp[next_symbol]);
								if (d2 != nullptr) {
									auto &info = *d2;
									stack.pop();
									error_count++;
								}
							}
						}
					} else {
						read(is, next_symbol);
						error_count++;
					}
				} else {
#ifdef DEBUG
					std::cout << " matched top stack ";
					print::print(state->symbol, true);
#endif
					if (next_symbol != state->symbol) {
						error_count++;
					} else {
						stack.pop();
					}
					read(is, next_symbol);
				}
			}
			return result;
		}

		template<class IStream>
		result_t *work2(IStream &is) {
			reset();
			auto result = new result_t();
			auto rt = result->alloc(begin_symbol);
			result->rt = rt;
			stack.push(rt);
			read(is, next_symbol);
			while (stack.size()) {
				if (next_symbol == grammar_traits::eof) {
					return result;
				}
				auto state = stack.top();
				if (state->symbol.is_unterm()) {
					auto &acmp = *(*table)[state->symbol];
					if (acmp.count(next_symbol)) {
						auto d0 = dynamic_cast<action_space::replace_action1<symbol_t>*>(acmp[next_symbol]);
						if (d0 != nullptr) {
							auto &prod = *d0;
							if (state->symbol != prod.reduce) {
								if (follow[state->symbol]->count(next_symbol)) {
									stack.pop();
								} else {
									read(is, next_symbol);
								}
								error_count++;
							} else {
								stack.pop();
								for (auto &sym : prod.produce) {
									state->insert(result->alloc(sym));
								}
								for (auto iter = state->ch.rbegin(); iter != state->ch.rend(); iter++) {
									stack.push(*iter);
								}
							}
						} else {
							auto d1 = dynamic_cast<action_space::error_action*>(acmp[next_symbol]);
							if (d1 != nullptr) {
								auto &info = *d1;
								error_count++;
							}
						}
					} else {
						read(is, next_symbol);
						error_count++;
					}
				} else {
					if (next_symbol != state->symbol) {
						error_count++;
					} else {
						stack.pop();
					}
					read(is, next_symbol);
				}
			}
			return result;
		}
};

}

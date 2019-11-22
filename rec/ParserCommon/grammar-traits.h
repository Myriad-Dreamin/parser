#pragma once

#include <stdint.h>
#include "model.h"
#include "ast-node.h"

namespace parse {
	struct __M_TRAITS {
		static const int32_t epsilon = -1;
		static const int32_t dollar = -2;
	};

	template<typename __term_t = int32_t, typename __uterm_t = int32_t, class m_traits = __M_TRAITS, typename __state_id_t = int32_t>
	struct basic_grammar_traits {
		using term_t = __term_t;
		using uterm_t = __uterm_t;
		using model_t = Model<term_t, uterm_t>;
		using string = typename model_t::string;
		using strvec = typename model_t::strvec;
		using symbol_t = typename model_t::symbol_t;
		using production_t = Production<symbol_t>;
		using node_t = ASTNode<term_t, uterm_t>;
		using state_id_t = __state_id_t;
		static constexpr auto eof = m_traits::eof;
		static constexpr auto epsilon = m_traits::epsilon;
		static constexpr auto dollar = m_traits::dollar;
	};

}

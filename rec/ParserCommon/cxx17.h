#pragma once

#if __cplusplus >= 201703L
#define CXX17_STATIC_CONDITION constexpr
#else
#define CXX17_STATIC_CONDITION
#endif

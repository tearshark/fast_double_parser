#pragma once
#include <limits>
#include <tuple>

#define JSON_ENABLE_SSE4	0

#include <cstdlib>
#include <mmintrin.h>		//MMX
#include <xmmintrin.h>		//SSE
#include <emmintrin.h>		//SSE2
#if JSON_ENABLE_SSE4
#include <pmmintrin.h>		//SSE3
#include <tmmintrin.h>		//SSSE3
#include <smmintrin.h>		//SSE4.1
#include <nmmintrin.h>		//SSE4.2
#endif


namespace simd_double_parser
{
	union number_value
	{
		int64_t l;
		double d;
	};

	enum struct parser_result
	{
		Invalid,
		Long,
		Double,
	};

#include "simd_double_parser.inl"

	template<class _CharType>
	std::tuple<number_value, parser_result> parser(const _CharType* psz)
	{
		typedef x_convert_char_selector<sizeof(_CharType)> char_selector;
		return simd_double_parser2((typename char_selector::type*)psz);
	}
}

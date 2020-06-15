
namespace
{
	/*
	#define _MM_SHUFFLE8(fp7, fp6, fp5, fp4, fp3, fp2, fp1, fp0)\
		(((fp7) << 21) | ((fp6) << 18) | ((fp5) << 15) | ((fp4) << 12)) | \
		(((fp3) << 9) | ((fp2) << 6) | ((fp1) << 3) | ((fp0)))

	__m128i _mm_shuffle_epi16(__m128i _A, int _Imm)
	{
		_Imm &= 0xffffff;
		char m01 = (_Imm >> 0) & 0x7, m03 = (_Imm >> 3) & 0x7;
		char m05 = (_Imm >> 6) & 0x7, m07 = (_Imm >> 9) & 0x7;
		char m09 = (_Imm >> 12) & 0x7, m11 = (_Imm >> 15) & 0x7;
		char m13 = (_Imm >> 18) & 0x7, m15 = (_Imm >> 21) & 0x7;
		m01 <<= 1; m03 <<= 1; m05 <<= 1; m07 <<= 1;
		m09 <<= 1; m11 <<= 1; m13 <<= 1; m15 <<= 1;
		char m00 = m01 + 1, m02 = m03 + 1, m04 = m05 + 1, m06 = m07 + 1;
		char m08 = m09 + 1, m10 = m11 + 1, m12 = m13 + 1, m14 = m15 + 1;

		//__m128i vMask = _mm_set_epi8(m00, m01, m02, m03, m04, m05, m06, m07, m08, m09, m10, m11, m12, m13, m14, m15);
		__m128i vMask = _mm_set_epi8(m14, m15, m12, m13, m10, m11, m08, m09, m06, m07, m04, m05, m02, m03, m00, m01);
		return _mm_shuffle_epi8(_A, vMask);
	}
	*/

	//a��b����8��16λ������
	//r[0..7] = a[0]*b[0]+a[1]*b[1]+a[2]*b[2]+a[3]*b[3]+a[4]*b[4]+a[5]*b[5]+a[6]*b[6]+a[7]*b[7]
	inline __m128i x_mm_dotp_i16x4(__m128i a, __m128i b)
	{
		__m128i value = _mm_mullo_epi16(a, b);
		value = _mm_add_epi16(value, _mm_shufflelo_epi16(value, _MM_SHUFFLE(0, 1, 2, 3)));
		value = _mm_add_epi16(value, _mm_shufflelo_epi16(value, _MM_SHUFFLE(1, 0, 3, 2)));
		return value;
	}

	//��4��16λ��ĸ(���Ը�4��16λ��ĸ����ת����һ�����9999������
	//i16x4_mul��4��16λ����(��4��16λ���ֱ����趨Ϊ0)��������Ҫ�趨��ת�����ٸ���ĸ:
	//	4��ĸ : _mm_set_epi16(0, 0, 0, 0, 1, 10, 100, 1000)
	//	3��ĸ : _mm_set_epi16(0, 0, 0, 0, 0, 1, 10, 100)
	//	2��ĸ : _mm_set_epi16(0, 0, 0, 0, 0, 0, 1, 10)
	//	1��ĸ : _mm_set_epi16(0, 0, 0, 0, 0, 0, 0, 1)
	//r = (i16x4[0]-'0')*i16x4_mul[0] + (i16x4[1]-'0')*i16x4_mul[1] + (i16x4[2]-'0')*i16x4_mul[2] + (i16x4[3]-'0')*i16x4_mul[3]
	inline int x_mm_cvt_i16x4_i32(__m128i i16x4, __m128i i16x4_mul)
	{
		i16x4 = _mm_sub_epi16(i16x4, _mm_set_epi16(0, 0, 0, 0, '0', '0', '0', '0'));

		__m128i i16x4_dp = x_mm_dotp_i16x4(i16x4, i16x4_mul);
		int val = _mm_cvtsi128_si32(i16x4_dp) & 0xffff;

		return val;
	}

	//��4��8λ��ĸ(���Ը�12��8λ��ĸ����ת����һ�����9999������
	//i16x4_mul��4��16λ����(��4��16λ���ֱ����趨Ϊ0)��������Ҫ�趨��ת�����ٸ���ĸ:
	//	4��ĸ : _mm_set_epi16(0, 0, 0, 0, 1, 10, 100, 1000)
	//	3��ĸ : _mm_set_epi16(0, 0, 0, 0, 0, 1, 10, 100)
	//	2��ĸ : _mm_set_epi16(0, 0, 0, 0, 0, 0, 1, 10)
	//	1��ĸ : _mm_set_epi16(0, 0, 0, 0, 0, 0, 0, 1)
	//r = (i8x4[0]-'0')*i16x4_mul[0] + (i8x4[1]-'0')*i16x4_mul[1] + (i8x4[2]-'0')*i16x4_mul[2] + (i8x4[3]-'0')*i16x4_mul[3]
	inline int x_mm_cvt_i8x4_i32(__m128i i8x4, __m128i i16x4_mul)
	{
		__m128i i16x4 = _mm_unpacklo_epi8(i8x4, _mm_setzero_si128());
		return x_mm_cvt_i16x4_i32(i16x4, i16x4_mul);
	}

	//��4��32λ��ĸ��ת����һ�����9999������
	//i16x4_mul��4��16λ����(��4��16λ���ֱ����趨Ϊ0)��������Ҫ�趨��ת�����ٸ���ĸ:
	//	4��ĸ : _mm_set_epi16(0, 0, 0, 0, 1, 10, 100, 1000)
	//	3��ĸ : _mm_set_epi16(0, 0, 0, 0, 0, 1, 10, 100)
	//	2��ĸ : _mm_set_epi16(0, 0, 0, 0, 0, 0, 1, 10)
	//	1��ĸ : _mm_set_epi16(0, 0, 0, 0, 0, 0, 0, 1)
	//r = (i32x4[0]-'0')*i16x4_mul[0] + (i32x4[1]-'0')*i16x4_mul[1] + (i32x4[2]-'0')*i16x4_mul[2] + (i32x4[3]-'0')*i16x4_mul[3]
	inline int x_mm_cvt_i32x4_i32(__m128i i32x4, __m128i i16x4_mul)
	{
		__m128i i16x4 = _mm_packs_epi32(i32x4, _mm_setzero_si128());
		return x_mm_cvt_i16x4_i32(i16x4, i16x4_mul);
	}

	//�ж�x�ǲ���һ��������ĸ([0,9]֮�����ĸ)
#ifndef x_is_digit
	inline bool x_is_digit(int32_t x)
	{
		return ((x - '0') | ('9' - x)) >= 0;
	}
#endif



	//wchar_t�ڲ�ͬ��ƽ̨�ϣ��ǲ�ͬ�ġ�16λ����32λ������ϸ�ڻ᲻ͬ
	//�ʲ��ܼ򵥵����char/wchar_t�����ػ�
	template<size_t _CharSize>
	struct x_convert_char_selector;

	template<>
	struct x_convert_char_selector<1>
	{
		using type = char;

		static __m128i load_xcharx4(const type* s)
		{
			return _mm_cvtsi32_si128(*(int*)s);
		}

		//i8x4��16��8λ��ĸ
		//��� i8x4[0], i8x4[1], i8x4[2], i8x4[3] �ĸ����ǲ��Ƕ���['0', '9']֮���������ĸ
		//����i8x4[4...15]
		//r.bits[0] = is_digit(i8x4[0])
		//r.bits[1] = is_digit(i8x4[1])
		//r.bits[2] = is_digit(i8x4[2])
		//r.bits[3] = is_digit(i8x4[3])
		static int digit_mask(__m128i i8x4)
		{
			__m128i i8x16_gt = _mm_cmpgt_epi8(i8x4, _mm_set_epi8(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '0' - 1, '0' - 1, '0' - 1, '0' - 1));
			__m128i i8x16_lt = _mm_cmplt_epi8(i8x4, _mm_set_epi8(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '9' + 1, '9' + 1, '9' + 1, '9' + 1));
			__m128i i8x16_and = _mm_and_si128(i8x16_gt, i8x16_lt);
			int i8x4_mask = _mm_movemask_epi8(i8x16_and);
			return i8x4_mask;
		}

		//���ַ���ת��Ϊ����
		//�������������ַ������߳���int64_t�ɱ��ķ�Χ����ֹͣ
		//psz:��Ҫת�����ַ���
		//overflow:�������������������Ϊtrue�����򣬲���ı�overflow��ֵ
		//����ֵ:�Ѿ�ת��������
		static int64_t convert_long(int64_t result, const type*& psz, bool& overflow)
		{
			const int64_t MAX_LONG = (std::numeric_limits<int64_t>::max)();
			const int64_t LIMIT_LONG_9999 = (MAX_LONG - 9999) / 10000;
			const int64_t LIMIT_LONG_999 = (MAX_LONG - 999) / 1000;
			const int64_t LIMIT_LONG_99 = (MAX_LONG - 99) / 100;
			const int64_t LIMIT_LONG_9 = (MAX_LONG - 9) / 10;

			const type* s = psz;
			for (; ; )
			{
				__m128i i8x4 = load_xcharx4(s);
				int mask = digit_mask(i8x4);
				if (mask == 0xf)
				{
					if (result > LIMIT_LONG_9999) break;

					int val = x_mm_cvt_i8x4_i32(i8x4, _mm_set_epi16(0, 0, 0, 0, 1, 10, 100, 1000));
					result = result * 10000 + val;
					s += 4;
				}
				else if ((mask & 0x7) == 0x7)
				{
					if (result > LIMIT_LONG_999) break;

					int val = x_mm_cvt_i8x4_i32(i8x4, _mm_set_epi16(0, 0, 0, 0, 0, 1, 10, 100));
					result = result * 1000 + val;

					psz = s + 3;
					return result;
				}
				else if ((mask & 0x3) == 0x3)
				{
					if (result > LIMIT_LONG_99) break;

					result = result * 100 + (s[0] - '0') * 10 + (s[1] - '0');

					psz = s + 2;
					return result;
				}
				else if ((mask & 0x1) == 0x1)
				{
					if (result > LIMIT_LONG_9) break;

					result = result * 10 + (*s - '0');

					psz = s + 1;
					return result;
				}
				else
				{
					psz = s;
					return result;
				}
			}

			for (; x_is_digit(*s); ++s)
			{
				if (result >= 0x0CCCCCCCCCCCCCCCULL)	// 2^63 = 9223372036854775808
				{
					if (result != 0x0CCCCCCCCCCCCCCCULL || *s >= '8')
					{
						overflow = true;
						break;
					}
				}

				result = result * 10 + (*s - '0');
			}

			psz = s;
			return result;
		}
	};

	template<>
	struct x_convert_char_selector<2>
	{
		using type = char16_t;

		static __m128i load_xcharx4(const type* s)
		{
			return _mm_cvtsi64_si128(*(int64_t*)s);
		}

		//i16x4��8��16λ��ĸ
		//��� i16x4[0], i16x4[1], i16x4[2], i16x4[3] �ĸ����ǲ��Ƕ���['0', '9']֮���������ĸ
		//����i8x4[4...7]
		//���ڴ���unicode 16bits
		//r.bits[0,1] = is_digit(i16x4[0])
		//r.bits[2,3] = is_digit(i16x4[1])
		//r.bits[4,5] = is_digit(i16x4[2])
		//r.bits[6,7] = is_digit(i16x4[3])
		static int digit_mask(__m128i i16x4)
		{
			__m128i i16x8_gt = _mm_cmpgt_epi16(i16x4, _mm_set_epi16(0, 0, 0, 0, '0' - 1, '0' - 1, '0' - 1, '0' - 1));
			__m128i i16x8_lt = _mm_cmplt_epi16(i16x4, _mm_set_epi16(0, 0, 0, 0, '9' + 1, '9' + 1, '9' + 1, '9' + 1));
			__m128i i16x8_and = _mm_and_si128(i16x8_gt, i16x8_lt);
			int i8x4_mask = _mm_movemask_epi8(i16x8_and);
			return i8x4_mask;
		}

		//���ַ���ת��Ϊ����
		//�������������ַ������߳���int64_t�ɱ��ķ�Χ����ֹͣ
		//psz:��Ҫת�����ַ���
		//overflow:�������������������Ϊtrue�����򣬲���ı�overflow��ֵ
		//����ֵ:�Ѿ�ת��������
		static int64_t convert_long(int64_t result, const type*& psz, bool& overflow)
		{
			const int64_t MAX_LONG = (std::numeric_limits<int64_t>::max)();
			const int64_t LIMIT_LONG_9999 = (MAX_LONG - 9999) / 10000;
			const int64_t LIMIT_LONG_999 = (MAX_LONG - 999) / 1000;
			const int64_t LIMIT_LONG_99 = (MAX_LONG - 99) / 100;
			const int64_t LIMIT_LONG_9 = (MAX_LONG - 9) / 10;

			const type* s = psz;
			for (; ; )
			{
				__m128i i16x4 = load_xcharx4(s);
				int mask = digit_mask(i16x4);
				if (mask == 0xff)
				{
					if (result > LIMIT_LONG_9999) break;

					int val = x_mm_cvt_i16x4_i32(i16x4, _mm_set_epi16(0, 0, 0, 0, 1, 10, 100, 1000));
					result = result * 10000 + val;
					s += 4;
				}
				else if ((mask & 0x3f) == 0x3f)
				{
					if (result > LIMIT_LONG_999) break;

					int val = x_mm_cvt_i16x4_i32(i16x4, _mm_set_epi16(0, 0, 0, 0, 0, 1, 10, 100));
					result = result * 1000 + val;

					psz = s + 3;
					return result;
				}
				else if ((mask & 0x0f) == 0x0f)
				{
					if (result > LIMIT_LONG_99) break;

					result = result * 100 + (s[0] - '0') * 10 + (s[1] - '0');

					psz = s + 2;
					return result;
				}
				else if ((mask & 0x03) == 0x03)
				{
					if (result > LIMIT_LONG_9) break;

					result = result * 10 + (*s - '0');

					psz = s + 1;
					return result;
				}
				else
				{
					psz = s;
					return result;
				}
			}

			for (; x_is_digit(*s); ++s)
			{
				if (result >= 0x0CCCCCCCCCCCCCCCULL)	// 2^63 = 9223372036854775808
				{
					if (result != 0x0CCCCCCCCCCCCCCCULL || *s >= '8')
					{
						overflow = true;
						break;
					}
				}

				result = result * 10 + (*s - '0');
			}

			psz = s;
			return result;
		}
	};

	template<>
	struct x_convert_char_selector<4>
	{
		using type = char32_t;

		static __m128i load_xcharx4(const type* s)
		{
			return _mm_loadu_si128((__m128i*)s);
		}

		//i32x4��4��32λ��ĸ
		//��� i32x4[0], i32x4[1], i32x4[2], i32x4[3] �ĸ����ǲ��Ƕ���['0', '9']֮���������ĸ
		//���ڴ���unicode 32bits
		//r.bits[0 ...3 ] = is_digit(i32x4[0])
		//r.bits[4 ...7 ] = is_digit(i32x4[1])
		//r.bits[8 ...11] = is_digit(i32x4[2])
		//r.bits[12...15] = is_digit(i32x4[3])
		static int digit_mask(__m128i i32x4)
		{
			__m128i i32x4_gt = _mm_cmpgt_epi32(i32x4, _mm_set_epi32('0' - 1, '0' - 1, '0' - 1, '0' - 1));
			__m128i i32x4_lt = _mm_cmplt_epi32(i32x4, _mm_set_epi32('9' + 1, '9' + 1, '9' + 1, '9' + 1));
			__m128i i32x4_and = _mm_and_si128(i32x4_gt, i32x4_lt);
			int i32x4_mask = _mm_movemask_epi8(i32x4_and);
			return i32x4_mask;
		}

		//���ַ���ת��Ϊ����
		//�������������ַ������߳���int64_t�ɱ��ķ�Χ����ֹͣ
		//psz:��Ҫת�����ַ���
		//overflow:�������������������Ϊtrue�����򣬲���ı�overflow��ֵ
		//����ֵ:�Ѿ�ת��������
		static int64_t convert_long(int64_t result, const type*& psz, bool& overflow)
		{
			const int64_t MAX_LONG = (std::numeric_limits<int64_t>::max)();
			const int64_t LIMIT_LONG_9999 = (MAX_LONG - 9999) / 10000;
			const int64_t LIMIT_LONG_999 = (MAX_LONG - 999) / 1000;
			const int64_t LIMIT_LONG_99 = (MAX_LONG - 99) / 100;
			const int64_t LIMIT_LONG_9 = (MAX_LONG - 9) / 10;

			const type* s = psz;
			for (; ; )
			{
				__m128i i32x4 = load_xcharx4(s);
				int mask = digit_mask(i32x4);
				if (mask == 0xffff)
				{
					if (result > LIMIT_LONG_9999) break;

					int val = x_mm_cvt_i32x4_i32(i32x4, _mm_set_epi16(0, 0, 0, 0, 1, 10, 100, 1000));
					result = result * 10000 + val;
					s += 4;
				}
				else if ((mask & 0x0fff) == 0x0fff)
				{
					if (result > LIMIT_LONG_999) break;

					int val = x_mm_cvt_i32x4_i32(i32x4, _mm_set_epi16(0, 0, 0, 0, 0, 1, 10, 100));
					result = result * 1000 + val;

					psz = s + 3;
					return result;
				}
				else if ((mask & 0x00ff) == 0x00ff)
				{
					if (result > LIMIT_LONG_99) break;

					result = result * 100 + (s[0] - '0') * 10 + (s[1] - '0');

					psz = s + 2;
					return result;
				}
				else if ((mask & 0x000f) == 0x000f)
				{
					if (result > LIMIT_LONG_9) break;

					result = result * 10 + (*s - '0');

					psz = s + 1;
					return result;
				}
				else
				{
					psz = s;
					return result;
				}
			}

			for (; x_is_digit(*s); ++s)
			{
				if (result >= 0x0CCCCCCCCCCCCCCCULL)	// 2^63 = 9223372036854775808
				{
					if (result != 0x0CCCCCCCCCCCCCCCULL || *s >= '8')
					{
						overflow = true;
						break;
					}
				}

				result = result * 10 + (*s - '0');
			}

			psz = s;
			return result;
		}
	};

	static const double DOUBLE_E[] =
	{ // 1e-0...1e308: 309 * 8 bytes = 2472 bytes
		1e+0,
		1e+1,  1e+2,  1e+3,  1e+4,  1e+5,  1e+6,  1e+7,  1e+8,  1e+9,  1e+10, 1e+11, 1e+12, 1e+13, 1e+14, 1e+15, 1e+16, 1e+17, 1e+18, 1e+19, 1e+20,
		1e+21, 1e+22, 1e+23, 1e+24, 1e+25, 1e+26, 1e+27, 1e+28, 1e+29, 1e+30, 1e+31, 1e+32, 1e+33, 1e+34, 1e+35, 1e+36, 1e+37, 1e+38, 1e+39, 1e+40,
		1e+41, 1e+42, 1e+43, 1e+44, 1e+45, 1e+46, 1e+47, 1e+48, 1e+49, 1e+50, 1e+51, 1e+52, 1e+53, 1e+54, 1e+55, 1e+56, 1e+57, 1e+58, 1e+59, 1e+60,
		1e+61, 1e+62, 1e+63, 1e+64, 1e+65, 1e+66, 1e+67, 1e+68, 1e+69, 1e+70, 1e+71, 1e+72, 1e+73, 1e+74, 1e+75, 1e+76, 1e+77, 1e+78, 1e+79, 1e+80,
		1e+81, 1e+82, 1e+83, 1e+84, 1e+85, 1e+86, 1e+87, 1e+88, 1e+89, 1e+90, 1e+91, 1e+92, 1e+93, 1e+94, 1e+95, 1e+96, 1e+97, 1e+98, 1e+99, 1e+100,
		1e+101,1e+102,1e+103,1e+104,1e+105,1e+106,1e+107,1e+108,1e+109,1e+110,1e+111,1e+112,1e+113,1e+114,1e+115,1e+116,1e+117,1e+118,1e+119,1e+120,
		1e+121,1e+122,1e+123,1e+124,1e+125,1e+126,1e+127,1e+128,1e+129,1e+130,1e+131,1e+132,1e+133,1e+134,1e+135,1e+136,1e+137,1e+138,1e+139,1e+140,
		1e+141,1e+142,1e+143,1e+144,1e+145,1e+146,1e+147,1e+148,1e+149,1e+150,1e+151,1e+152,1e+153,1e+154,1e+155,1e+156,1e+157,1e+158,1e+159,1e+160,
		1e+161,1e+162,1e+163,1e+164,1e+165,1e+166,1e+167,1e+168,1e+169,1e+170,1e+171,1e+172,1e+173,1e+174,1e+175,1e+176,1e+177,1e+178,1e+179,1e+180,
		1e+181,1e+182,1e+183,1e+184,1e+185,1e+186,1e+187,1e+188,1e+189,1e+190,1e+191,1e+192,1e+193,1e+194,1e+195,1e+196,1e+197,1e+198,1e+199,1e+200,
		1e+201,1e+202,1e+203,1e+204,1e+205,1e+206,1e+207,1e+208,1e+209,1e+210,1e+211,1e+212,1e+213,1e+214,1e+215,1e+216,1e+217,1e+218,1e+219,1e+220,
		1e+221,1e+222,1e+223,1e+224,1e+225,1e+226,1e+227,1e+228,1e+229,1e+230,1e+231,1e+232,1e+233,1e+234,1e+235,1e+236,1e+237,1e+238,1e+239,1e+240,
		1e+241,1e+242,1e+243,1e+244,1e+245,1e+246,1e+247,1e+248,1e+249,1e+250,1e+251,1e+252,1e+253,1e+254,1e+255,1e+256,1e+257,1e+258,1e+259,1e+260,
		1e+261,1e+262,1e+263,1e+264,1e+265,1e+266,1e+267,1e+268,1e+269,1e+270,1e+271,1e+272,1e+273,1e+274,1e+275,1e+276,1e+277,1e+278,1e+279,1e+280,
		1e+281,1e+282,1e+283,1e+284,1e+285,1e+286,1e+287,1e+288,1e+289,1e+290,1e+291,1e+292,1e+293,1e+294,1e+295,1e+296,1e+297,1e+298,1e+299,1e+300,
		1e+301,1e+302,1e+303,1e+304,1e+305,1e+306,1e+307,1e+308
	};

	static const double DOUBLE_NE[] =
	{ // 1e-0...1e308: 309 * 8 bytes = 2472 bytes
		1e-0,
		1e-1,  1e-2,  1e-3,  1e-4,  1e-5,  1e-6,  1e-7,  1e-8,  1e-9,  1e-10, 1e-11, 1e-12, 1e-13, 1e-14, 1e-15, 1e-16, 1e-17, 1e-18, 1e-19, 1e-20,
		1e-21, 1e-22, 1e-23, 1e-24, 1e-25, 1e-26, 1e-27, 1e-28, 1e-29, 1e-30, 1e-31, 1e-32, 1e-33, 1e-34, 1e-35, 1e-36, 1e-37, 1e-38, 1e-39, 1e-40,
		1e-41, 1e-42, 1e-43, 1e-44, 1e-45, 1e-46, 1e-47, 1e-48, 1e-49, 1e-50, 1e-51, 1e-52, 1e-53, 1e-54, 1e-55, 1e-56, 1e-57, 1e-58, 1e-59, 1e-60,
		1e-61, 1e-62, 1e-63, 1e-64, 1e-65, 1e-66, 1e-67, 1e-68, 1e-69, 1e-70, 1e-71, 1e-72, 1e-73, 1e-74, 1e-75, 1e-76, 1e-77, 1e-78, 1e-79, 1e-80,
		1e-81, 1e-82, 1e-83, 1e-84, 1e-85, 1e-86, 1e-87, 1e-88, 1e-89, 1e-90, 1e-91, 1e-92, 1e-93, 1e-94, 1e-95, 1e-96, 1e-97, 1e-98, 1e-99, 1e-100,
		1e-101,1e-102,1e-103,1e-104,1e-105,1e-106,1e-107,1e-108,1e-109,1e-110,1e-111,1e-112,1e-113,1e-114,1e-115,1e-116,1e-117,1e-118,1e-119,1e-120,
		1e-121,1e-122,1e-123,1e-124,1e-125,1e-126,1e-127,1e-128,1e-129,1e-130,1e-131,1e-132,1e-133,1e-134,1e-135,1e-136,1e-137,1e-138,1e-139,1e-140,
		1e-141,1e-142,1e-143,1e-144,1e-145,1e-146,1e-147,1e-148,1e-149,1e-150,1e-151,1e-152,1e-153,1e-154,1e-155,1e-156,1e-157,1e-158,1e-159,1e-160,
		1e-161,1e-162,1e-163,1e-164,1e-165,1e-166,1e-167,1e-168,1e-169,1e-170,1e-171,1e-172,1e-173,1e-174,1e-175,1e-176,1e-177,1e-178,1e-179,1e-180,
		1e-181,1e-182,1e-183,1e-184,1e-185,1e-186,1e-187,1e-188,1e-189,1e-190,1e-191,1e-192,1e-193,1e-194,1e-195,1e-196,1e-197,1e-198,1e-199,1e-200,
		1e-201,1e-202,1e-203,1e-204,1e-205,1e-206,1e-207,1e-208,1e-209,1e-210,1e-211,1e-212,1e-213,1e-214,1e-215,1e-216,1e-217,1e-218,1e-219,1e-220,
		1e-221,1e-222,1e-223,1e-224,1e-225,1e-226,1e-227,1e-228,1e-229,1e-230,1e-231,1e-232,1e-233,1e-234,1e-235,1e-236,1e-237,1e-238,1e-239,1e-240,
		1e-241,1e-242,1e-243,1e-244,1e-245,1e-246,1e-247,1e-248,1e-249,1e-250,1e-251,1e-252,1e-253,1e-254,1e-255,1e-256,1e-257,1e-258,1e-259,1e-260,
		1e-261,1e-262,1e-263,1e-264,1e-265,1e-266,1e-267,1e-268,1e-269,1e-270,1e-271,1e-272,1e-273,1e-274,1e-275,1e-276,1e-277,1e-278,1e-279,1e-280,
		1e-281,1e-282,1e-283,1e-284,1e-285,1e-286,1e-287,1e-288,1e-289,1e-290,1e-291,1e-292,1e-293,1e-294,1e-295,1e-296,1e-297,1e-298,1e-299,1e-300,
		1e-301,1e-302,1e-303,1e-304,1e-305,1e-306,1e-307,1e-308
	};

	inline double x_fast_path(double significand, intptr_t exp)
	{
		if (exp < -308)
			return -INFINITY;
		else if (exp >= 0)
			return significand * DOUBLE_E[exp];
		else
			return significand * DOUBLE_NE[-exp];
	}

	template<class _CharType>
	std::tuple<number_value, parser_result> simd_double_parser2(const _CharType* psz)
	{
		typedef x_convert_char_selector<sizeof(_CharType)> char_selector;

		//�ȴ���������
		bool minus = *psz == '-';
		if (minus) ++psz;
		else if (*psz == '+') ++psz;

		intptr_t exp = 0;

		double dval;
		bool useDouble = false;	//��ʼû����������������ˣ�����Ҫʹ�ø������㷨
		int64_t i64 = char_selector::template convert_long(0, psz, useDouble);
		if (useDouble)
		{//���������ʹ�ø������㷨
			dval = (double)i64;

			const _CharType* const pszSaved = psz;
			for (; x_is_digit(*psz); ++psz);
			exp = psz - pszSaved;
		}

		if (*psz == '.')
		{//����С������
			++psz;
			const _CharType* pszDot = psz;

			if (!useDouble)
			{//��δ���������С������������
				i64 = char_selector::template convert_long(i64, psz, useDouble);
				dval = (double)i64;
			}

			if (useDouble)
			{
				exp += pszDot - psz;	//����ĸ�������ָ��

				//�Ѿ��ִﾫ�����ޣ������ַ����ٷ���
				for (; x_is_digit(*psz); ++psz);
			}
			else
			{
				exp += pszDot - psz;
				useDouble = true;
			}
		}

		if (*psz && (*psz | 32) == 'e')
		{
			if (!useDouble)
			{
				dval = (double)i64;
				useDouble = true;
			}
			++psz;

			bool expMinus = false;
			if (*psz && *psz == '+')
			{
				++psz;
			}
			else if (*psz && *psz == '-')
			{
				++psz;
				expMinus = true;
			}

			bool overflow = false;	//��ʼû����������������ˣ�����Ҫʹ�ø������㷨
			int64_t e2 = char_selector::template convert_long(0, psz, overflow);

			if (overflow || e2 > ((std::numeric_limits<int32_t>::max)() / 2))
			{
				number_value nv = { 0 };
				return { nv, parser_result::Invalid };
			}

			if (expMinus)
				exp -= e2;
			else
				exp += e2;
		}

		if (useDouble)
		{
			if (exp < -330)	//324 = 308 + 22
			{
				dval = -INFINITY;
			}
			else if (exp < -308)
			{
				dval = x_fast_path(dval, -308);
				dval = x_fast_path(dval, exp + 308);
			}
			else if (exp > 330)
			{
				dval = INFINITY;
			}
			else if (exp > 308)
			{
				dval = x_fast_path(dval, 308);
				dval = x_fast_path(dval, exp - 308);
			}
			else if (exp != 0)
			{
				dval = x_fast_path(dval, exp);
			}

			number_value nv;
			nv.d = minus ? -dval : dval;
			return { nv, parser_result::Double };
		}
		else
		{
			number_value nv;
			nv.l = minus ? -i64 : i64;
			return { nv, parser_result::Long };
		}
	}
}

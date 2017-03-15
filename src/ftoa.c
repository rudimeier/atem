/*** ftoa.c -- fast float to text converters
 *
 * Copyright (C) 2010-2017 Ruediger Meier
 *
 * Author:  Ruediger Meier <sweet_f_a@gmx.de>
 *
 * This file is part of atem.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the author nor the names of any contributors
 *    may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ***/

#include <string.h>
#include <limits.h>



#define PRECISION 5
#define DO_ROUNDING
// #define NO_TRAIL_NULL
/* PRECISION_IS_SIGNIFICANT whithout NO_TRAIL_NULL is not supported */
// #define PRECISION_IS_SIGNIFICANT



typedef union {
	int L;
	float F;
} LF_t;


int ftoa( char *outbuf, float f )
{
	uint64_t mantissa, int_part, frac_part;
	int safe_shift;
	uint64_t safe_mask;
	short exp2;
	LF_t x;
	char *p;

	x.F = f;
	p = outbuf;

	exp2 = (unsigned char)(x.L >> 23) - 127;
	mantissa = (x.L & 0xFFFFFF) | 0x800000;
	frac_part = 0;
	int_part = 0;

	if( x.L < 0  ) {
		*p++ = '-';
	}

	/* Our algorithm works only on exponents >= -36 because safe_mask must
	   start with at least 4 zero bits. So we quickly print 0.0 here. (We could
	   do this even for bigger exponents dependently on PRECISION but would be
	   a useless optimization.) BTW the case f == 0.0 is also handled here. */
	if ( exp2 < -36 ) {
#if defined NO_TRAIL_NULL
		*p++ = '0';
#else
		// print 0.000... like "%._f" does
		memset( p, '0', PRECISION + 1 + 1);
		p[1] = '.';
		p += PRECISION + 1 + 1;
#endif
		goto END;
	}

	safe_shift = -(exp2 + 1);
	safe_mask = 0xFFFFFFFFFFFFFFFFULL >>(64 - 24 - safe_shift);

	if (exp2 >= 64) {
		/* |f| >= 2^64 > ULONG_MAX */
		/* NaNs and +-INF are also handled here*/
		int_part = ULONG_MAX;
	} else if (exp2 >= 23) {
		int_part = mantissa << (exp2 - 23);
	} else if (exp2 >= 0) {
		int_part = mantissa >> (23 - exp2);
		frac_part = (mantissa) & safe_mask;
	} else /* if (exp2 < 0) */ {
		frac_part = (mantissa & 0xFFFFFF);
	}

	if (int_part == 0) {
		*p++ = '0';
	} else {
		p += itoa_uint64(p, int_part);
	}

	if (frac_part != 0) {
		*p++ = '.';

		/* print BCD, calculating digits of frac_part (one more digit is needed
		   when rounding, less digits are needed when then precision should be
		   significant*/
		char max = PRECISION;
#ifdef PRECISION_IS_SIGNIFICANT
		int cnt_dig = ((p - outbuf) + (x.L < 0 ? 1 : 0) - 1);
		max -= (cnt_dig < max) ? cnt_dig : 0;
#endif
#if defined DO_ROUNDING
		max++;
#endif
		for (char m = 0; m < max; m++) {
			/* frac_part *= 10; */
			frac_part = (frac_part << 3) + (frac_part << 1);

			*p++ = (frac_part >> (24 + safe_shift)) + '0';
			frac_part &= safe_mask;
		}
#if defined DO_ROUNDING

		int round_int = 0;
		p--;

		if(  *p >= '5' ) {
			char *w = p-1;
			if( *w != '.' ) {
				if( frac_part != 0 || (*w & 1) ) {
					// round up
					while( *w == '9' ) {
						*w-- = '0';
					}
					if( *w != '.' ) {
						*w += 1;
					} else {
						round_int = 1;
					}
				}
			} else {
				if( frac_part != 0 || ( (int_part & 1)) ) {
					round_int = 1;
				}
			}

			if( round_int ) {
				// we have to round up int_part too
				w--;
				while( *w == '9' && w >= outbuf && *w != '-' ) {
					*w-- = '0';
				}
				if( w >= outbuf &&  *w != '-' ) {
					*w += 1;
				} else {
					// int_part has one digit more now
					w++;
					p++;
					memmove( w+1, w, p-w  );
					*w = '1';
				}
			}
		}
#endif
#if defined NO_TRAIL_NULL
		for (; p[-1] == '0'; --p) {
		}
		if( p[-1] == '.' ) {
			p--;
		}
#else
	} else {
		// print _.000... like "%._f" does
		*p++ = '.';
		memset( p, '0', PRECISION );
		p += PRECISION;
#endif
	} /*  if (frac_part != 0) */

END:
	*p = 0;
	return p - outbuf;
}

#undef PRECISION




/**
 * ftoa_prec_f0: printing floats as rounded integer
 * works exactly like sprintf "%.0f" for |f| < 2^64,
 * +-ULONG_MAX is printed when |f| >= 2^64 or +-inf or nan,
 * round to even like IEEE 754-1985 4.1 says,
 * about 20 times faster than sprintf ( exponent range 2^-1, 2^20 )
 */
int ftoa_prec_f0( char *outbuf, float f )
{
	char *p = outbuf;
	LF_t x;
	x.F = f;

	short exp2 = (unsigned char)(x.L >> 23) - 127;

	if( x.L < 0  ) {
		*p++ = '-';
	}

	if( exp2 < -1 ) {
		/*  |f| <= 0.5  */
		*p++ = '0';
	} else {
		uint64_t int_part;
		uint64_t mantissa = (x.L & 0xFFFFFF) | 0x800000;
		if (exp2 >= 64) {
			/* |f| >= 2^64 > ULONG_MAX */
			/* NaNs and +-INF are also handled here*/
			int_part = ULONG_MAX;
		} else if (exp2 >= 23) {
			int_part = mantissa << (exp2 - 23);
			/* frac_part is 0 */
		} else {
			unsigned int frac_part;
			int_part = mantissa >> (23 - exp2);
			frac_part = (mantissa << (exp2 + 1)) & 0xFFFFFF;
			if( frac_part >= 0x800000 ) {
				/* ieee - *.5 round to even */
				if( frac_part != 0x800000 || ( int_part & 1 ) ) {
					int_part++;
				}
			}
		}

		p += itoa_uint64(p, int_part);
	}
	*p = 0;
	return p - outbuf;
}

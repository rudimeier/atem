#include "util.h"

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <limits.h>

#include "itoa.c"





int itodatestr( char *s, unsigned int n )
{
	s[9] = (n % 10) + '0';
	n /= 10;
	s[8] = (n % 10) + '0';
	n /= 10;
	s[7] = '-';
	s[6] = (n % 10) + '0';
	n /= 10;
	s[5] = (n % 10) + '0';
	n /= 10;
	s[4] = '-';
	s[3] = (n % 10) + '0';
	n /= 10;
	s[2] = (n % 10) + '0';
	n /= 10;
	s[1] = (n % 10) + '0';
	n /= 10;
	s[0] = (n % 10) + '0';
	return 10;
}


int itotimestr( char *s, unsigned int n )
{
	s[7] = (n % 10) + '0';
	n /= 10;
	s[6] = (n % 10) + '0';
	n /= 10;
	s[5] = ':';
	s[4] = (n % 10) + '0';
	n /= 10;
	s[3] = (n % 10) + '0';
	n /= 10;
	s[2] = ':';
	s[1] = (n % 10) + '0';
	n /= 10;
	s[0] = (n % 10) + '0';
	return 8;
}





#define PRECISION 6
#define DO_ROUNDING
#define NO_TRAIL_NULL
/* PRECISION_IS_SIGNIFICANT whithout NO_TRAIL_NULL is not supported */
#define PRECISION_IS_SIGNIFICANT


#if defined DO_ROUNDING
	#define _INC_PREC_ROUND_ 1
#else
	#define _INC_PREC_ROUND_ 0
#endif

#if defined PRECISION_IS_SIGNIFICANT
	#define _INC_PREC_SIGNIFICANT_ ((p - outbuf) + (x.L < 0 ? 1 : 0) - 1)
#else
	#define _INC_PREC_SIGNIFICANT_ 0
#endif


typedef union {
	int L;
	float F;
} LF_t;


int ftoa( char *outbuf, float f )
{
	unsigned long mantissa, int_part, frac_part;
	int safe_shift;
	unsigned long safe_mask;
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
	safe_mask = 0xFFFFFFFFFFFFFFFF >>(64 - 24 - safe_shift);
	
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
		const char max = PRECISION - _INC_PREC_SIGNIFICANT_ + _INC_PREC_ROUND_;
		for (char m = 0; m < max; m++) {
			/* frac_part *= 10; */
			frac_part = (frac_part << 3) + (frac_part << 1); 
			
			*p++ = (frac_part >> (24 + safe_shift)) + '0';
			frac_part &= safe_mask;
		}
#if defined DO_ROUNDING
		// rounding, works only for PRECISION > 1
		p--;
		if(  *p >= '5' ) {
			char *w = p-1;
			if( frac_part != 0 || (*w & 1) ) {
				// round up
				while( *w == '9' ) {
					*w-- = '0';
				}
				if( *w != '.' ) {
					*w += 1;
				} else {
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
#undef _INC_PREC_ROUND_
#undef _INC_PREC_SIGNIFICANT_




int ftoa_prec_f0( char *outbuf, float f )
{
	unsigned long mantissa, int_part, frac_part;
	int safe_shift;
	unsigned long safe_mask;
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
	
	if (x.F == 0.0 || exp2 < -23 ) {
		*p++ = '0';
		goto END;
	} else if (exp2 >= 31) {
		/* |f| >= 2^31 > INT_MAX */
		*p++ = 'i';
		*p++ = 'n';
		*p++ = 'f';
		goto END;
	}
	
	safe_shift = -(exp2 + 1);
	safe_mask = 0xFFFFFFFFFFFFFFFF >>(64 - 24 - safe_shift);
	
	if (exp2 >= 23) {
		int_part = mantissa << (exp2 - 23);
	} else if (exp2 >= 0) {
		int_part = mantissa >> (23 - exp2);
		frac_part = (mantissa) & safe_mask;
	} else /* if (exp2 < 0) */ {
		frac_part = (mantissa & 0xFFFFFF);
	}
	
	if (frac_part != 0) {
		/* frac_part *= 10; */
		frac_part = (frac_part << 3) + (frac_part << 1);
		char c = (frac_part >> (24 + safe_shift)) + '0';
		frac_part &= safe_mask;
		if( c >= '5' ) {
			if( frac_part != 0 || (int_part & 1) ) {
				int_part++;
			}
		}
	}
	
	if (int_part == 0) {
		*p++ = '0';
	} else {
		p += itoa(p, int_part);
	}
	
END:
	*p = 0;
	return p - outbuf;
}


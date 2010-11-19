#include "util.h"

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

/**
 * reverse:  reverse first l chars of string s in place
 */
void reverse( char *s, int l )
{
	int i, j;
	char c;
	
	for (i = 0, j = l-1; i<j; i++, j--) {
		c = s[i];
		s[i] = s[j];
		s[j] = c;
	}
}


int ltoa( char *s, long n )
{
	char *begin = s;
	char *rev = s;
	
	if( n < 0 ) {
		 // forget about the sign, don't reverse it later
		*s++ = '-';
		rev++;
	} else {
		// make n negative, this way because |LONG_MIN| > |LONG_MAX|
		n = -n;
	}
	
	do {       /* generate digits in reverse order */
		*s++ = -(n % 10) + '0';   /* get next digit */
	} while ((n /= 10) < 0);     /* delete it */
	
	reverse( rev, s - rev);
	
	return s - begin;
}


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





//---------------------------------------------------------------
// itoas() - iMalc version updated ver. 0.8
//---------------------------------------------------------------
int itoa( char *s, int snum )
{
	char *ps = s;
	unsigned int num1 = snum, num2, num3, div;
	if (snum < 0) {
		*ps++ = '-';
		num1 = -snum;
	}
	if (num1 < 10000) {
		if (num1 < 10) goto L1;
		if (num1 < 100) goto L2;
		if (num1 < 1000) goto L3;
	} else {
		num2 = num1 / 10000;
		num1 -= num2 * 10000;
		if (num2 < 10000) {
			if (num2 < 10) goto L5;
			if (num2 < 100) goto L6;
			if (num2 < 1000) goto L7;
		} else {
			num3 = num2 / 10000;
			num2 -= num3 * 10000;
			if (num3 >= 10) {
				*ps++ = '0' + (char)(div = (num3*6554UL)>>16);
				num3 -= div*10;
			}
			*ps++ = '0' + (char)(num3);
		}
		*ps++ = '0' + (char)(div = (num2*8389UL)>>23);
		num2 -= div*1000;
L7:
		*ps++ = '0' + (char)(div = (num2*5243UL)>>19);
		num2 -= div*100;
L6:
		*ps++ = '0' + (char)(div = (num2*6554UL)>>16);
		num2 -= div*10;
L5:
		*ps++ = '0' + (char)(num2);
    }
	*ps++ = '0' + (char)(div = (num1*8389UL)>>23);
	num1 -= div*1000;
L3:
	*ps++ = '0' + (char)(div = (num1*5243UL)>>19);
	num1 -= div*100;
L2:
	*ps++ = '0' + (char)(div = (num1*6554UL)>>16);
	num1 -= div*10;
L1:
	*ps++ = '0' + (char)(num1);
	
	return ps - s;
}




int ltoa2( char *s, long snum )
{
	char *ps = s;
	unsigned long num1 = snum, num2, num3, num4, num5, div;
	
	if (snum < 0) {
		*ps++ = '-';
		num1 = -snum;
	}
	
	if (num1 < 10000) {
		if (num1 < 10) goto L1;
		if (num1 < 100) goto L2;
		if (num1 < 1000) goto L3;
	} else {
		num2 = num1 / 10000;
		num1 -= num2 * 10000;
		if (num2 < 10000) {
			if (num2 < 10) goto L5;
			if (num2 < 100) goto L6;
			if (num2 < 1000) goto L7;
		} else {
			num3 = num2 / 10000;
			num2 -= num3 * 10000;
			if (num3 < 10000) {
				if (num3 < 10) goto L9;
				if (num3 < 100) goto L10;
				if (num3 < 1000) goto L11;
			} else {
				num4 = num3 / 10000;
				num3 -= num4 * 10000;
				if (num4 < 10000) {
					if (num4 < 10) goto L13;
					if (num4 < 100) goto L14;
					if (num4 < 1000) goto L15;
				} else {
					num5 = num4 / 10000;
					num4 -= num5 * 10000;
					if (num5 < 10000) {
						if (num5 < 10) goto L17;
						if (num5 < 100) goto L18;
#if defined FOR_UNSIGNED_LONGINT
						if (num5 < 1000) goto L19;
					}
					
					*ps++ = '0' + (char)(div = (num5*8389UL)>>23);
					num5 -= div*1000;
L19:
#else
					}
#endif
					*ps++ = '0' + (char)(div = (num5*5243UL)>>19);
					num5 -= div*100;
L18:
					*ps++ = '0' + (char)(div = (num5*6554UL)>>16);
					num5 -= div*10;
L17:
					*ps++ = '0' + (char)(num5);
				}
				*ps++ = '0' + (char)(div = (num4*8389UL)>>23);
				num4 -= div*1000;
L15:
				*ps++ = '0' + (char)(div = (num4*5243UL)>>19);
				num4 -= div*100;
L14:
				*ps++ = '0' + (char)(div = (num4*6554UL)>>16);
				num4 -= div*10;
L13:
				*ps++ = '0' + (char)(num4);
			}
			*ps++ = '0' + (char)(div = (num3*8389UL)>>23);
			num3 -= div*1000;
L11:
			*ps++ = '0' + (char)(div = (num3*5243UL)>>19);
			num3 -= div*100;
L10:
			*ps++ = '0' + (char)(div = (num3*6554UL)>>16);
			num3 -= div*10;
L9:
			*ps++ = '0' + (char)(num3);
		}
		*ps++ = '0' + (char)(div = (num2*8389UL)>>23);
		num2 -= div*1000;
L7:
		*ps++ = '0' + (char)(div = (num2*5243UL)>>19);
		num2 -= div*100;
L6:
		*ps++ = '0' + (char)(div = (num2*6554UL)>>16);
		num2 -= div*10;
L5:
		*ps++ = '0' + (char)(num2);
    }
	*ps++ = '0' + (char)(div = (num1*8389UL)>>23);
	num1 -= div*1000;
L3:
	*ps++ = '0' + (char)(div = (num1*5243UL)>>19);
	num1 -= div*100;
L2:
	*ps++ = '0' + (char)(div = (num1*6554UL)>>16);
	num1 -= div*10;
L1:
	*ps++ = '0' + (char)(num1);
	
	return ps - s;

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
	
	if (x.F == 0.0 || exp2 < -23 ) {
#if defined NO_TRAIL_NULL
		*p++ = '0';
#else
		// print 0.000... like "%._f" does
		memset( p, '0', PRECISION + 1 + 1);
		p[1] = '.';
		p += PRECISION + 1 + 1;
#endif
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
	
	if (int_part == 0) {
		*p++ = '0';
	} else {
		p += itoa(p, int_part);
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


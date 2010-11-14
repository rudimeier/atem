#include "util.h"

#include <math.h>

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


int ftoa( char *s, float f )
{
	char *buf = s;
	if( f < 0 ) {
		f = -f;
		*buf++ = '-';
	}
	
	float a;
	float b = modff(f, &a );
	
	int b_ = (b * 1000000);
	if( b_ % 10 >= 5 ) {
		b_ = b_ / 10 + 1;
	} else {
		b_ /= 10;
	}
	buf += ltoa( buf, a );
	int d = ltoa( buf, 100000 + b_ );
	*buf = '.';
	return buf + d - s;
}





#if ! defined I_MALC_ATOI
struct bla
{
	unsigned int div;
	unsigned int rem;
};

// seems it's slower when we inline it manually
static inline struct bla div1000(unsigned int a, unsigned int b)
{
	register unsigned int c = a / b;
	register unsigned int d = a % b;
	struct bla ret = {c ,d };
	return ret;
}

#define MOD_500( _num, _tens ) \
	l2 = div1000( _num, _tens ); \
	*ps++ = '0' + (char)l2.div; \
	_num = l2.rem 


int itoa( char *s, long snum )
{
	struct bla l2;
	char *ps = s;
	unsigned long num1 = snum, num2, num3;
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
				MOD_500( num3, 10);
			}
			*ps++ = '0' + (char)(num3);
		}
		MOD_500( num2, 1000);
L7:
		MOD_500( num2, 100);
L6:
		MOD_500( num2, 10);
L5:
		*ps++ = '0' + (char)(num2);
    }
	
	MOD_500( num1, 1000);
L3:
	MOD_500( num1, 100);
L2:
	MOD_500( num1, 10);
	
L1:
	*ps++ = '0' + (char)(num1);
	*ps = '\0';
	return ps - s;
}

#else

//---------------------------------------------------------------
// itoas() - iMalc version updated ver. 0.8
//---------------------------------------------------------------
int itoa( char *s, long snum )
{
	char *ps = s;
	unsigned long num1 = snum, num2, num3, div;
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
	*ps = '\0';
	return ps - s;
}


#endif
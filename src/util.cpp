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


/**
 * ltoa:  convert n to characters in s
 */
int ltoa( long n, char *s )
{
	char *begin = s;
	char *rev = s;
	
	if( n < 0 ) {
		 // forget about the sign, don't reverse it later
		*s++ = '-';
		*s++ = -(n % 10) + '0';
		n /= -10;
		rev++;
	}
	
	do {       /* generate digits in reverse order */
		*s++ = (n % 10) + '0';   /* get next digit */
	} while ((n /= 10) > 0);     /* delete it */
	
	reverse( rev, s - rev);
	
	return s - begin;
}


int ftoa(float f, char *s)
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
	buf += ltoa(a, buf);
	int d = ltoa( 100000 + b_, buf);
	*buf = '.';
	return buf + d - s;
}

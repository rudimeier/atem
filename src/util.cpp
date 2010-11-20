#include "util.h"

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <limits.h>

#include "itoa.c"

#if defined FAST_PRINTING
	#include "ftoa.c"
#else

int ftoa(char *s, float f )
{
	return sprintf( s, "%.5f", f );
}
int ftoa_prec_f0(char *s, float f )
{
	return sprintf( s, "%.0f", f );
}

#endif




int itodatestr( char *s, unsigned int n )
{
#if defined FAST_PRINTING
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
#else
	return sprintf( s, "%d", n );
#endif
}


int itotimestr( char *s, unsigned int n )
{
#if defined FAST_PRINTING
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
#else
	return sprintf( s, "%d", n );
#endif
}




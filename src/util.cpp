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
#else
	sprintf( s, "%08u",
		(n % 100000000) );
	s[9] = s[7];
	s[8] = s[6];
	s[7] = '-';
	s[6] = s[5];
	s[5] = s[4];
	s[4] = '-';
#endif
	return 10;
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
#else
	sprintf( s, "%06u",
		(n % 1000000) );
	s[7] = s[5];
	s[6] = s[4];
	s[5] = ':';
	s[4] = s[3];
	s[3] = s[2];
	s[2] = ':';
#endif
	return 8;
}




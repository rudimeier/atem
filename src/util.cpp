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
	if( n <= 0 || n >= 1000000 ) {
		memcpy(s, "00:00:00", 8);
		return 8;
	}
	
#if defined FAST_PRINTING
	uint32_t num2, div;
	
	num2 = n / 1000;
	n -= num2 * 1000;
	
	s[0] = '0' + (char)(div = (num2*5243)>>19);
	num2 -= div*100;
	
	s[1] = '0' + (char)(div = (num2*6554)>>16);
	num2 -= div*10;
	
	s[2] = ':';
	
	s[3] = '0' + (char)(num2);
	s[4] = '0' + (char)(div = (n*5243)>>19);
	n -= div*100;
	
	s[5] = ':';
	
	s[6] = '0' + (char)(div = (n*6554)>>16);
	n -= div*10;
	s[7] = '0' + (char)(n);
#else
	sprintf( s, "%06u", n );
	s[7] = s[5];
	s[6] = s[4];
	s[5] = ':';
	s[4] = s[3];
	s[3] = s[2];
	s[2] = ':';
#endif
	return 8;
}




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
#if 0
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
	char *ps = s;
	uint32_t num1 = n, num2, div;
	if ( n >= 1000000) {
		assert(0);
	}
	
	
	if (num1 < 1000) {
		if (num1 < 10) { strcpy(ps, "00:00:0"); ps+=7; goto L1;}
		if (num1 < 100) { strcpy(ps, "00:00:"); ps+=6; goto L2;}
		strcpy(ps, "00:0"); ps+=4;
	} else {
		num2 = (num1*8389UL)>>23;
		num1 -= num2 * 1000;
		if (num2 < 10000) {
			if (num1 < 10) { strcpy(ps, "00:"); ps+=3; goto L4;}
			if (num2 < 100) { strcpy(ps, "0");ps+=1;  goto L5;}
		}
		*ps++ = '0' + (char)(div = (num2*5243UL)>>19);
		num2 -= div*100;
L5:
		*ps++ = '0' + (char)(div = (num2*6554UL)>>16);
		num2 -= div*10;
		*ps++ = ':';
L4:
		*ps++ = '0' + (char)(num2);
	}
	*ps++ = '0' + (char)(div = (num1*5243UL)>>19);
	num1 -= div*100;
	*ps++ = ':';
L2:
	*ps++ = '0' + (char)(div = (num1*6554UL)>>16);
	num1 -= div*10;
L1:
	*ps++ = '0' + (char)(num1);
	
	return ps - s;
#endif
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




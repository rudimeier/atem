#include "util.h"

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <limits.h>

#include "itoa.c"
#include "ftoa.c"





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




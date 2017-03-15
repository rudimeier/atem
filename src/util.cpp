/*** util.cpp -- common utilities
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

#include "util.h"

#include <string.h>

#include "config.h"


#if defined FAST_PRINTING
	#define itoa_int32 itoa
	#define itoa_int64 ltoa
	#include "itoa.c"
	#include "ftoa.c"
#else
	#include <stdio.h>

int itoa( char *s, int n )
{
	return sprintf( s, "%d", n );
}
int ltoa( char *s, long n )
{
	return sprintf( s, "%ld", n );
}

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
	if( n <= 0 || n >= 100000000 ) {
		memcpy(s, "0000-00-00", 10);
		return 10;
	}

#if defined FAST_PRINTING
	uint32_t num1 = n, num2, div;

	num2 = num1 / 10000;
	num1 -= num2 * 10000;

	s[0] = '0' + (char)(div = (num2*8389)>>23);
	num2 -= div*1000;
	s[1] = '0' + (char)(div = (num2*5243)>>19);
	num2 -= div*100;
	s[2] = '0' + (char)(div = (num2*6554)>>16);
	num2 -= div*10;
	s[3] = '0' + (char)(num2);
	s[4] = '-';

	s[5] = '0' + (char)(div = (num1*8389)>>23);
	num1 -= div*1000;
	s[6] = '0' + (char)(div = (num1*5243)>>19);
	num1 -= div*100;
	s[7] = '-';
	s[8] = '0' + (char)(div = (num1*6554)>>16);
	num1 -= div*10;
	s[9] = '0' + (char)(num1);
#else
	sprintf( s, "%08u", n );
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

/*** util.h -- common utilities
 *
 * Copyright (C) 2010-2012 Ruediger Meier
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

#ifndef ATEM_UTILS_H
#define ATEM_UTILS_H




inline unsigned char count_bits( unsigned int fields )
{
	unsigned int c = 0;
	for (c = 0; fields; fields >>= 1) {
		c += fields & 1;
	}
	return c;
}

/**
 * copy src to dst string without trailing spaces,
 * src does not need to be zero terminated, dst will be zero terminated so size of
 * dst must be len + 1
 */
inline int trim_end( char *dst, const char *src, int len )
{
	int last = -1;
	for( int i=0; i<len && src[i]!='\0'; i++  ) {
		dst[i] = src[i];
		if( src[i] != ' ' ) {
			last = i;
		}
	}
	dst[last+1] = '\0';
	return last+1;
}



extern int itoa( char *s, int n );
extern int ltoa( char *s, long n );

extern int itodatestr( char *s, unsigned int n );
extern int itotimestr( char *s, unsigned int n );

extern int ftoa(char *s, float f );
extern int ftoa_prec_f0(char *s, float f );




#endif

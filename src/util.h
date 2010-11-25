#ifndef ATEM_UTILS_H
#define ATEM_UTILS_H

#include "config.h"



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


#if defined FAST_PRINTING
	#define itoa itoa_int32
	#define ltoa itoa_int64
#endif

extern int itoa( char *s, int n );
extern int ltoa( char *s, long n );

extern int itodatestr( char *s, unsigned int n );
extern int itotimestr( char *s, unsigned int n );

extern int ftoa(char *s, float f );
extern int ftoa_prec_f0(char *s, float f );




#endif

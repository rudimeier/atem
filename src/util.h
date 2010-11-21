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


#define itoa itoa_int32
extern int itoa( char *s, int n );

#define ltoa itoa_int64
extern int ltoa( char *s, long n );

extern int ltoa_simple( char *s, long n );

extern int itodatestr( char *s, unsigned int n );

extern int itotimestr( char *s, unsigned int n );


extern int ftoa(char *s, float f );
extern int ftoa_prec_f0(char *s, float f );




#endif

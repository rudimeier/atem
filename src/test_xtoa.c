#include "util.h"
#include "util.cpp"

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <float.h>


#define BUF_LEN 512


// #define TEST_ITOA
#define TEST_FTOA



static struct rudi_printf_info pinfo;

void init_print_info()
{
	pinfo.prec = 20;
	pinfo.width = 0;
	pinfo.spec = 'f';
	pinfo.space = 0;
	pinfo.left = 0;
	pinfo.showsign = 0;
	pinfo.pad = ' ';
}




int compare2()
{
	float f_start = 0.0000001f;
	unsigned int start = *( (unsigned int*)  &(f_start));
	unsigned int end = INT_MAX;
	unsigned int i;
	for( i = start; i <= end; i +=1 ) {
		char s1[512];
		char s2[512];
		
		double f = *( (float*) (&i) );
		
		sprintf( s1, "%.6g", f );
		rudi_printf_fp( s2, &pinfo, f);
		
		if( strcmp(s1, s2/*, l - 1*/) ) {
			fprintf( stdout, "error(%d): %.18g  %s != %s\n", i, f, s1, s2 );
// 			return 1;
		}
		if( (i % 10000000) == 0 ) {
			fprintf( stdout, "#%d: %s, %s\n", i, s1, s2 );
		}
	}
	return 0;
}


int bench2( unsigned int mant1, unsigned int mant2,
	unsigned int expnt1, unsigned int expnt2 )
{
	typedef union {
		unsigned int I;
		float F;
	} funi;
	
	char buf[BUF_LEN];
	for( int j= 0; j<BUF_LEN; j++ ) {
		buf[j] = 'a';
	}
	
	for( unsigned int e = expnt1; e <= expnt2; e++ ) {
		printf("EXPNT %u\n", e);
		unsigned int float_I = e << 23;
// 		float_I |= 0x80000000; //sign
		for( unsigned int i=mant1; i<=mant2; i++ ) {
			char *s = buf;
			unsigned int float_i = float_I | i;
			float f = *( (float*) (&float_i) );
			
#if 0
			s += ftoa2(s, f);
			*s++ = '\n';
#else
			s += rudi_printf_fp(s, &pinfo, f);
			*s++ = '\n';
			
#endif
			*s = '\0';
			
			fputs( buf, stdout);
		}
	}
	return 0;
}




#define CMP_FUNC_1 glibc_ltoa
#define CMP_FUNC_2 ltoa
#define BENCH_FUNC CMP_FUNC_2



int glibc_itoa( char *s, int n )
{
	int len = sprintf( s, "%d", n );
	return len;
}

int glibc_ltoa( char *s, long n )
{
	int len = sprintf( s, "%ld", n );
	return len;
}


int compare( long start, long end )
{
	char s1[BUF_LEN];
	char s2[BUF_LEN];
	
	int j;
	for( j= 0; j<BUF_LEN; j++ ) {
		s1[j] = s2[j] = 'a';
	}
	
	long i = start;
	do {
		
		int len1 = CMP_FUNC_1( s1, i );
		int len2 = CMP_FUNC_2( s2, i );
		
		if( len1 != len2 || strncmp(s1, s2, len1 ) ) {
			s1[len1] = s2[len2] = '\0';
			fprintf( stderr, "error: _%s_(%d) != _%s_(%d)\n", s1, len1, s2, len2 );
			return 1;
		}
		if( i % 0xfffff == 0) {
			s1[len1] = s2[len2] = '\0';
			fprintf( stdout, "%lX, %s\n", i, s1 );
		}
		if( i == end ) {
			break;
		}
		i++;
	} while(1);
	
	return 0;
}


int bench( long start, long end )
{
	char s2[BUF_LEN];
	
	int j;
	for( j= 0; j<BUF_LEN; j++ ) {
		s2[j] = 'a';
	}
	
	long i = start;
	do{
		int len2 = BENCH_FUNC( s2, i );
		s2[len2] = '\n';
		s2[len2+1] = '\0';
		
		fputs( s2, stdout);
		// write seems so be slower here
		// write( 1, s2, len2+1 );
		if( i == end ) {
			break;
		}
		i++;
	} while(1);
	
	return 0;
}


#if defined TEST_ITOA

int main(int argc, char *argv[])
{
	long start = - (long)UINT_MAX;
	long end   = UINT_MAX;
	printf("test from %ld to %ld\n", start, end);
	
	return compare( start, end );
}

#endif


#if defined TEST_FTOA

int main(int argc, char *argv[])
{
	init_print_info();
	
	bench2(0 , 0x7fffff, 127-4, 127+13 );
	return 0;
}

#endif

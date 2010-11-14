#include "util.h"
#include "util.cpp"

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <float.h>




static struct rudi_printf_info pinfo;

void init_print_info()
{
	pinfo.prec = 6;
	pinfo.width = 0;
	pinfo.spec = 'g';
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



#define CMP_FUNC_1 glibc_ltoa
#define CMP_FUNC_2 ltoa
#define BENCH_FUNC CMP_FUNC_2

#define BUF_LEN 32


int glibc_itoa( char *s, long n )
{
	int len = sprintf( s, "%d", n );
	return len;
}

int glibc_ltoa( char *s, long n )
{
	int len = sprintf( s, "%ld", n );
	return len;
}


int compare( long start, long count )
{
	long end = start + count;
	
	char s1[BUF_LEN];
	char s2[BUF_LEN];
	
	int j;
	for( j= 0; j<BUF_LEN; j++ ) {
		s1[j] = s2[j] = 'a';
	}
	
	long i;
	for( i = start; i<end; i++ ) {
		
		int len1 = CMP_FUNC_1( s1, i );
		int len2 = CMP_FUNC_2( s2, i );
		
		if( len1 != len2 || strncmp(s1, s2, len1 ) ) {
			s1[len1] = s2[len2] = '\0';
			fprintf( stdout, "error: _%s_(%d) != _%s_(%d)\n", s1, len1, s2, len2 );
			return 1;
		}
		if( i % 0xfffff == 0) {
			s1[len1] = s2[len2] = '\0';
			fprintf( stdout, "%lX, %s\n", i, s1 );
		}
	}
	return 0;
}


int bench( long start, long count )
{
	long end = start + count;
	
	char s2[BUF_LEN];
	
	int j;
	for( j= 0; j<BUF_LEN; j++ ) {
		s2[j] = 'a';
	}
	
	long i;
	for( i = start; i<end; i++ ) {
		
		int len2 = BENCH_FUNC( s2, i );
		s2[len2] = '\n';
		s2[len2+1] = '\0';
		
		fputs( s2, stdout);
		// write seems so be slower here
		// write( 1, s2, len2+1 );
	}
	return 0;
}



int main(int argc, char *argv[])
{
	long start = INT_MIN;
	long count = (long)INT_MAX - (long)INT_MIN;
	
	init_print_info();
	return bench( start, count );
// 	double  f = NAN;
// 	char ccc[512];
// 	rudi_printf_fp( ccc, &pinfo, f );
// 	fprintf( stdout, "MY  : %s \n", ccc );
// 	fprintf( stdout, "ORIG: %.6f \n", f );
}

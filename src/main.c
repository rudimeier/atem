#include "util.h"
#include "util.cpp"

#include <stdio.h>
#include <string.h>
#include <limits.h>




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







int compare1()
{
	long start = 9999999999;
	long count = INT_MAX;	
	long end = start + count;
	
	for( long i = start; i<end; i++ ) {
		char s1[32];
		char s2[32];
		
		sprintf( s1, "%ld", i );
		itoa( s2, i );
		
		if( strcmp(s1, s2) ) {
			fprintf( stdout, "error: %s != %s\n", s1, s2 );
			return 1;
		}
		if( i % 0xfffff == 0) {
			fprintf( stdout, "%lX, %s\n", i, s1 );
		}
	}
	return 0;
}



int main(int argc, char *argv[])
{
	init_print_info();
// 	return compare2();
	double  f = NAN;
	char ccc[512];
	rudi_printf_fp( ccc, &pinfo, f );
	fprintf( stdout, "MY  : %s \n", ccc );
	fprintf( stdout, "ORIG: %.6f \n", f );
}

#ifndef ATEM_UTILS_H
#define ATEM_UTILS_H



/**
 * ltoa:  convert n to characters in s
 */
extern int ltoa( long n, char *s );

extern int itoa( long n, char *s );

extern int ftoa(float f, char *s);

#ifdef __cplusplus
extern "C" {
#endif

extern int rudi_printf_fp( char *s, double f );

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif

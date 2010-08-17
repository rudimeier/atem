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

/**
 * taken from <fprintf.h> and modified:
 *    - removed flags: wide, extra, group, i18n, alt (treat them as unset)
 *                     is_short, is_long, is_char, __pad, user (not needed)
 *                     is_long_double (to be handled separately if needed)
 *    - spec must be lowercase char
 *    - spec and pad have type char instead of wchar_t
 */
struct rudi_printf_info
{
	int prec; /* Precision.  */
	int width; /* Width.  */
	char spec; /* Format letter.  */
	unsigned int space:1; /* Space flag.  */
	unsigned int left:1; /* - flag.  */
	unsigned int showsign:1;	/* + flag.  */
	char pad; /* Padding character.  */
};


extern int
#ifdef COMPILE_FOR_LONG_DOUBLE
rudi_printf_fp_long ( char *ccc, const struct rudi_printf_info *info,
	const long double fpnum_ldbl );
#else
rudi_printf_fp ( char *ccc, const struct rudi_printf_info *info,
	const double fpnum_dbl );
#endif




#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif

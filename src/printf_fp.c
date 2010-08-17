/* Floating point output for `printf'.
   Copyright (C) 1995-2003, 2006, 2007, 2008 Free Software Foundation, Inc.

   This file is part of the GNU C Library.
   Written by Ulrich Drepper <drepper@gnu.ai.mit.edu>, 1995.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

/* The gmp headers need some configuration frobs.  */
#define HAVE_ALLOCA 1


/* this avoids including libc-symbols.h */
#define attribute_hidden


#include <alloca.h>
#include <float.h>
#include <gmp-mparam.h>
#include <gmp.h>
#ifdef PRINT_NAN_SIGN
	#include <ieee754.h>
#endif
#include <stdlib/gmp-impl.h>
#include <stdlib/longlong.h>
#include <stdlib/fpioconst.h>
#include <limits.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>



///////////////////////



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
  int prec;			/* Precision.  */
  int width;			/* Width.  */
  char spec;			/* Format letter.  */
  unsigned int space:1;		/* Space flag.  */
  unsigned int left:1;		/* - flag.  */
  unsigned int showsign:1;	/* + flag.  */
  char pad;			/* Padding character.  */
};






#ifndef NDEBUG
# define NDEBUG			/* Undefine this for debugging assertions.  */
#endif
#undef NDEBUG
#include <assert.h>



/* Macros for doing the actual output.  */

#define outchar(ch)							      \
	do {									      \
		*ccc++ = ch;						      \
		done++;								      \
	} while(0)


#define PRINT(ptr, len)						      \
	do {									      \
		memcpy(ccc, ptr, len);				      \
		ccc += len;							      \
		done += len;						      \
	} while(0)


#define PADN(ch, len)						      \
	do {									      \
		char *end = ccc + len;				      \
		while( ccc < end ) {				      \
			*ccc++ = ch;					      \
		}									      \
		done += len;						      \
	} while(0)




/* We use the GNU MP library to handle large numbers.

   An MP variable occupies a varying number of entries in its array.  We keep
   track of this number for efficiency reasons.  Otherwise we would always
   have to process the whole array.  */
#define MPN_VAR(name) mp_limb_t *name; mp_size_t name##size

#define MPN_ASSIGN(dst,src)						      \
  memcpy (dst, src, (dst##size = src##size) * sizeof (mp_limb_t))
#define MPN_GE(u,v) \
  (u##size > v##size || (u##size == v##size && __mpn_cmp (u, v, u##size) >= 0))


#if ! defined COMPILE_FOR_LONG_DOUBLE
extern mp_size_t __mpn_extract_double (mp_ptr res_ptr, mp_size_t size,
				       int *expt, int *is_neg,
				       double value);
#else
extern mp_size_t __mpn_extract_long_double (mp_ptr res_ptr, mp_size_t size,
					    int *expt, int *is_neg,
					    long double value);
#endif



int
#ifdef COMPILE_FOR_LONG_DOUBLE
rudi_printf_fp_long ( char *ccc, const long double fpnum_ldbl )
#else
rudi_printf_fp ( char *ccc, const double fpnum_dbl )
#endif
{
	struct rudi_printf_info _info;
  _info.prec = 6;			/* Precision.  */
  _info.width = 0;			/* Width.  */
  _info.spec = 'g';			/* Format letter.  */
  _info.space = 0;		/* Space flag.  */
  _info.left = 0;		/* - flag.  */
  _info.showsign = 0;	/* + flag.  */
  _info.pad = ' ';			/* Padding character.  */

	struct rudi_printf_info *info = &_info;





	assert( info->spec > 60 /*lower case*/ );


  /* Locale-dependent representation of decimal point.	*/

  /* Figure out the decimal point character.  */
  char decimalwc = '.';

  /* "NaN" or "Inf" for the special cases.  */
  const char *special = NULL;

  /* We need just a few limbs for the input before shifting to the right
     position.	*/
  mp_limb_t fp_input[(LDBL_MANT_DIG + BITS_PER_MP_LIMB - 1) / BITS_PER_MP_LIMB];
  /* We need to shift the contents of fp_input by this amount of bits.	*/
  int to_shift = 0;

  /* The fraction of the floting-point value in question  */
  MPN_VAR(frac);
  /* and the exponent.	*/
  int exponent;
  /* Sign of the exponent.  */
  int expsign = 0;
  /* Sign of float number.  */
  int is_neg = 0;

  /* Scaling factor.  */
  MPN_VAR(scale);

  /* Temporary bignum value.  */
  MPN_VAR(tmp);

  /* Digit which is result of last hack_digit() call.  */
  char digit;

  /* The type of output format that will be used: 'e'/'E' or 'f'.  */
  int type;

  /* Counter for number of written characters.	*/
  int done = 0;

  /* General helper (carry limb).  */
  mp_limb_t cy;

  /* Buffer in which we produce the output.  */
  char *wbuffer = NULL;

  auto char hack_digit (void);

  char hack_digit (void)
    {
      mp_limb_t hi;

      if (expsign != 0 && type == 'f' && exponent-- > 0)
	hi = 0;
      else if (scalesize == 0)
	{
	  hi = frac[fracsize - 1];
	  frac[fracsize - 1] = __mpn_mul_1 (frac, frac, fracsize - 1, 10);
	}
      else
	{
	  if (fracsize < scalesize)
	    hi = 0;
	  else
	    {
	      hi = mpn_divmod (tmp, frac, fracsize, scale, scalesize);
	      tmp[fracsize - scalesize] = hi;
	      hi = tmp[0];

	      fracsize = scalesize;
	      while (fracsize != 0 && frac[fracsize - 1] == 0)
		--fracsize;
	      if (fracsize == 0)
		{
		  /* We're not prepared for an mpn variable with zero
		     limbs.  */
		  fracsize = 1;
		  return '0' + hi;
		}
	    }

	  mp_limb_t _cy = __mpn_mul_1 (frac, frac, fracsize, 10);
	  if (_cy != 0)
	    frac[fracsize++] = _cy;
	}

      return '0' + hi;
    }


  /* Fetch the argument value.	*/
#ifdef COMPILE_FOR_LONG_DOUBLE
    {
      /* Check for special values: not a number or infinity.  */
      if (__isnanl (fpnum_ldbl))
	{
#ifdef PRINT_NAN_SIGN
	  union ieee854_long_double u = { .d = fpnum_ldbl };
	  is_neg = u.ieee.negative != 0;
#else
	  is_neg = 0;
#endif
	  special = "nan";
	}
      else if (__isinfl (fpnum_ldbl))
	{
	  is_neg = fpnum_ldbl < 0;
	  special = "inf";
	}
      else
	{
	  fracsize = __mpn_extract_long_double (fp_input,
						(sizeof (fp_input) /
						 sizeof (fp_input[0])),
						&exponent, &is_neg,
						fpnum_ldbl);
	  to_shift = 1 + fracsize * BITS_PER_MP_LIMB - LDBL_MANT_DIG;
	}
    }
#else	/* no long double */
    {
      /* Check for special values: not a number or infinity.  */
      if (__isnan (fpnum_dbl))
	{
#ifdef PRINT_NAN_SIGN
	  union ieee754_double u = { .d = fpnum_dbl };
	  is_neg = u.ieee.negative != 0;
#else
	  is_neg = 0;
#endif
	  special = "nan";
	}
      else if (__isinf (fpnum_dbl))
	{
	  is_neg = fpnum_dbl < 0;
	  special = "inf";
	}
      else
	{
	  fracsize = __mpn_extract_double (fp_input,
					   (sizeof (fp_input)
					    / sizeof (fp_input[0])),
					   &exponent, &is_neg, fpnum_dbl);
	  to_shift = 1 + fracsize * BITS_PER_MP_LIMB - DBL_MANT_DIG;
	}
    }
#endif

  if (special)
    {
      int width = info->width;

      if (is_neg || info->showsign || info->space)
	--width;
      width -= 3;

      if (!info->left && width > 0)
	PADN (' ', width);

      if (is_neg)
	outchar ('-');
      else if (info->showsign)
	outchar ('+');
      else if (info->space)
	outchar (' ');

      PRINT (special, 3 );

      if (info->left && width > 0)
	PADN (' ', width);

      *ccc = '\0';
      return done;
    }


  /* We need three multiprecision variables.  Now that we have the exponent
     of the number we can allocate the needed memory.  It would be more
     efficient to use variables of the fixed maximum size but because this
     would be really big it could lead to memory problems.  */
  {
    mp_size_t bignum_size = ((ABS (exponent) + BITS_PER_MP_LIMB - 1)
			     / BITS_PER_MP_LIMB
			     + (LDBL_MANT_DIG / BITS_PER_MP_LIMB > 2 ? 8 : 4))
			    * sizeof (mp_limb_t);
    frac = (mp_limb_t *) alloca (bignum_size);
    tmp = (mp_limb_t *) alloca (bignum_size);
    scale = (mp_limb_t *) alloca (bignum_size);
  }

  /* We now have to distinguish between numbers with positive and negative
     exponents because the method used for the one is not applicable/efficient
     for the other.  */
  scalesize = 0;
  if (exponent > 2)
    {
      /* |FP| >= 8.0.  */
      int scaleexpo = 0;
      int explog = LDBL_MAX_10_EXP_LOG;
      int exp10 = 0;
      const struct mp_power *powers = &_fpioconst_pow10[explog + 1];
      int cnt_h, cnt_l, i;

      if ((exponent + to_shift) % BITS_PER_MP_LIMB == 0)
	{
	  MPN_COPY_DECR (frac + (exponent + to_shift) / BITS_PER_MP_LIMB,
			 fp_input, fracsize);
	  fracsize += (exponent + to_shift) / BITS_PER_MP_LIMB;
	}
      else
	{
	  cy = __mpn_lshift (frac + (exponent + to_shift) / BITS_PER_MP_LIMB,
			     fp_input, fracsize,
			     (exponent + to_shift) % BITS_PER_MP_LIMB);
	  fracsize += (exponent + to_shift) / BITS_PER_MP_LIMB;
	  if (cy)
	    frac[fracsize++] = cy;
	}
      MPN_ZERO (frac, (exponent + to_shift) / BITS_PER_MP_LIMB);

      assert (powers > &_fpioconst_pow10[0]);
      do
	{
	  --powers;

	  /* The number of the product of two binary numbers with n and m
	     bits respectively has m+n or m+n-1 bits.	*/
	  if (exponent >= scaleexpo + powers->p_expo - 1)
	    {
	      if (scalesize == 0)
		{
#ifdef COMPILE_FOR_LONG_DOUBLE
		  if (LDBL_MANT_DIG > _FPIO_CONST_OFFSET * BITS_PER_MP_LIMB)
		    {
#define _FPIO_CONST_SHIFT \
  (((LDBL_MANT_DIG + BITS_PER_MP_LIMB - 1) / BITS_PER_MP_LIMB) \
   - _FPIO_CONST_OFFSET)
		      /* 64bit const offset is not enough for
			 IEEE quad long double.  */
		      tmpsize = powers->arraysize + _FPIO_CONST_SHIFT;
		      memcpy (tmp + _FPIO_CONST_SHIFT,
			      &__tens[powers->arrayoff],
			      tmpsize * sizeof (mp_limb_t));
		      MPN_ZERO (tmp, _FPIO_CONST_SHIFT);
		      /* Adjust exponent, as scaleexpo will be this much
			 bigger too.  */
		      exponent += _FPIO_CONST_SHIFT * BITS_PER_MP_LIMB;
		    }
		  else
#endif
		    {
		      tmpsize = powers->arraysize;
		      memcpy (tmp, &__tens[powers->arrayoff],
			      tmpsize * sizeof (mp_limb_t));
		    }
		}
	      else
		{
		  cy = __mpn_mul (tmp, scale, scalesize,
				  &__tens[powers->arrayoff
					 + _FPIO_CONST_OFFSET],
				  powers->arraysize - _FPIO_CONST_OFFSET);
		  tmpsize = scalesize + powers->arraysize - _FPIO_CONST_OFFSET;
		  if (cy == 0)
		    --tmpsize;
		}

	      if (MPN_GE (frac, tmp))
		{
		  int cnt;
		  MPN_ASSIGN (scale, tmp);
		  count_leading_zeros (cnt, scale[scalesize - 1]);
		  scaleexpo = (scalesize - 2) * BITS_PER_MP_LIMB - cnt - 1;
		  exp10 |= 1 << explog;
		}
	    }
	  --explog;
	}
      while (powers > &_fpioconst_pow10[0]);
      exponent = exp10;

      /* Optimize number representations.  We want to represent the numbers
	 with the lowest number of bytes possible without losing any
	 bytes. Also the highest bit in the scaling factor has to be set
	 (this is a requirement of the MPN division routines).  */
      if (scalesize > 0)
	{
	  /* Determine minimum number of zero bits at the end of
	     both numbers.  */
	  for (i = 0; scale[i] == 0 && frac[i] == 0; i++)
	    ;

	  /* Determine number of bits the scaling factor is misplaced.	*/
	  count_leading_zeros (cnt_h, scale[scalesize - 1]);

	  if (cnt_h == 0)
	    {
	      /* The highest bit of the scaling factor is already set.	So
		 we only have to remove the trailing empty limbs.  */
	      if (i > 0)
		{
		  MPN_COPY_INCR (scale, scale + i, scalesize - i);
		  scalesize -= i;
		  MPN_COPY_INCR (frac, frac + i, fracsize - i);
		  fracsize -= i;
		}
	    }
	  else
	    {
	      if (scale[i] != 0)
		{
		  count_trailing_zeros (cnt_l, scale[i]);
		  if (frac[i] != 0)
		    {
		      int cnt_l2;
		      count_trailing_zeros (cnt_l2, frac[i]);
		      if (cnt_l2 < cnt_l)
			cnt_l = cnt_l2;
		    }
		}
	      else
		count_trailing_zeros (cnt_l, frac[i]);

	      /* Now shift the numbers to their optimal position.  */
	      if (i == 0 && BITS_PER_MP_LIMB - cnt_h > cnt_l)
		{
		  /* We cannot save any memory.	 So just roll both numbers
		     so that the scaling factor has its highest bit set.  */

		  (void) __mpn_lshift (scale, scale, scalesize, cnt_h);
		  cy = __mpn_lshift (frac, frac, fracsize, cnt_h);
		  if (cy != 0)
		    frac[fracsize++] = cy;
		}
	      else if (BITS_PER_MP_LIMB - cnt_h <= cnt_l)
		{
		  /* We can save memory by removing the trailing zero limbs
		     and by packing the non-zero limbs which gain another
		     free one. */

		  (void) __mpn_rshift (scale, scale + i, scalesize - i,
				       BITS_PER_MP_LIMB - cnt_h);
		  scalesize -= i + 1;
		  (void) __mpn_rshift (frac, frac + i, fracsize - i,
				       BITS_PER_MP_LIMB - cnt_h);
		  fracsize -= frac[fracsize - i - 1] == 0 ? i + 1 : i;
		}
	      else
		{
		  /* We can only save the memory of the limbs which are zero.
		     The non-zero parts occupy the same number of limbs.  */

		  (void) __mpn_rshift (scale, scale + (i - 1),
				       scalesize - (i - 1),
				       BITS_PER_MP_LIMB - cnt_h);
		  scalesize -= i;
		  (void) __mpn_rshift (frac, frac + (i - 1),
				       fracsize - (i - 1),
				       BITS_PER_MP_LIMB - cnt_h);
		  fracsize -= frac[fracsize - (i - 1) - 1] == 0 ? i : i - 1;
		}
	    }
	}
    }
  else if (exponent < 0)
    {
      /* |FP| < 1.0.  */
      int exp10 = 0;
      int explog = LDBL_MAX_10_EXP_LOG;
      const struct mp_power *powers = &_fpioconst_pow10[explog + 1];
      mp_size_t used_limbs = fracsize - 1;

      /* Now shift the input value to its right place.	*/
      cy = __mpn_lshift (frac, fp_input, fracsize, to_shift);
      frac[fracsize++] = cy;
      assert (cy == 1 || (frac[fracsize - 2] == 0 && frac[0] == 0));

      expsign = 1;
      exponent = -exponent;

      assert (powers != &_fpioconst_pow10[0]);
      do
	{
	  --powers;

	  if (exponent >= powers->m_expo)
	    {
	      int i, incr, cnt_h, cnt_l;
	      mp_limb_t topval[2];

	      /* The __mpn_mul function expects the first argument to be
		 bigger than the second.  */
	      if (fracsize < powers->arraysize - _FPIO_CONST_OFFSET)
		cy = __mpn_mul (tmp, &__tens[powers->arrayoff
					    + _FPIO_CONST_OFFSET],
				powers->arraysize - _FPIO_CONST_OFFSET,
				frac, fracsize);
	      else
		cy = __mpn_mul (tmp, frac, fracsize,
				&__tens[powers->arrayoff + _FPIO_CONST_OFFSET],
				powers->arraysize - _FPIO_CONST_OFFSET);
	      tmpsize = fracsize + powers->arraysize - _FPIO_CONST_OFFSET;
	      if (cy == 0)
		--tmpsize;

	      count_leading_zeros (cnt_h, tmp[tmpsize - 1]);
	      incr = (tmpsize - fracsize) * BITS_PER_MP_LIMB
		     + BITS_PER_MP_LIMB - 1 - cnt_h;

	      assert (incr <= powers->p_expo);

	      /* If we increased the exponent by exactly 3 we have to test
		 for overflow.	This is done by comparing with 10 shifted
		 to the right position.	 */
	      if (incr == exponent + 3)
		{
		  if (cnt_h <= BITS_PER_MP_LIMB - 4)
		    {
		      topval[0] = 0;
		      topval[1]
			= ((mp_limb_t) 10) << (BITS_PER_MP_LIMB - 4 - cnt_h);
		    }
		  else
		    {
		      topval[0] = ((mp_limb_t) 10) << (BITS_PER_MP_LIMB - 4);
		      topval[1] = 0;
		      (void) __mpn_lshift (topval, topval, 2,
					   BITS_PER_MP_LIMB - cnt_h);
		    }
		}

	      /* We have to be careful when multiplying the last factor.
		 If the result is greater than 1.0 be have to test it
		 against 10.0.  If it is greater or equal to 10.0 the
		 multiplication was not valid.  This is because we cannot
		 determine the number of bits in the result in advance.  */
	      if (incr < exponent + 3
		  || (incr == exponent + 3 &&
		      (tmp[tmpsize - 1] < topval[1]
		       || (tmp[tmpsize - 1] == topval[1]
			   && tmp[tmpsize - 2] < topval[0]))))
		{
		  /* The factor is right.  Adapt binary and decimal
		     exponents.	 */
		  exponent -= incr;
		  exp10 |= 1 << explog;

		  /* If this factor yields a number greater or equal to
		     1.0, we must not shift the non-fractional digits down. */
		  if (exponent < 0)
		    cnt_h += -exponent;

		  /* Now we optimize the number representation.	 */
		  for (i = 0; tmp[i] == 0; ++i);
		  if (cnt_h == BITS_PER_MP_LIMB - 1)
		    {
		      MPN_COPY (frac, tmp + i, tmpsize - i);
		      fracsize = tmpsize - i;
		    }
		  else
		    {
		      count_trailing_zeros (cnt_l, tmp[i]);

		      /* Now shift the numbers to their optimal position.  */
		      if (i == 0 && BITS_PER_MP_LIMB - 1 - cnt_h > cnt_l)
			{
			  /* We cannot save any memory.	 Just roll the
			     number so that the leading digit is in a
			     separate limb.  */

			  cy = __mpn_lshift (frac, tmp, tmpsize, cnt_h + 1);
			  fracsize = tmpsize + 1;
			  frac[fracsize - 1] = cy;
			}
		      else if (BITS_PER_MP_LIMB - 1 - cnt_h <= cnt_l)
			{
			  (void) __mpn_rshift (frac, tmp + i, tmpsize - i,
					       BITS_PER_MP_LIMB - 1 - cnt_h);
			  fracsize = tmpsize - i;
			}
		      else
			{
			  /* We can only save the memory of the limbs which
			     are zero.	The non-zero parts occupy the same
			     number of limbs.  */

			  (void) __mpn_rshift (frac, tmp + (i - 1),
					       tmpsize - (i - 1),
					       BITS_PER_MP_LIMB - 1 - cnt_h);
			  fracsize = tmpsize - (i - 1);
			}
		    }
		  used_limbs = fracsize - 1;
		}
	    }
	  --explog;
	}
      while (powers != &_fpioconst_pow10[1] && exponent > 0);
      /* All factors but 10^-1 are tested now.	*/
      if (exponent > 0)
	{
	  int cnt_l;

	  cy = __mpn_mul_1 (tmp, frac, fracsize, 10);
	  tmpsize = fracsize;
	  assert (cy == 0 || tmp[tmpsize - 1] < 20);

	  count_trailing_zeros (cnt_l, tmp[0]);
	  if (cnt_l < MIN (4, exponent))
	    {
	      cy = __mpn_lshift (frac, tmp, tmpsize,
				 BITS_PER_MP_LIMB - MIN (4, exponent));
	      if (cy != 0)
		frac[tmpsize++] = cy;
	    }
	  else
	    (void) __mpn_rshift (frac, tmp, tmpsize, MIN (4, exponent));
	  fracsize = tmpsize;
	  exp10 |= 1;
	  assert (frac[fracsize - 1] < 10);
	}
      exponent = exp10;
    }
  else
    {
      /* This is a special case.  We don't need a factor because the
	 numbers are in the range of 1.0 <= |fp| < 8.0.  We simply
	 shift it to the right place and divide it by 1.0 to get the
	 leading digit.	 (Of course this division is not really made.)	*/
      assert (0 <= exponent && exponent < 3 &&
	      exponent + to_shift < BITS_PER_MP_LIMB);

      /* Now shift the input value to its right place.	*/
      cy = __mpn_lshift (frac, fp_input, fracsize, (exponent + to_shift));
      frac[fracsize++] = cy;
      exponent = 0;
    }

  {
    int width = info->width;
    char *wstartp, *wcp;
    size_t chars_needed;
    int expscale;
    int intdig_max, intdig_no = 0;
    int fracdig_min;
    int fracdig_max;
    int dig_max;
    int significant;

    if (info->spec == 'e')
      {
	type = info->spec;
	intdig_max = 1;
	fracdig_min = fracdig_max = info->prec < 0 ? 6 : info->prec;
	chars_needed = 1 + 1 + (size_t) fracdig_max + 1 + 1 + 4;
	/*	       d   .	 ddd	     e	 +-  ddd  */
	dig_max = INT_MAX;		/* Unlimited.  */
	significant = 1;		/* Does not matter here.  */
      }
    else if (info->spec == 'f')
      {
	type = 'f';
	fracdig_min = fracdig_max = info->prec < 0 ? 6 : info->prec;
	dig_max = INT_MAX;		/* Unlimited.  */
	significant = 1;		/* Does not matter here.  */
	if (expsign == 0)
	  {
	    intdig_max = exponent + 1;
	    /* This can be really big!	*/  /* XXX Maybe malloc if too big? */
	    chars_needed = (size_t) exponent + 1 + 1 + (size_t) fracdig_max;
	  }
	else
	  {
	    intdig_max = 1;
	    chars_needed = 1 + 1 + (size_t) fracdig_max;
	  }
      }
    else
      {
	dig_max = info->prec < 0 ? 6 : (info->prec == 0 ? 1 : info->prec);
	if ((expsign == 0 && exponent >= dig_max)
	    || (expsign != 0 && exponent > 4))
	  {
	    type = 'e';
	    fracdig_max = dig_max - 1;
	    intdig_max = 1;
	    chars_needed = 1 + 1 + (size_t) fracdig_max + 1 + 1 + 4;
	  }
	else
	  {
	    type = 'f';
	    intdig_max = expsign == 0 ? exponent + 1 : 0;
	    fracdig_max = dig_max - intdig_max;
	    /* We need space for the significant digits and perhaps
	       for leading zeros when < 1.0.  The number of leading
	       zeros can be as many as would be required for
	       exponential notation with a negative two-digit
	       exponent, which is 4.  */
	    chars_needed = (size_t) dig_max + 1 + 4;
	  }
	fracdig_min = 0;
	significant = 0;		/* We count significant digits.	 */
      }

    /* Allocate buffer for output.  We need two more because while rounding
       it is possible that we need two more characters in front of all the
       other output.  If the amount of memory we have to allocate is too
       large use `malloc' instead of `alloca'.  */
    if (__builtin_expect (chars_needed >= (size_t) -1 / sizeof (char) - 2
			  || chars_needed < fracdig_max, 0))
      {
	/* Some overflow occurred.  */
// 	__set_errno (ERANGE);
	return -1;
      }
    size_t wbuffer_to_alloc = (2 + chars_needed) * sizeof (char);

//     assert( __libc_use_alloca (wbuffer_to_alloc) );
    wbuffer = (char *) alloca (wbuffer_to_alloc);

    wcp = wstartp = wbuffer + 2;	/* Let room for rounding.  */

    /* Do the real work: put digits in allocated buffer.  */
    if (expsign == 0 || type != 'f')
      {
	assert (expsign == 0 || intdig_max == 1);
	while (intdig_no < intdig_max)
	  {
	    ++intdig_no;
	    *wcp++ = hack_digit ();
	  }
	significant = 1;
	if ( fracdig_min > 0
	    || (fracdig_max > 0 && (fracsize > 1 || frac[0] != 0)))
	  *wcp++ = decimalwc;
      }
    else
      {
	/* |fp| < 1.0 and the selected type is 'f', so put "0."
	   in the buffer.  */
	*wcp++ = '0';
	--exponent;
	*wcp++ = decimalwc;
      }

    /* Generate the needed number of fractional digits.	 */
    int fracdig_no = 0;
    int added_zeros = 0;
    while (fracdig_no < fracdig_min + added_zeros
	   || (fracdig_no < fracdig_max && (fracsize > 1 || frac[0] != 0)))
      {
	++fracdig_no;
	*wcp = hack_digit ();
	if (*wcp++ != '0')
	  significant = 1;
	else if (significant == 0)
	  {
	    ++fracdig_max;
	    if (fracdig_min > 0)
	      ++added_zeros;
	  }
      }

    /* Do rounding.  */
    digit = hack_digit ();
    if (digit > '4')
      {
	char *wtp = wcp;

	if (digit == '5'
	    && ((*(wcp - 1) != decimalwc && (*(wcp - 1) & 1) == 0)
		|| ((*(wcp - 1) == decimalwc && (*(wcp - 2) & 1) == 0))))
	  {
	    /* This is the critical case.	 */
	    if (fracsize == 1 && frac[0] == 0)
	      /* Rest of the number is zero -> round to even.
		 (IEEE 754-1985 4.1 says this is the default rounding.)  */
	      goto do_expo;
	    else if (scalesize == 0)
	      {
		/* Here we have to see whether all limbs are zero since no
		   normalization happened.  */
		size_t lcnt = fracsize;
		while (lcnt >= 1 && frac[lcnt - 1] == 0)
		  --lcnt;
		if (lcnt == 0)
		  /* Rest of the number is zero -> round to even.
		     (IEEE 754-1985 4.1 says this is the default rounding.)  */
		  goto do_expo;
	      }
	  }

	if (fracdig_no > 0)
	  {
	    /* Process fractional digits.  Terminate if not rounded or
	       radix character is reached.  */
	    int removed = 0;
	    while (*--wtp != decimalwc && *wtp == '9')
	      {
		*wtp = '0';
		++removed;
	      }
	    if (removed == fracdig_min && added_zeros > 0)
	      --added_zeros;
	    if (*wtp != decimalwc)
	      /* Round up.  */
	      (*wtp)++;
	  }

	if (fracdig_no == 0 || *wtp == decimalwc)
	  {
	    /* Round the integer digits.  */
	    if (*(wtp - 1) == decimalwc)
	      --wtp;

	    while (--wtp >= wstartp && *wtp == '9')
	      *wtp = '0';

	    if (wtp >= wstartp)
	      /* Round up.  */
	      (*wtp)++;
	    else
	      /* It is more critical.  All digits were 9's.  */
	      {
		if (type != 'f')
		  {
		    *wstartp = '1';
		    exponent += expsign == 0 ? 1 : -1;

		    /* The above exponent adjustment could lead to 1.0e-00,
		       e.g. for 0.999999999.  Make sure exponent 0 always
		       uses + sign.  */
		    if (exponent == 0)
		      expsign = 0;
		  }
		else if (intdig_no == dig_max)
		  {
		    /* This is the case where for type %g the number fits
		       really in the range for %f output but after rounding
		       the number of digits is too big.	 */
		    *--wstartp = decimalwc;
		    *--wstartp = '1';

		    if (fracdig_no > 0)
		      {
			/* Overwrite the old radix character.  */
			wstartp[intdig_no + 2] = '0';
			++fracdig_no;
		      }

		    fracdig_no += intdig_no;
		    intdig_no = 1;
		    fracdig_max = intdig_max - intdig_no;
		    ++exponent;
		    /* Now we must print the exponent.	*/
		    type = 'e';
		  }
		else
		  {
		    /* We can simply add another another digit before the
		       radix.  */
		    *--wstartp = '1';
		    ++intdig_no;
		  }

		/* While rounding the number of digits can change.
		   If the number now exceeds the limits remove some
		   fractional digits.  */
		if (intdig_no + fracdig_no > dig_max)
		  {
		    wcp -= intdig_no + fracdig_no - dig_max;
		    fracdig_no -= intdig_no + fracdig_no - dig_max;
		  }
	      }
	  }
      }

  do_expo:
    /* Now remove unnecessary '0' at the end of the string.  */
    while (fracdig_no > fracdig_min + added_zeros && *(wcp - 1) == '0')
      {
	--wcp;
	--fracdig_no;
      }
    /* If we eliminate all fractional digits we perhaps also can remove
       the radix character.  */
    if (fracdig_no == 0 && *(wcp - 1) == decimalwc)
      --wcp;


    /* Write the exponent if it is needed.  */
    if (type != 'f')
      {
	if (__builtin_expect (expsign != 0 && exponent == 4 && info->spec == 'g', 0))
	  {
	    /* This is another special case.  The exponent of the number is
	       really smaller than -4, which requires the 'e'/'E' format.
	       But after rounding the number has an exponent of -4.  */
	    assert (wcp >= wstartp + 1);
	    assert (wstartp[0] == '1');
	    memcpy (wstartp, "0.0001", 6);
	    wstartp[1] = decimalwc;
	    if (wcp >= wstartp + 2)
	      {
		memset (wstartp + 6, '0', wcp - (wstartp + 2));
		wcp += 4;
	      }
	    else
	      wcp += 5;
	  }
	else
	  {
	    *wcp++ = (char) type;
	    *wcp++ = expsign ? '-' : '+';

	    /* Find the magnitude of the exponent.	*/
	    expscale = 10;
	    while (expscale <= exponent)
	      expscale *= 10;

	    if (exponent < 10)
	      /* Exponent always has at least two digits.  */
	      *wcp++ = '0';
	    else
	      do
		{
		  expscale /= 10;
		  *wcp++ = '0' + (exponent / expscale);
		  exponent %= expscale;
		}
	      while (expscale > 10);
	    *wcp++ = '0' + exponent;
	  }
      }

    /* Compute number of characters which must be filled with the padding
       character.  */
    if (is_neg || info->showsign || info->space)
      --width;
    width -= wcp - wstartp;

    if (!info->left && info->pad != '0' && width > 0)
      PADN (info->pad, width);

    if (is_neg)
      outchar ('-');
    else if (info->showsign)
      outchar ('+');
    else if (info->space)
      outchar (' ');

    if (!info->left && info->pad == '0' && width > 0)
      PADN ('0', width);

    {

    PRINT (wstartp, wcp - wstartp );

    }

    if (info->left && width > 0)
      PADN (info->pad, width);
  }

  *ccc = '\0';
  return done;
}



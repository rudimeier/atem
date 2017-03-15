/*** boobs.h -- byte-order oberations
 *
 * Copyright (C) 2012 Sebastian Freundt
 * Copyright (C) 2012-2017 Ruediger Meier
 * Author:  Sebastian Freundt <freundt@ga-group.nl>
 * License: BSD 3-Clause, see LICENSE file
 *
 ***/

#if !defined INCLUDED_boobs_h_
#define INCLUDED_boobs_h_

#if defined HAVE_CONFIG_H
# include "config.h"
#endif	/* HAVE_CONFIG_H */
/* *bsd except for openbsd */
#if defined HAVE_ENDIAN_H
# include <endian.h>
#elif defined HAVE_SYS_ENDIAN_H
# include <sys/endian.h>
#elif defined HAVE_BOOBS_H
# include <byteorder.h>
#endif	/* ENDIAN_H || SYS/ENDIAN_H || BOOBS_H */

/* check for byteswap to do the swapping ourselves if need be */
#if defined HAVE_BYTESWAP_H
# include <byteswap.h>
#endif	/* BYTESWAP_H */

#if !defined le16toh
# if defined letoh16
#  define le16toh	letoh16
# elif defined WORDS_BIGENDIAN
#  define le16toh(x)	__bswap_16(x)
# else
#  define le16toh(x)	(x)
# endif	 /* letoh16 */
#endif	/* !le16toh */

/* and even now we may be out of luck */
#if !defined be32toh
# if defined betoh32
#  define be32toh	betoh32
# elif defined WORDS_BIGENDIAN
#  define be32toh(x)	(x)
# else
#  define be32toh(x)	__bswap_32(x)
# endif
#endif	/* !be32toh */

#if !defined le32toh
# if defined letoh32
#  define le32toh	letoh32
# elif defined WORDS_BIGENDIAN
#  define le32toh(x)	__bswap_32(x)
# else
#  define le32toh(x)	(x)
# endif	 /* letoh32 */
#endif	/* !le32toh */

#if !defined htobe32
# if defined WORDS_BIGENDIAN
#  define htobe32(x)	(x)
# else
#  define htobe32(x)	__bswap_32(x)
# endif
#endif	/* !be32toh */

#if !defined htole32
# if defined WORDS_BIGENDIAN
#  define htole32(x)	__bswap_32(x)
# else
#  define htole32(x)	(x)
# endif
#endif	/* !htole32 */

/* we could technically include byteswap.h and to the swap ourselves
 * in the missing cases.  Instead we'll just leave it as is and wait
 * for bug reports. */

#endif	/* INCLUDED_boobs_h_ */

/* Copyright 2002-2024 Free Software Foundation, Inc.
     Written by Werner Lemberg <wl@gnu.org>

This file is part of groff, the GNU roff typesetting system.

groff is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation, either version 3 of the License, or
(at your option) any later version.

groff is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "lib.h"

#include "cset.h"
#include "stringclass.h"
#include "unicode.h"

const char *valid_unicode_code_sequence(const char *u, char *errbuf)
{
  if (errbuf != 0 /* nullptr */)
    (void) memset(errbuf, '\0', ERRBUFSZ);
  if (*u != 'u') {
    if (errbuf != 0 /* nullptr */)
      snprintf(errbuf, ERRBUFSZ, "Unicode special character sequence"
	       " lacks 'u' as first character");
    return 0 /* nullptr */;
  }
  const char *p = ++u;
  for (;;) {
    int val = 0;
    const char *start = p;
    for (;;) {
      // only uppercase hex digits allowed
      if (!csxdigit(*p)) {
	if (errbuf != 0 /* nullptr */)
	  snprintf(errbuf, ERRBUFSZ, "Unicode special character"
		   " sequence has non-hexadecimal digit '%c'", *p);
	return 0 /* nullptr */;
      }
      if (csdigit(*p))
	val = val*0x10 + (*p-'0');
      else if (csupper(*p))
	val = val*0x10 + (*p-'A'+10);
      else if ((*p >= 'a') && (*p <= 'f')) {
	if (errbuf != 0 /* nullptr */)
	  snprintf(errbuf, ERRBUFSZ, "Unicode special character"
		" sequence must use uppercase hexadecimal digit, not"
		" '%c'", *p);
	return 0 /* nullptr */;
      }
      else {
	assert(0 == "unhandled hexadecimal digit character");
	return 0 /* nullptr */;
      }
      // biggest Unicode value is U+10FFFF
      if (val > 0x10FFFF) {
	if (errbuf != 0 /* nullptr */)
	  snprintf(errbuf, ERRBUFSZ, "Unicode special character code"
		   " point %04X is out of range (0000..10FFFF)", val);
	return 0 /* nullptr */;
      }
      p++;
      if (*p == '\0' || *p == '_')
	break;
    }
    // surrogates not allowed
    if ((val >= 0xD800 && val <= 0xDBFF)
	|| (val >= 0xDC00 && val <= 0xDFFF)) {
      if (errbuf != 0 /* nullptr */)
	snprintf(errbuf, ERRBUFSZ, "Unicode special character code"
		 " point %04X is a surrogate", val);
      return 0 /* nullptr */;
    }
    const ptrdiff_t width = p - start;
    if (width < 4) {
      if (errbuf != 0 /* nullptr */)
	snprintf(errbuf, ERRBUFSZ, "Unicode special character sequence"
		 " must be 4..6 digits");
      return 0 /* nullptr */;
    }
    else if ((width > 4) && ('0' == *u)) {
      if (errbuf != 0 /* nullptr */)
	snprintf(errbuf, ERRBUFSZ, "Unicode special character sequence"
		 " %s has invalid leading zero(es)", u);
      return 0 /* nullptr */;
    }
    if (*p == '\0')
      break;
    p++;
  }
  return u;
}

// TODO: Does gnulib have a function that does this?
char *to_utf8_string(unsigned int ch)
{
  static char buf[16];

  if (ch < 0x80)
    sprintf(buf, "%c", (ch & 0xff));
  else if (ch < 0x800)
    sprintf(buf, "%c%c",
      0xc0 + ((ch >>  6) & 0x1f),
      0x80 + ((ch      ) & 0x3f));
  else if ((ch < 0xD800) || ((ch > 0xDFFF) && (ch < 0x10000)))
    sprintf(buf, "%c%c%c",
      0xe0 + ((ch >> 12) & 0x0f),
      0x80 + ((ch >>  6) & 0x3f),
      0x80 + ((ch      ) & 0x3f));
  else if ((ch > 0xFFFF) && (ch < 0x120000))
    sprintf(buf, "%c%c%c%c",
      0xf0 + ((ch >> 18) & 0x07),
      0x80 + ((ch >> 12) & 0x3f),
      0x80 + ((ch >>  6) & 0x3f),
      0x80 + ((ch      ) & 0x3f));
  else
    sprintf(buf, "&#x%X;", ch);
  return buf;
}

// Local Variables:
// fill-column: 72
// mode: C++
// End:
// vim: set cindent noexpandtab shiftwidth=2 textwidth=72:

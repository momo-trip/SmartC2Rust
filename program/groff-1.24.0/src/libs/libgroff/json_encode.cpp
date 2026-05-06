/* Copyright 2025 G. Branden Robinson

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

#include <stdio.h> // snprintf()

#include "cset.h" // csprint()
#include "json-encode.h" // json_char

// Return JSON representation of character `c` without bracketing `"`s.
json_char json_encode_char(unsigned char c)
{
  json_char jc;
  // These printable characters require escaping.
  if (('"' == c) || ('\\' == c) || ('/' == c)) {
    jc.len = 2;
    jc.buf[0] = '\\';
    jc.buf[1] = c;
  }
  else if (csprint(c)) {
    jc.len = 1;
    jc.buf[0] = c;
  }
  else if ('\b' == c) {
    jc.len = 2;
    jc.buf[0] = '\\';
    jc.buf[1] = 'b';
  }
  else if ('\t' == c) {
    jc.len = 2;
    jc.buf[0] = '\\';
    jc.buf[1] = 't';
  }
  else if ('\n' == c) {
    jc.len = 2;
    jc.buf[0] = '\\';
    jc.buf[1] = 'n';
  }
  else if ('\f' == c) {
    jc.len = 2;
    jc.buf[0] = '\\';
    jc.buf[1] = 'f';
  }
  else if ('\r' == c) {
    jc.len = 2;
    jc.buf[0] = '\\';
    jc.buf[1] = 'r';
  }
  else {
    jc.len = 6;
    (void) snprintf(jc.buf, sizeof jc.buf, "\\u%04X", c);
  }
  return jc;
}

// Local Variables:
// fill-column: 72
// mode: C++
// End:
// vim: set cindent noexpandtab shiftwidth=2 textwidth=72:

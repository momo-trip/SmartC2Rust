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

// Convert a groff glyph name to a C string containing an
// underscore-separated list of Unicode code points.  For example,
//
//   '-'   ->  '2010'
//   ',c'  ->  '00E7'
//   'fl'  ->  '0066_006C'
//
// Return NULL if there is no equivalent.
const char *glyph_name_to_unicode(const char *);

// Convert a C string containing an underscore-separated list of Unicode
// code points to a groff glyph name.  For example,
//
//   '2010'       ->  'hy'
//   '0066_006C'  ->  'fl'
//
// Return NULL if there is no equivalent.
const char *unicode_to_glyph_name(const char *);

// Convert a C string containing a precomposed Unicode character to a
// string containing an underscore-separated list of Unicode code
// points, representing its canonical decomposition.  Also perform
// compatibility equivalent replacement.  For example,
//
//   '1F3A' -> '0399_0313_0300'
//   'FA6A' -> '983B'
//
// Return NULL if there is no equivalent.
const char *decompose_unicode(const char *);

// Validate the given C string as representing a Unicode grapheme
// cluster to troff or an output driver.  The string must match the
// extended regular expression 'u1*[0-9]{4,5}(_1*[0-9]{4,5})*' and obey
// the following rules.
//
//   - 'NNNN' must consist of at least 4 hexadecimal digits in upper
//     case.
//   - If there are more than 4 hexadecimal digits, the leading one must
//     not be zero.
//   - 'NNNN' must denote a valid Unicode code point (U+0000..U+10FFFF,
//     excluding surrogate code points.
//   - The string may represent a sequence of Unicode code points
//     separated by '_' characters.  Each must satisfy the criteria
//     above.  It is up to the caller to ensure that the first is a base
//     character and that subsequent ones are valid combining characters
//     (in troff, these are set up with the `composite` request).
//
// Return a pointer to the second character in the string (skipping the
// leading 'u') if successful, and a null pointer otherwise.
//
// If given a second argument, store a diagnostic message there if the
// above rules are not satisfied.
const char *valid_unicode_code_sequence(const char *,
  char * /* errbuf */ = 0 /* nullptr */);

// valid_unicode_code_sequence() writes to an error message buffer.
const size_t ERRBUFSZ = 256;

// Track the length of the longest Unicode special character identifier
// that `glyph_name_to_unicode` might return.
const size_t UNIBUFSZ = sizeof "u10FFFF"; // see glyphuni.cpp

// Track the length of the longest special character identifier that
// `unicode_to_glyph_name` might return.
const size_t GLYPHBUFSZ = sizeof "bracketrighttp"; // see uniglyph.cpp

char *to_utf8_string (unsigned int);

// Local Variables:
// fill-column: 72
// mode: C++
// End:
// vim: set cindent noexpandtab shiftwidth=2 textwidth=72:

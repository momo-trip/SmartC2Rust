/* Copyright 1989-2025 Free Software Foundation, Inc.
     Written by James Clark (jjc@jclark.com)

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

#include <assert.h>
#include <ctype.h>
#include <stdlib.h> // strotol()
#include <string.h> // memcpy(), strcpy(), strncmp()

#include "lib.h" // strsave()

#include "errarg.h"
#include "error.h"
#include "font.h"
#include "ptable.h"
#include "itable.h"

// troff has a more elaborate `charinfo` class that stores much more
// information.  All libgroff and output drivers need is a way to
// retrieve the object's name as a C string.
class charinfo : glyph {
public:
  const char *name;	// The glyph name, or a null pointer.
  friend class character_indexer;
};

// PTABLE(charinfo) is a hash table mapping `const char *` to
// `charinfo *`.
declare_ptable(charinfo)
implement_ptable(charinfo)

// ITABLE(charinfo) is a hash table mapping `int >= 0` to `charinfo *`.
declare_itable(charinfo)
implement_itable(charinfo)

// This class is a registry storing all named and numbered glyphs known
// so far, assigning a unique index to each glyph.
class character_indexer {
public:
  character_indexer();
  ~character_indexer();
  // --------------------- Lookup or creation of a glyph.
  glyph *ascii_char_glyph(unsigned char);
  glyph *named_char_glyph(const char *);
  glyph *numbered_char_glyph(int);
private:
  int next_index;		// Number of glyphs already allocated.
  PTABLE(charinfo) table;	// Table mapping name to glyph.
  glyph *ascii_glyph[256];	// Shorthand table for looking up
				// "charNNN" glyphs.
  ITABLE(charinfo) ntable;	// Table mapping number to glyph.
  enum { NSMALL = 256 };
  glyph *small_number_glyph[NSMALL]; // Shorthand table for looking up
				// numbered glyphs with small numbers.
};

character_indexer::character_indexer()
: next_index(0)
{
  int i;
  for (i = 0; i < 256; i++)
    ascii_glyph[i] = UNDEFINED_GLYPH;
  for (i = 0; i < NSMALL; i++)
    small_number_glyph[i] = UNDEFINED_GLYPH;
}

character_indexer::~character_indexer()
{
}

// Keep this in sync with "src/roff/troff/input.cpp".
// constexpr // C++11
static const char char_prefix[] = { 'c', 'h', 'a', 'r' };
// constexpr // C++11
static const size_t char_prefix_len = sizeof char_prefix;

glyph *character_indexer::ascii_char_glyph(unsigned char c)
{
  if (UNDEFINED_GLYPH == ascii_glyph[c]) {
    char buf[sizeof char_prefix + 3 + 1]; // "char" + nnn + '\0'
    (void) memcpy(buf, char_prefix, char_prefix_len);
    (void) strcpy(buf + char_prefix_len, i_to_a(c));
    charinfo *ci = new charinfo;
    ci->index = next_index++;
    ci->number = -1;
    ci->name = strsave(buf);
    ascii_glyph[c] = ci;
  }
  return ascii_glyph[c];
}

inline glyph *character_indexer::named_char_glyph(const char *s)
{
  // Glyphs with name 'charNNN' are stored only in `ascii_glyph[]`, not
  // in the table.  Therefore treat them specially here.
  if (strncmp(s, char_prefix, char_prefix_len) == 0) {
    char *val;
    long n = strtol((s + char_prefix_len), &val, 10);
    if ((val != (s + char_prefix_len)) && ('\0' == *val)
	&& (n >= 0) && (n < 256))
      return ascii_char_glyph((unsigned char)n);
  }
  charinfo *ci = table.lookupassoc(&s);
  if (0 /* nullptr */ == ci) {
    ci = new charinfo[1];
    ci->index = next_index++;
    ci->number = -1;
    ci->name = table.define(s, ci);
  }
  return ci;
}

inline glyph *character_indexer::numbered_char_glyph(int n)
{
  if ((n >= 0) && (n < NSMALL)) {
    if (UNDEFINED_GLYPH == small_number_glyph[n]) {
      charinfo *ci = new charinfo;
      ci->index = next_index++;
      ci->number = n;
      ci->name = 0 /* nullptr */;
      small_number_glyph[n] = ci;
    }
    return small_number_glyph[n];
  }
  charinfo *ci = ntable.lookup(n);
  if (0 /* nullptr */ == ci) {
    ci = new charinfo[1];
    ci->index = next_index++;
    ci->number = n;
    ci->name = 0 /* nullptr */;
    ntable.define(n, ci);
  }
  return ci;
}

static character_indexer indexer;

glyph *number_to_glyph(int n)
{
  return indexer.numbered_char_glyph(n);
}

// troff overrides these functions with its own versions.

glyph *name_to_glyph(const char *s)
{
  assert((s != 0 /* nullptr */) && (s[0] != '\0') && (s[0] != ' '));
  if ('\0' == s[1])
    // \200 and char128 are synonyms
    return indexer.ascii_char_glyph(s[0]);
  return indexer.named_char_glyph(s);
}

const char *glyph_to_name(glyph *g)
{
  // In both libgroff and troff, `charinfo` has `glyph` as a base class.
  // But in troff, `charinfo` stores much more information.
  charinfo *ci = reinterpret_cast<charinfo *>(g);
  return ci->name;
}

// Local Variables:
// fill-column: 72
// mode: C++
// End:
// vim: set cindent noexpandtab shiftwidth=2 textwidth=72:

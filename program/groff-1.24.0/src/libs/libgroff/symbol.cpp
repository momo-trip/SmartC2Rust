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
#include <string.h> // strcat(), strcmp(), strcpy(), strlen()
#include <stdlib.h> // calloc()

#include "cset.h" // csprint()
#include "json-encode.h" // json_char, json_encode_char()
#include "lib.h"

#include "errarg.h"
#include "error.h"
#include "symbol.h"

const char **symbol::table = 0 /* nullptr */;
int symbol::table_used = 0; // # of entries in use
int symbol::table_size = 0;
char *symbol::block = 0 /* nullptr */;
size_t symbol::block_size = 0;

const symbol NULL_SYMBOL;
const symbol EMPTY_SYMBOL("");

#ifdef BLOCK_SIZE
#undef BLOCK_SIZE
#endif

const int BLOCK_SIZE = 1024;
// the table will increase in size as necessary
// the size will be chosen from the following array
// add some more if you want
static const unsigned int table_sizes[] = {
  101, 503, 1009, 2003, 3001, 4001, 5003, 10007, 20011, 40009, 80021,
  160001, 500009, 1000003, 1500007, 2000003, 0
};
// Don't populate table entries above this ratio.
const double FULL_MAX = 0.3;

static unsigned int hash_string(const char *p)
{
  // compute a hash code; this assumes 32-bit unsigned ints
  // see p436 of  Compilers by Aho, Sethi & Ullman
  // give special treatment to two-character names
  unsigned int hc = 0, g;
  if (*p != 0) {
    hc = *p++;
    if (*p != 0) {
      hc <<= 7;
      hc += *p++;
      for (; *p != 0; p++) {
	hc <<= 4;
	hc += *p;
	if ((g = (hc & 0xf0000000)) == 0) {
	  hc ^= g >> 24;
	  hc ^= g;
	}
      }
    }
  }
  return hc;
}

// Tell compiler that a variable is intentionally unused.
inline void unused(void *) { }

symbol::symbol(const char *p, int how)
{
  if (p == 0 /* nullptr */) {
    s = 0 /* nullptr */;
    return;
  }
  if (*p == 0 /* nullptr */) {
    s = "";
    return;
  }
  if (table == 0 /* nullptr */) {
    table_size = table_sizes[0];
    table = (const char **)new char*[table_size];
    for (int i = 0; i < table_size; i++)
      table[i] = 0 /* nullptr */;
    table_used = 0;
  }
  unsigned int hc = hash_string(p);
  const char **pp;
  for (pp = table + hc % table_size;
       *pp != 0 /* nullptr */;
       (pp == table ? pp = table + table_size - 1 : --pp))
    if (strcmp(p, *pp) == 0) {
      s = *pp;
      return;
    }
  if (how == MUST_ALREADY_EXIST) {
    s = 0 /* nullptr */;
    return;
  }
  if ((table_used >= (table_size - 1))
      || (table_used >= (table_size * FULL_MAX))) {
    const char **old_table = table;
    unsigned int old_table_size = table_size;
    int i;
    for (i = 1; table_sizes[i] <= old_table_size; i++)
      if (table_sizes[i] == 0)
	fatal("too many symbols");
    table_size = table_sizes[i];
    table_used = 0;
    table = (const char **)new char*[table_size];
    for (i = 0; i < table_size; i++)
      table[i] = 0 /* nullptr */;
    for (pp = old_table + old_table_size - 1;
	 pp >= old_table;
	 --pp) {
	   symbol temp(*pp, 1); /* insert it into the new table */
	   unused(&temp);
	 }
    delete[] old_table;
    for (pp = table + hc % table_size;
	 *pp != 0 /* nullptr */;
	 (pp == table ? pp = table + table_size - 1 : --pp))
      ;
  }
  ++table_used;
  if (how == DONT_STORE)
    s = *pp = p;
  else {
    size_t len = strlen(p) + 1;
    if ((block == 0 /* nullptr */) || (block_size < len)) {
      block_size = len > BLOCK_SIZE ? len : BLOCK_SIZE;
      block = new char [block_size];
    }
    (void) strcpy(block, p);
    s = *pp = block;
    block += len;
    block_size -= len;
  }
}

symbol concat(symbol s1, symbol s2)
{
  char *buf = new char [strlen(s1.contents())
			+ strlen(s2.contents())
			+ 1];
  strcpy(buf, s1.contents());
  strcat(buf, s2.contents());
  symbol res(buf);
  delete[] buf;
  return res;
}

// Compute length of JSON representation of object.
size_t symbol::json_length() const
{
  size_t len = 0;
  const char *p = s;
  char ch;
  int nextrachars = 2; // leading and trailing double quotes
  for (size_t i = 0; p[i] != '\0'; i++, len++) {
    ch = p[i];
    assert ((ch >= 0) && (ch <= 127));
    // These printable characters require escaping.
    if (('"' == ch) || ('\\' == ch) || ('/' == ch))
      nextrachars++;
    else if (csprint(ch))
      ;
    else
      switch (ch) {
      case '\b':
      case '\f':
      case '\n':
      case '\r':
      case '\t':
	nextrachars++;
	break;
      default:
	nextrachars += 5;
    }
  }
  return (len + nextrachars);
}

// Like `extract()`, but double-quote the string and escape characters
// per JSON.  (Unlike groff's `string`, a `symbol` doesn't contain
// embedded null characters.)
const char *symbol::json_extract() const
{
  const char *p = s;
  char *r;
  size_t n = strlen(s);
  size_t i;
  char *q = static_cast<char *>(calloc((this->json_length() + 1),
				       sizeof (char)));
  if (q != 0 /* nullptr */) {
    r = q;
    *r++ = '"';
    json_char ch;
    for (i = 0; i < n; i++, p++) {
      ch = json_encode_char(*p);
      for (size_t j = 0; j < ch.len; j++)
	*r++ = ch.buf[j];
    }
    *r++ = '"';
  }
  else
    return strdup("\"\""); // so it can be free()d
  *r++ = '\0';
  return q;
}

// Dump symbol in JSON representation to standard error stream.
void symbol::json_dump() const
{
  const char *repr = this->json_extract();
  size_t jsonlen = this->json_length();
  // Write it out by character to keep libc string functions from
  // interpreting escape sequences.
  for (size_t i = 0; i < jsonlen; i++)
    fputc(repr[i], stderr);
  free(const_cast<char *>(repr));
}

symbol default_symbol("default");

// Local Variables:
// fill-column: 72
// mode: C++
// End:
// vim: set cindent noexpandtab shiftwidth=2 textwidth=72:

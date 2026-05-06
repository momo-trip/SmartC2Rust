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

#include <stdio.h> // FILE, putc(), sprintf()
#include <stdlib.h> // calloc()
#include <string.h> // memchr(), memcmp(), memcpy(), memmem(), memset(),
		    // strlen(), size_t

#include "cset.h" // csprint()
#include "lib.h"
#include "json-encode.h" // json_char, json_encode_char()

#include "stringclass.h"

static char *salloc(int len, int *sizep);
static void sfree(char *ptr, int size);
static char *sfree_alloc(char *ptr, int size, int len, int *sizep);
static char *srealloc(char *ptr, int size, int oldlen, int newlen,
		      int *sizep);

static char *salloc(int len, int *sizep)
{
  if (len == 0) {
    *sizep = 0;
    return 0;
  }
  else
    return new char[*sizep = (len * 2)];
}

static void sfree(char *ptr, int)
{
  delete[] ptr;
}

static char *sfree_alloc(char *ptr, int oldsz, int len, int *sizep)
{
  if (oldsz >= len) {
    *sizep = oldsz;
    return ptr;
  }
  delete[] ptr;
  if (len == 0) {
    *sizep = 0;
    return 0;
  }
  else
    return new char[*sizep = (len * 2)];
}

static char *srealloc(char *ptr, int oldsz, int oldlen, int newlen,
		      int *sizep)
{
  if (oldsz >= newlen) {
    *sizep = oldsz;
    return ptr;
  }
  if (newlen == 0) {
    delete[] ptr;
    *sizep = 0;
    return 0;
  }
  else {
    char *p = new char[*sizep = (newlen * 2)];
    if (oldlen < newlen && oldlen != 0)
      memcpy(p, ptr, oldlen);
    delete[] ptr;
    return p;
  }
}

string::string() : ptr(0), len(0), sz(0)
{
}

string::string(const char *p, int n) : len(n)
{
  assert(n >= 0);
  ptr = salloc(n, &sz);
  if (n != 0)
    memcpy(ptr, p, n);
}

string::string(const char *p)
{
  if (p == 0 /* nullptr */) {
    len = 0;
    ptr = 0 /* nullptr */;
    sz = 0;
  }
  else {
    len = strlen(p);
    ptr = salloc(len, &sz);
    if (len < sz)
      memset(ptr, 0, sz);
    if (len != 0)
      memcpy(ptr, p, len);
  }
}

string::string(char c) : len(1)
{
  ptr = salloc(1, &sz);
  *ptr = c;
}

string::string(const string &s) : len(s.len)
{
  ptr = salloc(len, &sz);
  if (len != 0)
    memcpy(ptr, s.ptr, len);
}

string::~string()
{
  sfree(ptr, sz);
}

string &string::operator=(const string &s)
{
  ptr = sfree_alloc(ptr, sz, s.len, &sz);
  len = s.len;
  if (len != 0)
    memcpy(ptr, s.ptr, len);
  return *this;
}

string &string::operator=(const char *p)
{
  if (p == 0) {
    sfree(ptr, len);
    len = 0;
    ptr = 0;
    sz = 0;
  }
  else {
    size_t slen = strlen(p);
    ptr = sfree_alloc(ptr, sz, slen, &sz);
    len = slen;
    if (len != 0)
      memcpy(ptr, p, len);
  }
  return *this;
}

string &string::operator=(char c)
{
  ptr = sfree_alloc(ptr, sz, 1, &sz);
  len = 1;
  *ptr = c;
  return *this;
}

void string::move(string &s)
{
  sfree(ptr, sz);
  ptr = s.ptr;
  len = s.len;
  sz = s.sz;
  s.ptr = 0;
  s.len = 0;
  s.sz = 0;
}

void string::grow1()
{
  ptr = srealloc(ptr, sz, len, len + 1, &sz);
}

string &string::operator+=(const char *p)
{
  if (p != 0) {
    size_t n = strlen(p);
    int newlen = len + n;
    if (newlen > sz)
      ptr = srealloc(ptr, sz, len, newlen, &sz);
    memcpy(ptr + len, p, n);
    len = newlen;
  }
  return *this;
}

string &string::operator+=(const string &s)
{
  if (s.len != 0) {
    int newlen = len + s.len;
    if (newlen > sz)
      ptr = srealloc(ptr, sz, len, newlen, &sz);
    memcpy(ptr + len, s.ptr, s.len);
    len = newlen;
  }
  return *this;
}

void string::append(const char *p, int n)
{
  if (n > 0) {
    int newlen = len + n;
    if (newlen > sz)
      ptr = srealloc(ptr, sz, len, newlen, &sz);
    memcpy(ptr + len, p, n);
    len = newlen;
  }
}

string::string(const char *s1, int n1, const char *s2, int n2)
{
  assert(n1 >= 0 && n2 >= 0);
  len = n1 + n2;
  if (len == 0) {
    sz = 0;
    ptr = 0;
  }
  else {
    ptr = salloc(len, &sz);
    if (n1 == 0)
      memcpy(ptr, s2, n2);
    else {
      memcpy(ptr, s1, n1);
      if (n2 != 0)
	memcpy(ptr + n1, s2, n2);
    }
  }
}

int operator<=(const string &s1, const string &s2)
{
  return (s1.len <= s2.len
	  ? s1.len == 0 || memcmp(s1.ptr, s2.ptr, s1.len) <= 0
	  : s2.len != 0 && memcmp(s1.ptr, s2.ptr, s2.len) < 0);
}

int operator<(const string &s1, const string &s2)
{
  return (s1.len < s2.len
	  ? s1.len == 0 || memcmp(s1.ptr, s2.ptr, s1.len) <= 0
	  : s2.len != 0 && memcmp(s1.ptr, s2.ptr, s2.len) < 0);
}

int operator>=(const string &s1, const string &s2)
{
  return (s1.len >= s2.len
	  ? s2.len == 0 || memcmp(s1.ptr, s2.ptr, s2.len) >= 0
	  : s1.len != 0 && memcmp(s1.ptr, s2.ptr, s1.len) > 0);
}

int operator>(const string &s1, const string &s2)
{
  return (s1.len > s2.len
	  ? s2.len == 0 || memcmp(s1.ptr, s2.ptr, s2.len) >= 0
	  : s1.len != 0 && memcmp(s1.ptr, s2.ptr, s1.len) > 0);
}

void string::set_length(int i)
{
  assert(i >= 0);
  if (i > sz)
    ptr = srealloc(ptr, sz, len, i, &sz);
  len = i;
}

void string::clear()
{
  len = 0;
}

int string::search(const char c) const
{
  const char *p = ptr
		  ? static_cast<const char *>(memchr(ptr, c, len))
		  : 0 /* nullptr */;
  return (p != 0 /* nullptr */) ? (p - ptr) : -1;
}

// Return index of substring `c` in string, -1 if not found.
int string::find(const char *c) const
{
  const char *p = ptr
		  ? static_cast<const char *>(memmem(ptr, len, c,
						     strlen(c)))
		  : 0  /* nullptr */;
  return (p != 0 /* nullptr */) ? (p - ptr) : -1;
}

// Return pointer to null-terminated C string; any nulls internal to the
// string are omitted.  The caller is responsible for `free()`ing the
// returned storage.
char *string::extract() const
{
  char *p = ptr;
  int n = len;
  int nnuls = 0;
  int i;
  for (i = 0; i < n; i++)
    if (p[i] == '\0')
      nnuls++;
  char *q = static_cast<char *>(calloc(n + 1 - nnuls, sizeof(char)));
  if (q != 0 /* nullptr */) {
    char *r = q;
    for (i = 0; i < n; i++)
      if (p[i] != '\0')
	*r++ = p[i];
    *r = '\0';
  }
  return q;
}

// Compute length of JSON representation of object.
size_t string::json_length() const
{
  size_t n = len;
  const char *p = ptr;
  char ch;
  int nextrachars = 2; // leading and trailing double quotes
  for (size_t i = 0; i < n; i++) {
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
  return (n + nextrachars);
}

// Like `extract()`, but double-quote the string and escape characters
// per JSON and emit nulls.
const char *string::json_extract() const
{
  const char *p = ptr;
  char *r;
  size_t n = len;
  size_t i;
  char *q = static_cast<char *>(calloc(this->json_length() + 1,
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
    return strdup("\"\"");
  *r++ = '\0';
  return q;
}

// Dump string in JSON representation to standard error stream.
void string::json_dump() const
{
  const char *repr = this->json_extract();
  size_t jsonlen = this->json_length();
  // Write it out by character to keep libc string functions from
  // interpreting escape sequences.
  for (size_t i = 0; i < jsonlen; i++)
    fputc(repr[i], stderr);
  free(const_cast<char *>(repr));
}

void string::remove_spaces()
{
  int l = len - 1;
  while ((l >= 0) && (ptr[l] == ' '))
    l--;
  char *p = ptr;
  if (l > 0)
    while (*p == ' ') {
      p++;
      l--;
    }
  if (len - 1 != l) {
    if (l >= 0) {
      len = l + 1;
      char *tmp = new char[sz];
      memcpy(tmp, p, len);
      delete[] ptr;
      ptr = tmp;
    }
    else {
      len = 0;
      if (ptr) {
	delete[] ptr;
	ptr = 0;
	sz = 0;
      }
    }
  }
}

void put_string(const string &s, FILE *fp)
{
  int len = s.length();
  const char *ptr = s.contents();
  for (int i = 0; i < len; i++)
    putc(ptr[i], fp);
}

string as_string(int i)
{
  static char buf[INT_DIGITS + 2];
  sprintf(buf, "%d", i);
  return string(buf);
}

// Local Variables:
// fill-column: 72
// mode: C++
// End:
// vim: set cindent noexpandtab shiftwidth=2 textwidth=72:

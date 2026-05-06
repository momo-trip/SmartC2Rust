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

#ifndef GROFF_STRINGCLASS_H
#define GROFF_STRINGCLASS_H

#include <assert.h>
#include <string.h> // memcmp(), strlen()
#include <stdio.h> // FILE

// Ensure that the first declaration of functions that are later
// declared as inline declares them as inline.

class string;

inline string operator+(const string &, const string &);
inline string operator+(const string &, const char *);
inline string operator+(const char *, const string &);
inline string operator+(const string &, char);
inline string operator+(char, const string &);
inline int operator==(const string &, const string &);
inline int operator!=(const string &, const string &);

class string {
public:
  string();
  string(const string &);
  string(const char *);
  string(const char *, int);
  string(char);

  ~string();

  string &operator=(const string &);
  string &operator=(const char *);
  string &operator=(char);

  string &operator+=(const string &);
  string &operator+=(const char *);
  string &operator+=(char);
  void append(const char *, int);

  int length() const;
  int empty() const;
  int operator*() const;

  string substring(int i, int n) const;

  char &operator[](int);
  char operator[](int) const;

  void set_length(int i);
  const char *contents() const;
  int search(const char) const;
  int find(const char *) const;
  char *extract() const;
  size_t json_length() const;
  const char *json_extract() const;
  void json_dump() const;
  void remove_spaces();
  void clear();
  void move(string &);

  friend string operator+(const string &, const string &);
  friend string operator+(const string &, const char *);
  friend string operator+(const char *, const string &);
  friend string operator+(const string &, char);
  friend string operator+(char, const string &);

  friend int operator==(const string &, const string &);
  friend int operator!=(const string &, const string &);
  friend int operator<=(const string &, const string &);
  friend int operator<(const string &, const string &);
  friend int operator>=(const string &, const string &);
  friend int operator>(const string &, const string &);

private:
  char *ptr;
  int len;
  int sz;

  string(const char *, int, const char *, int);	// for use by operator+
  void grow1();
};


inline char &string::operator[](int i)
{
  assert((i >= 0) && (i < len));
  return ptr[i];
}

inline char string::operator[](int i) const
{
  assert((i >= 0) && (i < len));
  return ptr[i];
}

inline int string::length() const
{
  return len;
}

inline int string::empty() const
{
  return (len == 0);
}

inline int string::operator*() const
{
  return len;
}

inline const char *string::contents() const
{
  return ptr;
}

inline string operator+(const string &s1, const string &s2)
{
  return string(s1.ptr, s1.len, s2.ptr, s2.len);
}

inline string operator+(const string &s1, const char *s2)
{
  return (0 /* nullptr */ == s2)
	 ? s1 : string(s1.ptr, s1.len, s2, strlen(s2));
}

inline string operator+(const char *s1, const string &s2)
{
  return (0 /* nullptr */ == s1)
	 ? s2 : string(s1, strlen(s1), s2.ptr, s2.len);
}

inline string operator+(const string &s, char c)
{
  return string(s.ptr, s.len, &c, 1);
}

inline string operator+(char c, const string &s)
{
  return string(&c, 1, s.ptr, s.len);
}

inline int operator==(const string &s1, const string &s2)
{
  return (s1.len == s2.len
	  && (s1.len == 0 || memcmp(s1.ptr, s2.ptr, s1.len) == 0));
}

inline int operator!=(const string &s1, const string &s2)
{
  return (s1.len != s2.len
	  || (s1.len != 0 && memcmp(s1.ptr, s2.ptr, s1.len) != 0));
}

inline string string::substring(int i, int n) const
{
  assert((i >= 0) && ((i + n) <= len));
  return string(ptr + i, n);
}

inline string &string::operator+=(char c)
{
  if (len >= sz)
    grow1();
  ptr[len++] = c;
  return *this;
}

void put_string(const string &, FILE *);

string as_string(int);

#endif // GROFF_STRINGCLASS_H

// Local Variables:
// fill-column: 72
// mode: C++
// End:
// vim: set cindent noexpandtab shiftwidth=2 textwidth=72:

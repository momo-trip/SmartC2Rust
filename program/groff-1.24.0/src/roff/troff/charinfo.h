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

#include <vector>
#include <utility>

extern bool using_character_classes;	// if `.class` is invoked
extern void get_flags();

class macro;

// libgroff has a simpler `charinfo` class that stores much less
// information.
class charinfo : glyph {
  static int next_index;
  charinfo *translation;
  macro *mac;
  unsigned char special_translation;
  unsigned char hyphenation_code;
  unsigned int flags;
  unsigned char ascii_code;
  unsigned char asciify_code;
  bool is_not_found;
  bool is_transparently_translatable;
  bool translatable_as_input; // asciify_code is active for .asciify
  char_mode mode;
  // Unicode character classes
  std::vector<std::pair<int, int> > ranges;
  std::vector<charinfo *> nested_classes; // XXX: see Savannah #67770
public:
  // Values for the flags bitmask.  See groff manual, description of the
  // '.cflags' request.
  //
  // Keep these symbol names in sync with the subset used in the `enum`
  // `break_char_type`; see "node.cpp".
  //
  // C++11: Use `enum : unsigned int`.
  enum {
    ENDS_SENTENCE = 0x01,
    ALLOWS_BREAK_BEFORE = 0x02,
    ALLOWS_BREAK_AFTER = 0x04,
    OVERLAPS_HORIZONTALLY = 0x08,
    OVERLAPS_VERTICALLY = 0x10,
    IS_TRANSPARENT_TO_END_OF_SENTENCE = 0x20,
    IGNORES_SURROUNDING_HYPHENATION_CODES = 0x40,
    PROHIBITS_BREAK_BEFORE = 0x80,
    PROHIBITS_BREAK_AFTER = 0x100,
    IS_INTERWORD_SPACE = 0x200,
    CFLAGS_MAX = 0x2FF
  };
  //
  // C++11: Use `enum : unsigned char`.
  enum {
    TRANSLATE_NONE,
    TRANSLATE_SPACE,
    TRANSLATE_DUMMY,
    TRANSLATE_STRETCHABLE_SPACE,
    TRANSLATE_HYPHEN_INDICATOR
  };
  symbol nm;
  charinfo(symbol);
  glyph *as_glyph();
  bool ends_sentence();
  bool overlaps_vertically();
  bool overlaps_horizontally();
  bool allows_break_before();
  bool allows_break_after();
  bool is_transparent_to_end_of_sentence();
  bool ignores_surrounding_hyphenation_codes();
  bool prohibits_break_before();
  bool prohibits_break_after();
  bool is_interword_space();
  unsigned char get_hyphenation_code();
  unsigned char get_ascii_code();
  unsigned char get_asciify_code();
  int get_unicode_mapping();
  void set_hyphenation_code(unsigned char);
  void set_ascii_code(unsigned char);
  void set_asciify_code(unsigned char);
  void make_translatable_as_input();
  bool is_translatable_as_input();
  charinfo *get_translation(bool = false);
  void set_translation(charinfo *, bool /* transparently */,
		       bool /* as_input */);
  void get_flags();
  void set_flags(unsigned int);
  void set_special_translation(int, bool /* transparently */);
  int get_special_translation(bool = false);
  macro *set_macro(macro *);
  macro *set_macro(macro *, char_mode);
  macro *get_macro();
  bool first_time_not_found();
  void set_number(int);
  int get_number();
  bool is_numbered();
  bool is_normal();
  bool is_fallback();
  bool is_special();
  symbol *get_symbol();
  void add_to_class(int);
  void add_to_class(int, int);
  void add_to_class(charinfo *);
  bool is_class();
  bool contains(int, bool = false);
  bool contains(symbol, bool = false);
  bool contains(charinfo *, bool = false);
  void describe_flags();
  void dump_flags();
  void dump();
};

extern charinfo *lookup_charinfo(symbol,
				 bool /* suppress_creation */ = false);
extern charinfo *charset_table[];

inline bool charinfo::overlaps_horizontally()
{
  if (using_character_classes)
    ::get_flags();
  return (flags & OVERLAPS_HORIZONTALLY);
}

inline bool charinfo::overlaps_vertically()
{
  if (using_character_classes)
    ::get_flags();
  return (flags & OVERLAPS_VERTICALLY);
}

inline bool charinfo::allows_break_before()
{
  if (using_character_classes)
    ::get_flags();
  return (flags & ALLOWS_BREAK_BEFORE);
}

inline bool charinfo::allows_break_after()
{
  if (using_character_classes)
    ::get_flags();
  return (flags & ALLOWS_BREAK_AFTER);
}

inline bool charinfo::ends_sentence()
{
  if (using_character_classes)
    ::get_flags();
  return (flags & ENDS_SENTENCE);
}

inline bool charinfo::is_transparent_to_end_of_sentence()
{
  if (using_character_classes)
    ::get_flags();
  return (flags & IS_TRANSPARENT_TO_END_OF_SENTENCE);
}

inline bool charinfo::ignores_surrounding_hyphenation_codes()
{
  if (using_character_classes)
    ::get_flags();
  return (flags & IGNORES_SURROUNDING_HYPHENATION_CODES);
}

inline bool charinfo::prohibits_break_before()
{
  if (using_character_classes)
    ::get_flags();
  return (flags & PROHIBITS_BREAK_BEFORE);
}

inline bool charinfo::prohibits_break_after()
{
  if (using_character_classes)
    ::get_flags();
  return (flags & PROHIBITS_BREAK_AFTER);
}

inline bool charinfo::is_interword_space()
{
  if (using_character_classes)
    ::get_flags();
  return (flags & IS_INTERWORD_SPACE);
}

inline bool charinfo::is_numbered()
{
  return (number >= 0);
}

inline bool charinfo::is_normal()
{
  return (mode == CHAR_NORMAL);
}

inline bool charinfo::is_fallback()
{
  return (mode == CHAR_FALLBACK);
}

inline bool charinfo::is_special()
{
  return (mode == CHAR_SPECIAL_FALLBACK);
}

inline charinfo *charinfo::get_translation(bool for_transparent_throughput)
{
  return ((for_transparent_throughput && !is_transparently_translatable)
	  ? 0 /* nullptr */
	  : translation);
}

inline unsigned char charinfo::get_hyphenation_code()
{
  return hyphenation_code;
}

inline unsigned char charinfo::get_ascii_code()
{
  return ascii_code;
}

inline unsigned char charinfo::get_asciify_code()
{
  return (translatable_as_input ? asciify_code : 0U);
}

inline void charinfo::set_flags(unsigned int c)
{
  flags = c;
}

inline glyph *charinfo::as_glyph()
{
  return this;
}

inline void charinfo::make_translatable_as_input()
{
  translatable_as_input = true;
}

inline bool charinfo::is_translatable_as_input()
{
  return translatable_as_input;
}

inline int charinfo::get_special_translation(bool transparently)
{
  return (transparently && !is_transparently_translatable
	  ? int(TRANSLATE_NONE)
	  : special_translation);
}

inline macro *charinfo::get_macro()
{
  return mac;
}

inline bool charinfo::first_time_not_found()
{
  if (is_not_found)
    return false;
  else {
    is_not_found = true;
    return true;
  }
}

inline symbol *charinfo::get_symbol()
{
  return &nm;
}

inline void charinfo::add_to_class(int c)
{
  using_character_classes = true;
  // TODO ranges cumbersome for single characters?
  ranges.push_back(std::pair<int, int>(c, c));
}

inline void charinfo::add_to_class(int lo,
				   int hi)
{
  using_character_classes = true;
  ranges.push_back(std::pair<int, int>(lo, hi));
}

inline void charinfo::add_to_class(charinfo *ci)
{
  using_character_classes = true;
  nested_classes.push_back(ci);
}

inline bool charinfo::is_class()
{
  return (!ranges.empty() || !nested_classes.empty());
}

// Local Variables:
// fill-column: 72
// mode: C++
// End:
// vim: set cindent noexpandtab shiftwidth=2 textwidth=72:

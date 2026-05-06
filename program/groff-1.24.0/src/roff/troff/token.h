/* Copyright 1989-2020 Free Software Foundation, Inc.
             2021-2025 G. Branden Robinson

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

class charinfo;
struct node;

enum delimiter_context {
  DELIMITER_GROFF_EXPRESSION,
  DELIMITER_ATT_STRING_EXPRESSION,
  DELIMITER_ATT_NUMERIC_EXPRESSION,
  DELIMITER_ATT_OUTPUT_COMPARISON_EXPRESSION
};

class token {
  symbol nm;
  node *nd;
  unsigned char c;
  int val;
  units dim;
  enum token_type {
    TOKEN_BACKSPACE,		// ^H
    TOKEN_BEGIN_TRAP,
    TOKEN_CHAR,			// ordinary character
    TOKEN_DELIMITED_HORIZONTAL_MOTION,	// \h
    TOKEN_DELIMITED_SPECIAL_CHAR,	// \C
    TOKEN_DUMMY,		// dummy character: \&
    TOKEN_EMPTY,		// this is the initial value
    TOKEN_END_TRAP,
    TOKEN_EOF,			// end of file
    TOKEN_ESCAPE,		// \e
    TOKEN_HORIZONTAL_MOTION,	// fixed horizontal motion: \|, \^, \0
    TOKEN_HYPHEN_INDICATOR,	// \%
    TOKEN_INDEXED_CHAR,		// \N
    TOKEN_INTERRUPT,		// \c
    TOKEN_ITALIC_CORRECTION,	// \/
    TOKEN_LEADER,		// ^A
    TOKEN_LEFT_BRACE,		// \{
    TOKEN_MARK_INPUT,		// \k
    TOKEN_NEWLINE,		// ^J
    TOKEN_NODE,
    TOKEN_PAGE_EJECTOR,
    TOKEN_REQUEST,
    TOKEN_RIGHT_BRACE,		// \}
    TOKEN_SPACE,		// ' ' -- ordinary space
    TOKEN_SPECIAL_CHAR,		// \(, \[
    TOKEN_SPREAD,		// \p -- break and spread output line
    TOKEN_STRETCHABLE_SPACE,	// \~
    TOKEN_TAB,			// ^I
    TOKEN_TRANSPARENT,		// \!
    TOKEN_TRANSPARENT_DUMMY,	// \)
    TOKEN_UNSTRETCHABLE_SPACE,	// '\ '
    TOKEN_ZERO_WIDTH_BREAK	// \:
  } type;
public:
  token();
  ~token();
  token(const token &);
  void operator=(const token &);
  void next();
  void process();
  void skip_spaces();
  void diagnose_non_character();
  int nspaces();		// is_space() as integer
  bool is_node();
  bool is_eof();
  bool is_space();
  bool is_stretchable_space();
  bool is_unstretchable_space();
  bool is_horizontal_motion();
  bool is_horizontal_whitespace();
  bool is_any_character();
  // XXX: Do we need a `is_ordinary_character()`?
  bool is_special_character();
  bool is_indexed_character();
  bool is_newline();
  bool is_tab();
  bool is_leader();
  bool is_backspace();
  bool is_usable_as_delimiter(bool /* report_error */ = false,
    enum delimiter_context /* context */ = DELIMITER_GROFF_EXPRESSION);
  bool is_dummy();
  bool is_transparent_dummy();
  bool is_transparent();
  bool is_left_brace();
  bool is_right_brace();
  bool is_page_ejector();
  bool is_hyphen_indicator();
  bool is_zero_width_break();
  bool operator==(const token &); // for delimiters & conditional exprs
  bool operator!=(const token &); // ditto
  unsigned char ch();
  int character_index();
  charinfo *get_charinfo(bool /* required */ = false,
			 bool /* suppress_creation */ = false);
  bool add_to_zero_width_node_list(node **);
  void make_space();
  void make_newline();
  void describe_node(char * /* buf */, size_t /* bufsz */);
  const char *description();

  friend void process_input_stack();
  friend node *do_overstrike();
};

extern token tok;		// the current token

extern bool has_arg(bool /* want_peek */ = false);
extern void skip_line();
extern symbol read_identifier(bool /* required */ = false);
extern symbol read_long_identifier(bool /* required */ = false);
extern void handle_initial_title();
extern charinfo *read_character(); // TODO?: bool /* required */ = false
extern char *read_rest_of_line_as_argument();

enum char_mode {
  CHAR_NORMAL,
  CHAR_FALLBACK,
  CHAR_FONT_SPECIFIC_FALLBACK,
  CHAR_SPECIAL_FALLBACK
};

extern void define_character(char_mode,
			     const char * /* font_name */ = 0 /* nullptr */);

class hunits;
extern void read_title_parts(node **part, hunits *part_width);

extern bool read_measurement(units * /* result */,
			     unsigned char /* scale indicator */,
			     bool /* is_mandatory */ = false);
extern bool read_integer(int *result);

extern bool read_measurement(units *result, unsigned char si,
			     units prev_value);
extern bool read_integer(int *result, int prev_value);

extern void interpolate_register(symbol, int);

inline bool token::is_newline()
{
  return (TOKEN_NEWLINE == type);
}

inline bool token::is_space()
{
  return (TOKEN_SPACE == type);
}

inline bool token::is_stretchable_space()
{
  return (TOKEN_STRETCHABLE_SPACE == type);
}

inline bool token::is_unstretchable_space()
{
  return (TOKEN_UNSTRETCHABLE_SPACE == type);
}

inline bool token::is_horizontal_motion()
{
  return ((TOKEN_HORIZONTAL_MOTION == type)
	  || (TOKEN_DELIMITED_HORIZONTAL_MOTION == type));
}

inline bool token::is_special_character()
{
  return ((TOKEN_SPECIAL_CHAR == type)
	  || (TOKEN_DELIMITED_SPECIAL_CHAR == type));
}

inline int token::nspaces()
{
  return int(TOKEN_SPACE == type);
}

inline bool token::is_horizontal_whitespace()
{
  return (TOKEN_SPACE == type || TOKEN_TAB == type);
}

inline bool token::is_transparent()
{
  return (TOKEN_TRANSPARENT == type);
}

inline bool token::is_page_ejector()
{
  return (TOKEN_PAGE_EJECTOR == type);
}

inline unsigned char token::ch()
{
  return ((TOKEN_CHAR == type) ? c : 0U); // TODO: grochar
}

inline bool token::is_any_character()
{
  return ((TOKEN_CHAR == type)
	  || (TOKEN_SPECIAL_CHAR == type)
	  || (TOKEN_DELIMITED_SPECIAL_CHAR == type)
	  || (TOKEN_INDEXED_CHAR == type));
}

inline bool token::is_indexed_character()
{
  return (TOKEN_INDEXED_CHAR == type);
}

inline int token::character_index()
{
  assert(TOKEN_INDEXED_CHAR == type);
  return val;
}

inline bool token::is_node()
{
  return (TOKEN_NODE == type);
}

inline bool token::is_eof()
{
  return (TOKEN_EOF == type);
}

inline bool token::is_dummy()
{
  return (TOKEN_DUMMY == type);
}

inline bool token::is_transparent_dummy()
{
  return (TOKEN_TRANSPARENT_DUMMY == type);
}

inline bool token::is_left_brace()
{
  return (TOKEN_LEFT_BRACE == type);
}

inline bool token::is_right_brace()
{
  return (TOKEN_RIGHT_BRACE == type);
}

inline bool token::is_tab()
{
  return (TOKEN_TAB == type);
}

inline bool token::is_leader()
{
  return (TOKEN_LEADER == type);
}

inline bool token::is_backspace()
{
  return (TOKEN_BACKSPACE == type);
}

inline bool token::is_hyphen_indicator()
{
  return (TOKEN_HYPHEN_INDICATOR == type);
}

inline bool token::is_zero_width_break()
{
  return (TOKEN_ZERO_WIDTH_BREAK == type);
}

// Local Variables:
// fill-column: 72
// mode: C++
// End:
// vim: set cindent noexpandtab shiftwidth=2 textwidth=72:

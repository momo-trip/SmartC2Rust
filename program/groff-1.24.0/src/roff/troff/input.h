/* Copyright 2001-2024 Free Software Foundation, Inc.
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


// GNU troff uses these character codes for its own purposes.
//
// Except for the ones named `INPUT_`, they are not valid as input.

const int ESCAPE_QUESTION = 015;	// \u000d
const int BEGIN_TRAP = 016;		// \u000e
const int END_TRAP = 017;		// \u000f
const int PAGE_EJECTOR = 020;		// \u0010
const int ESCAPE_NEWLINE = 021;		// \u0011
const int ESCAPE_AMPERSAND = 022;	// \u0012
const int ESCAPE_UNDERSCORE = 023;	// \u0013
const int ESCAPE_BAR = 024;		// \u0014
const int ESCAPE_CIRCUMFLEX = 025;	// \u0015
const int ESCAPE_LEFT_BRACE = 026;	// \u0016
const int ESCAPE_RIGHT_BRACE = 027;	// \u0017
const int ESCAPE_LEFT_QUOTE = 030;	// \u0018
const int ESCAPE_RIGHT_QUOTE = 031;	// \u0019
const int ESCAPE_HYPHEN = 032;		// \u001a
const int ESCAPE_BANG = 033;		// \u001b
const int ESCAPE_c = 034;		// \u001c
const int ESCAPE_e = 035;		// \u001d
const int ESCAPE_PERCENT = 036;		// \u001e
const int ESCAPE_SPACE = 037;		// \u001f

const int INPUT_DELETE = 0177;		// \u007f

const int TITLE_REQUEST = 0200;			// \u0080
const int COPY_FILE_REQUEST = 0201;		// \u0081
const int TRANSPARENT_FILE_REQUEST = 0202;	// \u0082
#ifdef COLUMN
const int VJUSTIFY_REQUEST = 0203;		// \u0083
#endif /* COLUMN */
const int ESCAPE_E = 0204;			// \u0084
const int LAST_PAGE_EJECTOR = 0205;		// \u0085
const int ESCAPE_RIGHT_PARENTHESIS = 0206;	// \u0086
const int ESCAPE_TILDE = 0207;			// \u0087
const int ESCAPE_COLON = 0210;			// \u0088
const int PUSH_GROFF_MODE = 0211;		// \u0089
const int PUSH_COMP_MODE = 0212;		// \u008a
const int POP_GROFFCOMP_MODE = 0213;		// \u008b
const int BEGIN_QUOTE = 0214;			// \u008c
const int END_QUOTE = 0215;			// \u008d
const int DOUBLE_QUOTE = 0216;			// \u008e

const int INPUT_NO_BREAK_SPACE = 0240;		// \u00a3

const int INPUT_SOFT_HYPHEN = 0255;		// \u00ad

extern void do_stroke_color(symbol);
extern void do_fill_color(symbol);

extern const char *encode_for_stream_output(int c);
extern bool was_invoked_with_regular_control_character;
extern bool suppress_push;
extern bool want_nodes_dumped;

// Local Variables:
// fill-column: 72
// mode: C++
// End:
// vim: set cindent noexpandtab shiftwidth=2 textwidth=72:

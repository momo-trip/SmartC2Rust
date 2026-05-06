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

class statem;

struct size_range {
  int min;
  int max;
};

class font_size {
  static size_range *size_list;
  static int nranges;
  int p;
public:
  font_size();
  font_size(int points);
  int to_points();
  int to_scaled_points();
  int to_units();
  int operator==(font_size);
  int operator!=(font_size);
  static void init_size_list(int * /* sizes */);
  static void dump_size_list();
};

inline font_size::font_size() : p(0)
{
}

inline int font_size::operator==(font_size fs)
{
  return p == fs.p;
}

inline int font_size::operator!=(font_size fs)
{
  return p != fs.p;
}

inline int font_size::to_scaled_points()
{
  return p;
}

inline int font_size::to_points()
{
  return (p / sizescale);
}

class environment;

hunits env_digit_width(environment *);
hunits env_space_width(environment *);
hunits env_sentence_space_width(environment *);
hunits env_narrow_space_width(environment *);
hunits env_half_narrow_space_width(environment *);
int env_get_zoom(environment *);

struct tab;

enum tab_type { TAB_NONE, TAB_LEFT, TAB_CENTER, TAB_RIGHT };

class tab_stops {
  tab *initial_list;
  tab *repeated_list;
public:
  tab_stops();
  tab_stops(hunits /* distance */, tab_type /* type */);
  tab_stops(const tab_stops &);
  ~tab_stops();
  void operator=(const tab_stops &);
  tab_type distance_to_next_tab(hunits /* pos */,
				hunits * /* distance */);
  tab_type distance_to_next_tab(hunits /* curpos */,
				hunits * /* distance */,
				hunits * /* leftpos */);
  void clear();
  void add_tab(hunits /* pos */, tab_type /* type */,
	       bool /* is_repeated */);
  const char *to_string();
};

class charinfo;
struct node;
struct breakpoint;
class font_family;
class pending_output_line;

// declarations to avoid friend name injection problems
void title_length();
void space_size();
void fill();
void no_fill();
void adjust();
void no_adjust();
void center();
void right_justify();
void vertical_spacing();
void post_vertical_spacing();
void line_spacing();
void line_length();
void indent();
void temporary_indent();
void configure_underlining(bool);
void do_input_trap(bool);
void set_tabs();
void margin_character();
void no_number();
void number_lines();
void leader_character();
void tab_character();
void hyphenate_request();
void no_hyphenate();
void hyphen_line_max_request();
void hyphenation_space_request();
void hyphenation_margin_request();
void line_width();
#if 0
void tabs_save();
void tabs_restore();
#endif
void line_tabs_request();
void title();
#ifdef WIDOW_CONTROL
void widow_control_request();
#endif /* WIDOW_CONTROL */

class environment {
  bool is_dummy_env;			// dummy environment used for \w
  hunits prev_line_length;
  hunits line_length;
  hunits prev_title_length;
  hunits title_length;
  font_size prev_size;
  font_size size;
  int requested_size;
  int prev_requested_size;
  int char_height;
  int char_slant;
  int prev_fontno;
  int fontno;
  font_family *prev_family;
  font_family *family;
  int space_size;		// in 36ths of an em
  int sentence_space_size;	// same but for spaces at the end of sentences
  int adjust_mode;
  bool is_filling;
  bool was_line_interrupted;
  int was_previous_line_interrupted;	// three-valued Boolean :-|
  int centered_line_count;
  int right_aligned_line_count;
  vunits prev_vertical_spacing;
  vunits vertical_spacing;
  vunits prev_post_vertical_spacing;
  vunits post_vertical_spacing;
  int prev_line_spacing;
  int line_spacing;
  hunits prev_indent;
  hunits indent;
  hunits temporary_indent;
  bool have_temporary_indent;
  hunits saved_indent;
  hunits target_text_length;
  int pre_underline_fontno;
  int underlined_line_count;
  bool underline_spaces;
  symbol input_trap;
  int input_trap_count;
  bool continued_input_trap;
  node *line;			// in reverse order
  hunits prev_text_length;
  hunits width_total;
  int space_total;
  hunits input_line_start;
  node *tab_contents;
  hunits tab_width;
  hunits tab_distance;
  bool using_line_tabs;
  tab_type current_tab;
  node *leader_node;
  charinfo *tab_char;
  charinfo *leader_char;
  bool has_current_field;
  hunits field_distance;
  hunits pre_field_width;
  int field_spaces;
  int tab_field_spaces;
  bool tab_precedes_field;
  bool is_discarding;
  bool is_spreading;		// set by \p
  unsigned char margin_character_flags;
  node *margin_character_node;
  hunits margin_character_distance;
  node *numbering_nodes;
  hunits line_number_digit_width;
  int number_text_separation;	// in digit spaces
  int line_number_indent;	// in digit spaces
  int line_number_multiple;
  int no_number_count;
  unsigned int hyphenation_mode;
  unsigned int hyphenation_mode_default;
  int hyphen_line_count;
  int hyphen_line_max;
  hunits hyphenation_space;
  hunits hyphenation_margin;
  bool composite;	// used for construction of composite character
  pending_output_line *pending_lines;
#ifdef WIDOW_CONTROL
  bool want_widow_control;
#endif /* WIDOW_CONTROL */
  color *stroke_color;
  color *prev_stroke_color;
  color *fill_color;
  color *prev_fill_color;
  unsigned char control_character;
  unsigned char no_break_control_character;

  tab_type distance_to_next_tab(hunits *);
  tab_type distance_to_next_tab(hunits * /* distance */,
				hunits * /* leftpos */);
  void start_line();
  void output_line(node * /* nd */, hunits /* width */,
		   bool /* was_centered */);
  void output(node * /* nd */, bool /* suppress_filling */,
	      vunits /* vs */, vunits /* post_vs */, hunits /* width */,
	      bool /* was_centered */);
  void output_title(node * /* nd */, bool /* suppress_filling */,
		    vunits /* vs */, vunits /* post_vs */,
		    hunits /* width */);
#ifdef WIDOW_CONTROL
  void mark_last_line();
#endif /* WIDOW_CONTROL */
  breakpoint *choose_breakpoint();
  void possibly_hyphenate_line(bool /* must_break_here */ = false);
  void start_field();
  void wrap_up_field();
  void add_padding();
  node *make_tab_node(hunits /* d */,
		      node * /* next */ = 0 /* nullptr */);
  node *get_prev_char();
public:
  // C++11: Use `enum : unsigned char`.
  enum {
    MC_ON = 0x1,
    MC_NEXT = 0x2
  };
  bool seen_space;
  bool seen_eol;
  bool suppress_next_eol;
  bool seen_break;
  tab_stops tabs;
  const symbol name;
  charinfo *hyphen_indicator_char;

  environment(symbol);
  environment(const environment *);	// for temporary environment
  ~environment();
  unsigned char get_control_character();
  bool set_control_character(unsigned char);
  unsigned char get_no_break_control_character();
  bool set_no_break_control_character(unsigned char);
  statem *construct_state(bool has_only_eol);
  void dump();
  void copy(const environment *);
  bool is_dummy() { return is_dummy_env; }
  bool is_empty();
  bool is_composite() { return composite; }
  void set_composite() { composite = true; }
  vunits get_vertical_spacing();	// .v
  vunits get_post_vertical_spacing();	// .pvs
  int get_line_spacing();		// .L
  vunits total_post_vertical_spacing();
  int get_point_size() { return size.to_scaled_points(); }
  font_size get_font_size() { return size; }
  int get_size() { return size.to_units(); }
  int get_requested_point_size() { return requested_size; }
  int get_char_height() { return char_height; }
  int get_char_slant() { return char_slant; }
  hunits get_digit_width();
  int get_font() { return fontno; };	// .f
  int get_zoom();			// .zoom
  int get_numbering_nodes();		// .nm
  font_family *get_family() { return family; }
  hunits get_emboldening_offset();	// .b
  unsigned get_adjust_mode();		// .j
  int get_fill();			// .u
  hunits get_indent();			// .i
  hunits get_temporary_indent();
  hunits get_line_length();		// .l
  hunits get_saved_line_length();	// .ll
  hunits get_saved_indent();		// .in
  hunits get_title_length();
  hunits get_prev_char_width();		// .w
  hunits get_prev_char_skew();
  vunits get_prev_char_height();
  vunits get_prev_char_depth();
  hunits get_text_length();		// .k 
  hunits get_prev_text_length();	// .n
  hunits get_space_width() { return env_space_width(this); }
  int get_space_size() { return space_size; }	// in ems/36
  int get_sentence_space_size() { return sentence_space_size; }
  hunits get_narrow_space_width() { return env_narrow_space_width(this); }
  hunits get_half_narrow_space_width()
    { return env_half_narrow_space_width(this); }
  hunits get_input_line_position();
  const char *get_tabs();
  int is_using_line_tabs();
  unsigned get_hyphenation_mode();
  unsigned get_hyphenation_mode_default();
  int get_hyphen_line_max();
  int get_hyphen_line_count();
  hunits get_hyphenation_space();
  hunits get_hyphenation_margin();
  int get_centered_line_count();
  int get_input_trap_line_count();
  int get_input_trap_respects_continuation();
  const char *get_input_trap_macro();
  int get_right_aligned_line_count();
  int get_no_number_count();
  bool get_was_line_interrupted() { return was_line_interrupted; }
  int get_was_previous_line_interrupted() { return was_previous_line_interrupted; }
  color *get_fill_color();
  color *get_stroke_color();
  color *get_prev_stroke_color();
  color *get_prev_fill_color();
  void set_stroke_color(color *c);
  void set_fill_color(color *c);
  node *make_char_node(charinfo *);
  node *extract_output_line();
  void width_registers();
  void wrap_up_tab();
  bool set_font(int);
  bool set_font(symbol);
  void set_family(symbol);
  void set_size(int);
  void set_char_height(int);
  void set_char_slant(int);
  void set_input_line_position(hunits);	// used by \n(hp
  void interrupt();
  void spread() { is_spreading = true; }
  void possibly_break_line(bool /* must_break_here */ = false,
			   bool /* must_adjust */ = false);
  void do_break(bool /* want_adjustment */ = false);	// .br, .brp
  void final_break();
  node *make_tag(const char *name, int i);
  void newline();
  void advance_to_tab_stop(bool /* use_leader */ = false);
  void add_node(node *);
  void add_char(charinfo *);
  void add_hyphen_indicator();
  void add_italic_correction();
  void space();
  void space(hunits, hunits);
  void space_newline();
  const char *get_stroke_color_string();
  const char *get_fill_color_string();
  const char *get_prev_stroke_color_string();
  const char *get_prev_fill_color_string();
  const char *get_font_family_string();
  const char *get_font_name_string();
  const char *get_style_name_string();
  const char *get_name_string();
  const char *get_point_size_string();
  const char *get_requested_point_size_string();
  void output_pending_lines();
  void construct_format_state(node * /* nd */, bool /* was_centered */,
			      int /* fill */);
  void construct_new_line_state(node *n);
  void dump_troff_state();
  void dump_pending_nodes();

  friend void title_length();
  friend void space_size();
  friend void fill();
  friend void no_fill();
  friend void adjust();
  friend void no_adjust();
  friend void center();
  friend void right_justify();
  friend void vertical_spacing();
  friend void post_vertical_spacing();
  friend void line_spacing();
  friend void line_length();
  friend void indent();
  friend void temporary_indent();
  friend void configure_underlining(bool);
  friend void do_input_trap(bool);
  friend void set_tabs();
  friend void margin_character();
  friend void no_number();
  friend void number_lines();
  friend void leader_character_request();
  friend void tab_character_request();
  friend void hyphenate_request();
  friend void set_hyphenation_mode_default();
  friend void no_hyphenate();
  friend void hyphen_line_max_request();
  friend void hyphenation_space_request();
  friend void hyphenation_margin_request();
  friend void line_width();
#if 0
  friend void tabs_save();
  friend void tabs_restore();
#endif
  friend void line_tabs_request();
  friend void title();
#ifdef WIDOW_CONTROL
  friend void widow_control_request();
#endif /* WIDOW_CONTROL */
  friend void do_divert(bool /* appending */, bool /* boxing */);
};

extern environment *curenv;
extern void pop_env();
extern void push_env(int);

void select_font(symbol);
void init_environments();

extern double spread_limit;

extern symbol default_family;
extern bool translate_space_to_dummy;

extern unsigned char hpf_code_table[];

// Local Variables:
// fill-column: 72
// mode: C++
// End:
// vim: set cindent noexpandtab shiftwidth=2 textwidth=72:

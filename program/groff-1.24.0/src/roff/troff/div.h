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

void do_divert(bool /* appending */, bool /* boxing */);
void end_diversions();

class diversion {
  friend void do_divert(bool /* appending */, bool /* boxing */);
  friend void end_diversions();
  diversion *prev;
  node *saved_line;
  hunits saved_width_total;
  int saved_space_total;
  hunits saved_saved_indent;
  hunits saved_target_text_length;
  int saved_was_previous_line_interrupted; // three-valued Boolean :-|
protected:
  symbol nm;
  vunits vertical_position;
  vunits high_water_mark;
public:
  bool is_box;
  bool is_in_no_space_mode;
  bool saved_seen_break; // XXX: nilpotent?
  bool saved_seen_space; // XXX: nilpotent?
  bool saved_seen_eol; // XXX: nilpotent?
  bool saved_suppress_next_eol; // XXX: nilpotent?
  state_set modified_tag;
  vunits marked_place;
  diversion(symbol /* s */ = NULL_SYMBOL, bool /* boxing */ = false);
  virtual ~diversion();
  virtual void output(node * /* nd */, bool /* retain_size */,
		      vunits /* vs */, vunits /* post_vs */,
		      hunits /* width */) = 0;
  virtual void transparent_output(unsigned char) = 0;
  virtual void transparent_output(node *) = 0;
  virtual void space(vunits distance, bool /* forcing */ = 0) = 0;
#ifdef COLUMN
  virtual void vjustify(symbol) = 0;
#endif /* COLUMN */
  vunits get_vertical_position() { return vertical_position; }
  vunits get_high_water_mark() { return high_water_mark; }
  virtual vunits distance_to_next_trap() = 0;
  virtual const char * get_next_trap_name() = 0;
  void need(vunits);
  const char *get_diversion_name() { return nm.contents(); }
  virtual void set_diversion_trap(symbol, vunits) = 0;
  virtual void clear_diversion_trap() = 0;
  virtual void copy_file(const char *filename) = 0;
  virtual bool is_diversion() = 0;
};

class macro;

class macro_diversion : public diversion {
  macro *mac;
  hunits max_width;
  symbol diversion_trap;
  vunits diversion_trap_pos;
public:
  macro_diversion(symbol, bool /* appending */, bool /* boxing */);
  ~macro_diversion();
  void output(node * /* nd */, bool /* retain_size */, vunits /* vs */,
	      vunits /* post_vs */, hunits /* width */);
  void transparent_output(unsigned char);
  void transparent_output(node *);
  void space(vunits distance, bool /* forcing */ = 0);
#ifdef COLUMN
  void vjustify(symbol);
#endif /* COLUMN */
  vunits distance_to_next_trap();
  const char *get_next_trap_name();
  void set_diversion_trap(symbol, vunits);
  void clear_diversion_trap();
  void copy_file(const char *filename);
  bool is_diversion() { return true; }
};

struct trap {
  trap *next;
  vunits position;
  symbol nm;
  trap(symbol, vunits, trap *);
};

class output_file;

class top_level_diversion : public diversion {
  int page_number;
  int page_count;
  int last_page_count;
  vunits page_length;
  hunits prev_page_offset;
  hunits page_offset;
  trap *page_trap_list;
  trap *find_next_trap(vunits *);
  bool overriding_next_page_number;
  int next_page_number;
  bool ejecting_page;
public:
  int before_first_page_status; // three-valued Boolean :-|
  top_level_diversion();
  void output(node * /* nd */, bool /* retain_size */, vunits /* vs */,
	      vunits /* post_vs */, hunits /* width */);
  void transparent_output(unsigned char);
  void transparent_output(node *);
  void space(vunits distance, bool /* forcing */ = false);
#ifdef COLUMN
  void vjustify(symbol);
#endif /* COLUMN */
  hunits get_page_offset() { return page_offset; }
  hunits get_previous_page_offset() { return prev_page_offset; }
  void set_page_offset(hunits /* h */);
  vunits get_page_length() { return page_length; }
  vunits distance_to_next_trap();
  const char *get_next_trap_name();
  void add_trap(symbol nm, vunits pos);
  void change_trap(symbol nm, vunits pos);
  void remove_trap(symbol);
  void remove_trap_at(vunits pos);
  void print_traps();
  int get_page_count() { return page_count; }
  int get_page_number() { return page_number; }
  int get_next_page_number();
  void set_page_number(int n) { page_number = n; }
  bool begin_page(vunits = V0);
  void set_next_page_number(int);
  void set_page_length(vunits);
  void copy_file(const char *filename);
  bool get_ejecting() { return ejecting_page; }
  void set_ejecting() { ejecting_page = true; }
  friend void page_offset();
  void set_diversion_trap(symbol, vunits);
  void clear_diversion_trap();
  void set_last_page() { last_page_count = page_count; }
  bool is_diversion() { return false; }
};

inline void top_level_diversion::set_page_offset(hunits h)
{
  prev_page_offset = page_offset;
  page_offset = h;
}

extern top_level_diversion *topdiv;
extern diversion *curdiv;

extern bool is_exit_underway;
extern bool is_eoi_macro_finished;
extern bool seen_last_page_ejector;
extern int last_page_number;

void spring_trap(symbol);	// implemented by input.c
extern bool was_trap_sprung;
void postpone_traps();
bool unpostpone_traps();

void push_page_ejector();
void continue_page_eject();
void handle_first_page_transition();
void blank_line();

extern void write_any_trailer_and_exit(int /* exit_code */);

// Local Variables:
// fill-column: 72
// mode: C++
// End:
// vim: set cindent noexpandtab shiftwidth=2 textwidth=72:

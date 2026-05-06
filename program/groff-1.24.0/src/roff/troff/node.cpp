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

#include <errno.h>
#include <stdlib.h> // free(), malloc()
#include <string.h> // strerror()

#include "troff.h"
#include "dictionary.h"
#include "hvunits.h"
#include "stringclass.h"
#include "mtsm.h"
#include "env.h"
#include "request.h"
#include "node.h"
#include "token.h"
#include "div.h"
#include "reg.h"
#include "font.h"
#include "charinfo.h"
#include "input.h"
#include "geometry.h"
#include "json-encode.h" // json_encode_char()

#include "posix.h"
#include "nonposix.h"

#ifdef _POSIX_VERSION

#include <sys/wait.h>

#else /* not _POSIX_VERSION */

/* traditional Unix */

#define WIFEXITED(s) (((s) & 0377) == 0)
#define WEXITSTATUS(s) (((s) >> 8) & 0377)
#define WTERMSIG(s) ((s) & 0177)
#define WIFSTOPPED(s) (((s) & 0377) == 0177)
#define WSTOPSIG(s) (((s) >> 8) & 0377)
#define WIFSIGNALED(s) (((s) & 0377) != 0 && (((s) & 0377) != 0177))

#endif /* not _POSIX_VERSION */

#include <stack>

static bool is_output_supressed = false;

// declarations to avoid friend name injections
class tfont;
class tfont_spec;
tfont *make_tfont(tfont_spec &);


/*
 *  how many boundaries of images have been written? Useful for
 *  debugging grohtml
 */

int image_no = 0;
static int suppression_starting_page_number = 0;
static bool was_any_page_in_output_list = false;

#define STORE_WIDTH 1

symbol HYPHEN_SYMBOL("hy");

// Character used when a hyphen is inserted at a line break.
static charinfo *soft_hyphen_char;

enum constant_space_type {
  CONSTANT_SPACE_NONE,
  CONSTANT_SPACE_RELATIVE,
  CONSTANT_SPACE_ABSOLUTE
  };

struct special_font_list {
  int n;
  special_font_list *next;
};

special_font_list *global_special_fonts;
static int global_ligature_mode = 1; // three-valued Boolean :-|
static bool global_kern_mode = true;
// Font mounting positions are non-negative integers.
const int FONT_NOT_MOUNTED = -1;

class track_kerning_function {
  int non_zero;
  units min_size;
  hunits min_amount;
  units max_size;
  hunits max_amount;
public:
  track_kerning_function();
  track_kerning_function(units, hunits, units, hunits);
  int operator==(const track_kerning_function &);
  int operator!=(const track_kerning_function &);
  hunits compute(int point_size);
};

struct font_lookup_info {
  int position;
  int requested_position;
  char *requested_name;
  font_lookup_info();
};

font_lookup_info::font_lookup_info() : position(FONT_NOT_MOUNTED),
  requested_position(FONT_NOT_MOUNTED), requested_name(0)
{
}

// embolden fontno when this is the current font

struct conditional_bold {
  conditional_bold *next;
  int fontno;
  hunits offset;
  conditional_bold(int, hunits, conditional_bold * = 0 /* nullptr */);
};

class font_info {
  tfont *last_tfont;
  int number;
  font_size last_size;
  int last_height;
  int last_slant;
  symbol internal_name;
  symbol external_name;
  font *fm;
  bool has_emboldening;
  hunits bold_offset;
  track_kerning_function track_kern;
  constant_space_type is_constant_spaced;
  units constant_space;
  int last_ligature_mode;
  int last_kern_mode;
  conditional_bold *cond_bold_list;
  void flush();
public:
  special_font_list *sf;
  font_info(symbol, int, symbol, font *);
  int contains(charinfo *);
  void set_bold(hunits);
  void unbold();
  void set_conditional_bold(int, hunits);
  void conditional_unbold(int);
  void set_track_kern(track_kerning_function &);
  void set_constant_space(constant_space_type, units = 0);
  int is_named(symbol);
  symbol get_name();
  tfont *get_tfont(font_size, int, int, int);
  hunits get_space_width(font_size, int);
  hunits get_narrow_space_width(font_size);
  hunits get_half_narrow_space_width(font_size);
  bool is_emboldened(hunits *); // "by how many hunits?" in argument
  int is_special();
  int is_style();
  void set_zoom(int);
  int get_zoom();
  font *get_font() const;
  friend symbol get_font_name(int, environment *);
  friend symbol get_style_name(int);
};

class tfont_spec {
protected:
  symbol name;
  int input_position;
  font *fm;
  font_size size;
  bool has_emboldening;
  bool has_constant_spacing;
  int ligature_mode;
  int kern_mode;
  hunits bold_offset;
  hunits track_kern;			// add this to the width
  hunits constant_space_width;
  int height;
  int slant;
public:
  tfont_spec(symbol, int, font *, font_size, int, int);
  tfont_spec plain();
  bool operator==(const tfont_spec &);
  friend tfont *font_info::get_tfont(font_size fs, int, int, int);
};

class tfont : public tfont_spec {
  static tfont *tfont_list;
  tfont *next;
  tfont *plain_version;
public:
  tfont(tfont_spec &);
  int contains(charinfo *);
  hunits get_width(charinfo *c);
  bool is_emboldened(hunits *); // "by how many hunits?" in argument
  bool is_constantly_spaced(hunits *); // "by how many hunits?" in arg
  hunits get_track_kern();
  tfont *get_plain();
  font_size get_size();
  int get_zoom();
  symbol get_name();
  charinfo *get_lig(charinfo *c1, charinfo *c2);
  bool is_kerned(charinfo *c1, charinfo *c2, hunits *res);
  int get_input_position();
  int get_character_type(charinfo *);
  int get_height();
  int get_slant();
  vunits get_char_height(charinfo *);
  vunits get_char_depth(charinfo *);
  hunits get_char_skew(charinfo *);
  hunits get_italic_correction(charinfo *);
  hunits get_left_italic_correction(charinfo *);
  hunits get_subscript_correction(charinfo *);
  friend tfont *make_tfont(tfont_spec &);
};

static inline int env_resolve_font(environment *env)
{
  return env->get_family()->resolve(env->get_font());
}

/* font_info functions */

static font_info **font_table = 0 /* nullptr */;
static int font_table_size = 0;

font_info::font_info(symbol nm, int n, symbol enm, font *f)
: last_tfont(0 /* nullptr */), number(n), last_size(0),
  internal_name(nm), external_name(enm), fm(f),
  has_emboldening(false), is_constant_spaced(CONSTANT_SPACE_NONE),
  last_ligature_mode(1), last_kern_mode(1),
  cond_bold_list(0 /* nullptr */), sf(0 /* nullptr */)
{
}

inline int font_info::contains(charinfo *ci)
{
  return (fm != 0 /* nullptr */) && fm->contains(ci->as_glyph());
}

inline int font_info::is_special()
{
  return (fm != 0 /* nullptr */) && fm->is_special();
}

inline int font_info::is_style()
{
  return (0 /* nullptr */ == fm);
}

void font_info::set_zoom(int zoom)
{
  assert(fm != 0 /* nullptr */);
  fm->set_zoom(zoom);
}

inline int font_info::get_zoom()
{
  if (is_style())
    return 0;
  return fm->get_zoom();
}

font *font_info::get_font() const
{
  return fm;
}

tfont *make_tfont(tfont_spec &spec)
{
  for (tfont *p = tfont::tfont_list; p != 0 /* nullptr */; p = p->next)
    if (*p == spec)
      return p;
  return new tfont(spec);
}

int env_get_zoom(environment *env)
{
  int fontno = env->get_family()->resolve(env->get_font());
  return font_table[fontno]->get_zoom();
}

// this is the current_font, fontno is where we found the character,
// presumably a special font

tfont *font_info::get_tfont(font_size fs, int height, int slant,
			    int fontno)
{
  if (0 /* nullptr */ == last_tfont
      || fs != last_size
      || height != last_height
      || slant != last_slant
      || global_ligature_mode != last_ligature_mode
      || global_kern_mode != last_kern_mode
      || fontno != number) {
	font_info *f = font_table[fontno];
	tfont_spec spec(f->external_name, f->number, f->fm, fs, height,
			slant);
	for (conditional_bold *p = cond_bold_list;
	     p != 0 /* nullptr */;
	     p = p->next)
	  if (p->fontno == fontno) {
	    spec.has_emboldening = true;
	    spec.bold_offset = p->offset;
	    break;
	  }
	if (!spec.has_emboldening && has_emboldening) {
	  spec.has_emboldening = true;
	  spec.bold_offset = bold_offset;
	}
	spec.track_kern = track_kern.compute(fs.to_scaled_points());
	spec.ligature_mode = global_ligature_mode;
	spec.kern_mode = global_kern_mode;
	switch (is_constant_spaced) {
	case CONSTANT_SPACE_NONE:
	  break;
	case CONSTANT_SPACE_ABSOLUTE:
	  spec.has_constant_spacing = true;
	  spec.constant_space_width = constant_space;
	  break;
	case CONSTANT_SPACE_RELATIVE:
	  spec.has_constant_spacing = true;
	  spec.constant_space_width
	    = scale(constant_space * fs.to_scaled_points(),
		    units_per_inch,
		    36 * 72 * sizescale);
	  break;
	default:
	  assert(0 == "unhandled case of constant spacing mode");
	}
	if (fontno != number)
	  return make_tfont(spec);
	// save font for comparison purposes
	last_tfont = make_tfont(spec);
	// save font related values not contained in tfont
	last_size = fs;
	last_height = height;
	last_slant = slant;
	last_ligature_mode = global_ligature_mode;
	last_kern_mode = global_kern_mode;
      }
  return last_tfont;
}

bool font_info::is_emboldened(hunits *res)
{
  if (has_emboldening) {
    *res = bold_offset;
    return true;
  }
  else
    return false;
}

void font_info::unbold()
{
  if (has_emboldening) {
    has_emboldening = false;
    flush();
  }
}

void font_info::set_bold(hunits offset)
{
  if (!has_emboldening || (offset != bold_offset)) {
    has_emboldening = true;
    bold_offset = offset;
    flush();
  }
}

void font_info::set_conditional_bold(int fontno, hunits offset)
{
  for (conditional_bold *p = cond_bold_list;
       p != 0 /* nullptr */;
       p = p->next)
    if (p->fontno == fontno) {
      if (offset != p->offset) {
	p->offset = offset;
	flush();
      }
      return;
    }
  cond_bold_list = new conditional_bold(fontno, offset, cond_bold_list);
}

conditional_bold::conditional_bold(int f, hunits h, conditional_bold *x)
: next(x), fontno(f), offset(h)
{
}

void font_info::conditional_unbold(int fontno)
{
  for (conditional_bold **p = &cond_bold_list;
       *p != 0 /* nullptr */;
       p = &(*p)->next)
    if ((*p)->fontno == fontno) {
      conditional_bold *tem = *p;
      *p = (*p)->next;
      delete tem;
      flush();
      return;
    }
}

void font_info::set_constant_space(constant_space_type type, units x)
{
  if (type != is_constant_spaced
      || (type != CONSTANT_SPACE_NONE && x != constant_space)) {
    flush();
    is_constant_spaced = type;
    constant_space = x;
  }
}

void font_info::set_track_kern(track_kerning_function &tk)
{
  if (track_kern != tk) {
    track_kern = tk;
    flush();
  }
}

void font_info::flush()
{
  last_tfont = 0;
}

int font_info::is_named(symbol s)
{
  return internal_name == s;
}

symbol font_info::get_name()
{
  return internal_name;
}

symbol get_font_name(int fontno, environment *env)
{
  symbol f = font_table[fontno]->get_name();
  if (font_table[fontno]->is_style()) {
    return concat(env->get_family()->nm, f);
  }
  return f;
}

symbol get_style_name(int fontno)
{
  if (font_table[fontno]->is_style())
    return font_table[fontno]->get_name();
  else
    return EMPTY_SYMBOL;
}

hunits font_info::get_space_width(font_size fs, int space_sz)
{
  if (is_constant_spaced == CONSTANT_SPACE_NONE)
    return scale(hunits(fm->get_space_width(fs.to_scaled_points())),
			space_sz, 12);
  else if (is_constant_spaced == CONSTANT_SPACE_ABSOLUTE)
    return constant_space;
  else
    return scale(constant_space*fs.to_scaled_points(),
		 units_per_inch, 36 * 72 * sizescale);
}

hunits font_info::get_narrow_space_width(font_size fs)
{
  charinfo *ci = lookup_charinfo(symbol("|"));
  if (fm->contains(ci->as_glyph()))
    return hunits(fm->get_width(ci->as_glyph(), fs.to_scaled_points()));
  else
    return hunits(fs.to_units()/6);
}

hunits font_info::get_half_narrow_space_width(font_size fs)
{
  charinfo *ci = lookup_charinfo(symbol("^"));
  if (fm->contains(ci->as_glyph()))
    return hunits(fm->get_width(ci->as_glyph(), fs.to_scaled_points()));
  else
    return hunits(fs.to_units()/12);
}

/* tfont */

tfont_spec::tfont_spec(symbol nm, int n, font *f,
		       font_size s, int h, int sl)
: name(nm), input_position(n), fm(f), size(s),
  has_emboldening(false), has_constant_spacing(false), ligature_mode(1),
  kern_mode(1), height(h), slant(sl)
{
  if (height == size.to_scaled_points())
    height = 0;
}

bool tfont_spec::operator==(const tfont_spec &spec)
{
  if (fm == spec.fm
      && size == spec.size
      && input_position == spec.input_position
      && name == spec.name
      && height == spec.height
      && slant == spec.slant
      && (has_emboldening
	  ? (spec.has_emboldening && bold_offset == spec.bold_offset)
	  : !spec.has_emboldening)
      && track_kern == spec.track_kern
      && (has_constant_spacing
	  ? (spec.has_constant_spacing
	     && constant_space_width == spec.constant_space_width)
	  : !spec.has_constant_spacing)
      && ligature_mode == spec.ligature_mode
      && kern_mode == spec.kern_mode)
    return true;
  else
    return false;
}

tfont_spec tfont_spec::plain()
{
  return tfont_spec(name, input_position, fm, size, height, slant);
}

hunits tfont::get_width(charinfo *c)
{
  if (has_constant_spacing)
    return constant_space_width;
  else if (has_emboldening)
    return (hunits(fm->get_width(c->as_glyph(),
		   size.to_scaled_points()))
	    + track_kern + bold_offset);
  else
    return (hunits(fm->get_width(c->as_glyph(),
		   size.to_scaled_points()))
	    + track_kern);
}

vunits tfont::get_char_height(charinfo *c)
{
  vunits v = fm->get_height(c->as_glyph(), size.to_scaled_points());
  if (height != 0 && height != size.to_scaled_points())
    return scale(v, height, size.to_scaled_points());
  else
    return v;
}

vunits tfont::get_char_depth(charinfo *c)
{
  vunits v = fm->get_depth(c->as_glyph(), size.to_scaled_points());
  if (height != 0 && height != size.to_scaled_points())
    return scale(v, height, size.to_scaled_points());
  else
    return v;
}

hunits tfont::get_char_skew(charinfo *c)
{
  return hunits(fm->get_skew(c->as_glyph(), size.to_scaled_points(),
			     slant));
}

hunits tfont::get_italic_correction(charinfo *c)
{
  return hunits(fm->get_italic_correction(c->as_glyph(),
					  size.to_scaled_points()));
}

hunits tfont::get_left_italic_correction(charinfo *c)
{
  return hunits(fm->get_left_italic_correction(c->as_glyph(),
					       size.to_scaled_points()));
}

hunits tfont::get_subscript_correction(charinfo *c)
{
  return hunits(fm->get_subscript_correction(c->as_glyph(),
					     size.to_scaled_points()));
}

inline int tfont::get_input_position()
{
  return input_position;
}

inline int tfont::contains(charinfo *ci)
{
  return fm->contains(ci->as_glyph());
}

inline int tfont::get_character_type(charinfo *ci)
{
  return fm->get_character_type(ci->as_glyph());
}

inline bool tfont::is_emboldened(hunits *res)
{
  if (has_emboldening) {
    *res = bold_offset;
    return true;
  }
  else
    return false;
}

inline bool tfont::is_constantly_spaced(hunits *res)
{
  if (has_constant_spacing) {
    *res = constant_space_width;
    return true;
  }
  else
    return false;
}

inline hunits tfont::get_track_kern()
{
  return track_kern;
}

inline tfont *tfont::get_plain()
{
  return plain_version;
}

inline font_size tfont::get_size()
{
  return size;
}

inline int tfont::get_zoom()
{
  return fm->get_zoom();
}

inline symbol tfont::get_name()
{
  return name;
}

inline int tfont::get_height()
{
  return height;
}

inline int tfont::get_slant()
{
  return slant;
}

symbol SYMBOL_ff("ff");
symbol SYMBOL_fi("fi");
symbol SYMBOL_fl("fl");
symbol SYMBOL_Fi("Fi");
symbol SYMBOL_Fl("Fl");

charinfo *tfont::get_lig(charinfo *c1, charinfo *c2)
{
  if (0 == ligature_mode)
    return 0 /* nullptr */;
  charinfo *ci = 0 /* nullptr */;
  if (c1->get_ascii_code() == 'f') {
    switch (c2->get_ascii_code()) {
    case 'f':
      if (fm->has_ligature(font::LIG_ff))
	ci = lookup_charinfo(SYMBOL_ff);
      break;
    case 'i':
      if (fm->has_ligature(font::LIG_fi))
	ci = lookup_charinfo(SYMBOL_fi);
      break;
    case 'l':
      if (fm->has_ligature(font::LIG_fl))
	ci = lookup_charinfo(SYMBOL_fl);
      break;
    }
  }
  else if (ligature_mode != 2 && c1->nm == SYMBOL_ff) {
    switch (c2->get_ascii_code()) {
    case 'i':
      if (fm->has_ligature(font::LIG_ffi))
	ci = lookup_charinfo(SYMBOL_Fi);
      break;
    case 'l':
      if (fm->has_ligature(font::LIG_ffl))
	ci = lookup_charinfo(SYMBOL_Fl);
      break;
    }
  }
  if (ci != 0 /* nullptr */ && fm->contains(ci->as_glyph()))
    return ci;
  return 0 /* nullptr */;
}

inline bool tfont::is_kerned(charinfo *c1, charinfo *c2, hunits *res)
{
  if (0 == kern_mode)
    return false;
  else {
    int n = fm->get_kern(c1->as_glyph(),
			 c2->as_glyph(),
			 size.to_scaled_points());
    if (n) {
      *res = hunits(n);
      return true;
    }
    else
      return false;
  }
}

tfont *tfont::tfont_list = 0 /* nullptr */;

tfont::tfont(tfont_spec &spec) : tfont_spec(spec)
{
  next = tfont_list;
  tfont_list = this;
  tfont_spec plain_spec = plain();
  tfont *p;
  for (p = tfont_list; p != 0 /* nullptr */; p = p->next)
    if (*p == plain_spec) {
      plain_version = p;
      break;
    }
  if (0 /* nullptr */ == p)
    plain_version = new tfont(plain_spec);
}

/* output_file */

class real_output_file : public output_file {
  bool is_output_piped;		// as with `pi` request
  bool want_page_printed;	// if selected with `troff -o`
  bool is_output_on;		// as by \O[0], \O[1] escape sequences
  virtual void really_transparent_char(unsigned char) = 0;
  virtual void really_print_line(hunits x, vunits y, node *n,
				 vunits before, vunits after,
				 hunits width) = 0;
  virtual void really_begin_page(int pageno, vunits page_length) = 0;
  virtual void really_copy_file(hunits x, vunits y,
				const char *filename);
  virtual void really_put_filename(const char *, int);
  virtual void really_on();
  virtual void really_off();
public:
  FILE *fp;
  real_output_file();
  ~real_output_file();
  void flush();
  void transparent_char(unsigned char);
  void print_line(hunits x, vunits y, node *n,
		  vunits before, vunits after, hunits width);
  void begin_page(int pageno, vunits page_length);
  void put_filename(const char *, int);
  void on();
  void off();
  bool is_on();
  bool is_selected_for_printing();
  void copy_file(hunits x, vunits y, const char *filename);
};

class suppress_output_file : public real_output_file {
public:
  suppress_output_file();
  void really_transparent_char(unsigned char);
  void really_print_line(hunits x, vunits y, node *n,
			 vunits, vunits, hunits width);
  void really_begin_page(int pageno, vunits page_length);
};

class ascii_output_file : public real_output_file {
public:
  ascii_output_file();
  void really_transparent_char(unsigned char);
  void really_print_line(hunits x, vunits y, node *n,
			 vunits, vunits, hunits width);
  void really_begin_page(int pageno, vunits page_length);
  void outc(unsigned char c);
  void outs(const char *s);
};

void ascii_output_file::outc(unsigned char c)
{
  if (fp != 0 /* nullptr */)
    fputc(c, fp);
}

void ascii_output_file::outs(const char *s)
{
  if (fp != 0 /* nullptr */) {
    fputc('<', fp);
    if (s != 0 /* nullptr */)
      fputs(s, fp);
    fputc('>', fp);
  }
}

struct hvpair;

class troff_output_file : public real_output_file {
  units hpos;
  units vpos;
  units output_vpos;
  units output_hpos;
  bool must_update_drawing_position;
  int current_size;
  int current_slant;
  int current_height;
  tfont *current_tfont;
  color *current_fill_color;
  color *current_stroke_color;
  int current_font_number;
  symbol *font_position;
  int nfont_positions;
  enum { TBUF_SIZE = 256 };
  char tbuf[TBUF_SIZE];
  int tbuf_len;
  int tbuf_kern;
  bool has_page_begun;
  int cur_div_level;
  string tag_list;
  void do_motion();
  void put(char c);
  void put(unsigned char c);
  void put(int i);
  void put(unsigned int i);
  void put(const char *s);
  void set_font(tfont *tf);
  void flush_tbuf();
public:
  troff_output_file();
  ~troff_output_file();
  void flush();
  void trailer(vunits page_length);
  void put_char(charinfo *, tfont *, color *, color *);
  void put_char_width(charinfo *, tfont *, color *, color *, hunits,
		      hunits);
  void right(hunits);
  void down(vunits);
  void moveto(hunits, vunits);
  void start_device_extension(tfont * /* tf */,
			      color * /* gcol */, color * /* fcol */,
			      bool /* omit_command_prefix */ = false);
  void start_device_extension();
  void write_device_extension_char(unsigned char c);
  void end_device_extension();
  void word_marker();
  void really_transparent_char(unsigned char c);
  void really_print_line(hunits x, vunits y, node *n,
			 vunits before, vunits after, hunits width);
  void really_begin_page(int pageno, vunits page_length);
  void really_copy_file(hunits x, vunits y, const char *filename);
  void really_put_filename(const char *, int);
  void really_on();
  void really_off();
  void draw(char, hvpair *, int, font_size, color *, color *);
  void determine_line_limits (char code, hvpair *point, int npoints);
  void check_charinfo(tfont *tf, charinfo *ci);
  void stroke_color(color *c);
  void fill_color(color *c);
  int get_hpos() { return hpos; }
  int get_vpos() { return vpos; }
  void add_to_tag_list(string s);
  void comment(string s);
  friend void space_char_hmotion_node::tprint(troff_output_file *);
  friend void unbreakable_space_node::tprint(troff_output_file *);
};

static void put_string(const char *s, FILE *fp)
{
  if (fp != 0 /* nullptr */) {
    for (; *s != '\0'; ++s)
      putc(*s, fp);
  }
}

inline void troff_output_file::put(char c)
{
  if (fp != 0 /* nullptr */)
    putc(c, fp);
}

inline void troff_output_file::put(unsigned char c)
{
  if (fp != 0 /* nullptr */)
    putc(c, fp);
}

inline void troff_output_file::put(const char *s)
{
  put_string(s, fp);
}

inline void troff_output_file::put(int i)
{
  put_string(i_to_a(i), fp);
}

inline void troff_output_file::put(unsigned int i)
{
  put_string(ui_to_a(i), fp);
}

void troff_output_file::start_device_extension(tfont *tf, color *gcol,
					       color *fcol,
					       bool omit_command_prefix)
{
  flush_tbuf();
  set_font(tf);
  stroke_color(gcol);
  fill_color(fcol);
  do_motion();
  if (!omit_command_prefix)
    put("x X ");
}

void troff_output_file::start_device_extension()
{
  flush_tbuf();
  put("x X ");
}

void troff_output_file::write_device_extension_char(unsigned char c)
{
  put(c);
  if (c == '\n')
    put('+');
}

void troff_output_file::end_device_extension()
{
  put('\n');
}

inline void troff_output_file::moveto(hunits h, vunits v)
{
  hpos = h.to_units();
  vpos = v.to_units();
}

void troff_output_file::really_print_line(hunits x, vunits y, node *n,
					  vunits before, vunits after,
					  hunits)
{
  moveto(x, y);
  while (n != 0 /* nullptr */) {
    // Check whether we should push the current troff state and use
    // the state at the start of the invocation of this diversion.
    if (n->div_nest_level > cur_div_level && n->push_state) {
      state.push_state(n->push_state);
      cur_div_level = n->div_nest_level;
    }
    // Has the current diversion level decreased?  Then we must pop the
    // troff state.
    while (n->div_nest_level < cur_div_level) {
      state.pop_state();
      cur_div_level = n->div_nest_level;
    }
    // Now check whether the state has changed.
    if ((is_on() || n->causes_tprint())
	&& (state.changed(n->state) || n->is_tag() || n->is_special)) {
      flush_tbuf();
      do_motion();
      must_update_drawing_position = true;
      flush();
      state.flush(fp, n->state, tag_list);
      tag_list = string("");
      flush();
    }
    n->tprint(this);
    n = n->next;
  }
  flush_tbuf();
  // Ensure that transparent throughput (.output, \!) has a more
  // predictable position.
  do_motion();
  must_update_drawing_position = true;
  hpos = 0;
  put('n');
  put(before.to_units());
  put(' ');
  put(after.to_units());
  put('\n');
}

inline void troff_output_file::word_marker()
{
  flush_tbuf();
  if (is_on())
    put('w');
}

inline void troff_output_file::right(hunits n)
{
  hpos += n.to_units();
}

inline void troff_output_file::down(vunits n)
{
  vpos += n.to_units();
}

void troff_output_file::do_motion()
{
  if (must_update_drawing_position) {
    put('V');
    put(vpos);
    put('\n');
    put('H');
    put(hpos);
    put('\n');
  }
  else {
    if (hpos != output_hpos) {
      units n = hpos - output_hpos;
      if (n > 0 && n < hpos) {
	put('h');
	put(n);
      }
      else {
	put('H');
	put(hpos);
      }
      put('\n');
    }
    if (vpos != output_vpos) {
      units n = vpos - output_vpos;
      if (n > 0 && n < vpos) {
	put('v');
	put(n);
      }
      else {
	put('V');
	put(vpos);
      }
      put('\n');
    }
  }
  output_vpos = vpos;
  output_hpos = hpos;
  must_update_drawing_position = false;
}

void troff_output_file::flush_tbuf()
{
  if (!is_on()) {
    tbuf_len = 0;
    return;
  }

  if (0 == tbuf_len)
    return;
  if (0 == tbuf_kern)
    put('t');
  else {
    put('u');
    put(tbuf_kern);
    put(' ');
  }
  check_output_limits(hpos, vpos);
  assert(current_size > 0);
  check_output_limits(hpos, vpos - current_size);

  for (int i = 0; i < tbuf_len; i++)
    put(tbuf[i]);
  put('\n');
  tbuf_len = 0;
}

void troff_output_file::check_charinfo(tfont *tf, charinfo *ci)
{
  if (!is_on())
    return;

  int height = tf->get_char_height(ci).to_units();
  int width = tf->get_width(ci).to_units()
	      + tf->get_italic_correction(ci).to_units();
  int depth = tf->get_char_depth(ci).to_units();
  check_output_limits(output_hpos, output_vpos - height);
  check_output_limits(output_hpos + width, output_vpos + depth);
}

void troff_output_file::put_char_width(charinfo *ci, tfont *tf,
				       color *gcol, color *fcol,
				       hunits w, hunits k)
{
  int kk = k.to_units();
  if (!is_on()) {
    flush_tbuf();
    hpos += w.to_units() + kk;
    return;
  }
  set_font(tf);
  unsigned char c = ci->get_ascii_code();
  if (0U == c) {
    stroke_color(gcol);
    fill_color(fcol);
    flush_tbuf();
    do_motion();
    check_charinfo(tf, ci);
    if (ci->is_numbered()) {
      put('N');
      put(ci->get_number());
    }
    else {
      put('C');
      const char *s = ci->nm.contents();
      if (0 == s[1]) {
	put('\\');
	put(s[0]);
      }
      else
	put(s);
    }
    put('\n');
    hpos += w.to_units() + kk;
  }
  else if (device_has_tcommand) {
    if (tbuf_len > 0 && hpos == output_hpos && vpos == output_vpos
	&& (!gcol || gcol == current_stroke_color)
	&& (!fcol || fcol == current_fill_color)
	&& kk == tbuf_kern
	&& tbuf_len < TBUF_SIZE) {
      check_charinfo(tf, ci);
      tbuf[tbuf_len++] = c;
      output_hpos += w.to_units() + kk;
      hpos = output_hpos;
      return;
    }
    stroke_color(gcol);
    fill_color(fcol);
    flush_tbuf();
    do_motion();
    check_charinfo(tf, ci);
    tbuf[tbuf_len++] = c;
    output_hpos += w.to_units() + kk;
    tbuf_kern = kk;
    hpos = output_hpos;
  }
  else {
    // flush_tbuf();
    int n = hpos - output_hpos;
    check_charinfo(tf, ci);
    // check_output_limits(output_hpos, output_vpos);
    if (vpos == output_vpos
	&& (!gcol || gcol == current_stroke_color)
	&& (!fcol || fcol == current_fill_color)
	&& (n > 0) && (n < 100) && !must_update_drawing_position) {
      put(char(n / 10 + '0'));
      put(char(n % 10 + '0'));
      put(c);
      output_hpos = hpos;
    }
    else {
      stroke_color(gcol);
      fill_color(fcol);
      do_motion();
      put('c');
      put(c);
    }
    hpos += w.to_units() + kk;
  }
}

void troff_output_file::put_char(charinfo *ci, tfont *tf,
				 color *gcol, color *fcol)
{
  flush_tbuf();
  if (!is_on())
    return;
  set_font(tf);
  unsigned char c = ci->get_ascii_code();
  if (0U == c) {
    stroke_color(gcol);
    fill_color(fcol);
    flush_tbuf();
    do_motion();
    if (ci->is_numbered()) {
      put('N');
      put(ci->get_number());
    }
    else {
      put('C');
      const char *s = ci->nm.contents();
      if (0 == s[1]) {
	put('\\');
	put(s[0]);
      }
      else
	put(s);
    }
    put('\n');
  }
  else {
    int n = hpos - output_hpos;
    if (vpos == output_vpos
	&& (!gcol || gcol == current_stroke_color)
	&& (!fcol || fcol == current_fill_color)
	&& n > 0 && n < 100) {
      put(char(n/10 + '0'));
      put(char(n%10 + '0'));
      put(c);
      output_hpos = hpos;
    }
    else {
      stroke_color(gcol);
      fill_color(fcol);
      flush_tbuf();
      do_motion();
      put('c');
      put(c);
    }
  }
}

// set_font calls 'flush_tbuf' if necessary.

void troff_output_file::set_font(tfont *tf)
{
  if (current_tfont == tf)
    return;
  flush_tbuf();
  int n = tf->get_input_position();
  symbol nm = tf->get_name();
  if (n >= nfont_positions || font_position[n] != nm) {
    put("x font ");
    put(n);
    put(' ');
    put(nm.contents());
    put('\n');
    if (n >= nfont_positions) {
      int old_nfont_positions = nfont_positions;
      symbol *old_font_position = font_position;
      nfont_positions *= 3;
      nfont_positions /= 2;
      if (nfont_positions <= n)
	nfont_positions = n + 10;
      font_position = new symbol[nfont_positions];
      memcpy(font_position, old_font_position,
	     old_nfont_positions*sizeof(symbol));
      delete[] old_font_position;
    }
    font_position[n] = nm;
  }
  if (current_font_number != n) {
    put('f');
    put(n);
    put('\n');
    current_font_number = n;
  }
  int zoom = tf->get_zoom();
  int size;
  if (zoom)
    size = scale(tf->get_size().to_scaled_points(),
		 zoom, 1000);
  else
    size = tf->get_size().to_scaled_points();
  if (current_size != size) {
    put('s');
    put(size);
    put('\n');
    current_size = size;
  }
  int slant = tf->get_slant();
  if (current_slant != slant) {
    put("x Slant ");
    put(slant);
    put('\n');
    current_slant = slant;
  }
  int height = tf->get_height();
  if (current_height != height) {
    put("x Height ");
    put((0 == height) ? current_size : height);
    put('\n');
    current_height = height;
  }
  current_tfont = tf;
}

// fill_color calls 'flush_tbuf' and 'do_motion' if necessary.

void troff_output_file::fill_color(color *col)
{
  if ((0 /* nullptr */ == col) || current_fill_color == col)
    return;
  current_fill_color = col;
  if (!want_color_output)
    return;
  flush_tbuf();
  // In nroff-mode devices (grotty), the fill color is a property of the
  // character cell; our drawing position has to be on the page, lest
  // grotty grouse "output above first line discarded".
  if (in_nroff_mode)
    do_motion();
  put("DF");
  unsigned int components[4];
  color_scheme scheme;
  scheme = col->get_components(components);
  switch (scheme) {
  case DEFAULT:
    put('d');
    break;
  case RGB:
    put("r ");
    put(Red);
    put(' ');
    put(Green);
    put(' ');
    put(Blue);
    break;
  case CMY:
    put("c ");
    put(Cyan);
    put(' ');
    put(Magenta);
    put(' ');
    put(Yellow);
    break;
  case CMYK:
    put("k ");
    put(Cyan);
    put(' ');
    put(Magenta);
    put(' ');
    put(Yellow);
    put(' ');
    put(Black);
    break;
  case GRAY:
    put("g ");
    put(Gray);
    break;
  }
  put('\n');
}

// stroke_color calls 'flush_tbuf' and 'do_motion' if necessary.

void troff_output_file::stroke_color(color *col)
{
  if (!col || (current_stroke_color == col))
    return;
  current_stroke_color = col;
  if (!want_color_output)
    return;
  flush_tbuf();
  // In nroff-mode devices (grotty), the stroke color is a property of
  // the character cell; our drawing position has to be on the page,
  // lest grotty grouse "output above first line discarded".
  if (in_nroff_mode)
    do_motion();
  put("m");
  unsigned int components[4];
  color_scheme scheme;
  scheme = col->get_components(components);
  switch (scheme) {
  case DEFAULT:
    put('d');
    break;
  case RGB:
    put("r ");
    put(Red);
    put(' ');
    put(Green);
    put(' ');
    put(Blue);
    break;
  case CMY:
    put("c ");
    put(Cyan);
    put(' ');
    put(Magenta);
    put(' ');
    put(Yellow);
    break;
  case CMYK:
    put("k ");
    put(Cyan);
    put(' ');
    put(Magenta);
    put(' ');
    put(Yellow);
    put(' ');
    put(Black);
    break;
  case GRAY:
    put("g ");
    put(Gray);
    break;
  }
  put('\n');
}

void troff_output_file::add_to_tag_list(string s)
{
  if (tag_list == string(""))
    tag_list = s;
  else {
    tag_list += string("\n");
    tag_list += s;
  }
}

void troff_output_file::comment(string s)
{
  flush_tbuf();
  assert(s.search('\n') == -1); // Don't write a multi-line comment.
  put("# ");
  string t = s + '\0';
  put(t.contents());
  put('\n');
}

// determine_line_limits - works out the smallest box which will contain
//			   the entity, code, built from the point array.
void troff_output_file::determine_line_limits(char code, hvpair *point,
					      int npoints)
{
  int i, x, y;

  if (!is_on())
    return;

  switch (code) {
  case 'c':
  case 'C':
    // only the h field is used when defining a circle
    check_output_limits(output_hpos,
			output_vpos - point[0].h.to_units() / 2);
    check_output_limits(output_hpos + point[0].h.to_units(),
			output_vpos + point[0].h.to_units() / 2);
    break;
  case 'E':
  case 'e':
    check_output_limits(output_hpos,
			output_vpos - point[0].v.to_units() / 2);
    check_output_limits(output_hpos + point[0].h.to_units(),
			output_vpos + point[0].v.to_units() / 2);
    break;
  case 'P':
  case 'p':
    x = output_hpos;
    y = output_vpos;
    check_output_limits(x, y);
    for (i = 0; i < npoints; i++) {
      x += point[i].h.to_units();
      y += point[i].v.to_units();
      check_output_limits(x, y);
    }
    break;
  case 't':
    x = output_hpos;
    y = output_vpos;
    for (i = 0; i < npoints; i++) {
      x += point[i].h.to_units();
      y += point[i].v.to_units();
      check_output_limits(x, y);
    }
    break;
  case 'a':
    double c[2];
    int p[4];
    int minx, miny, maxx, maxy;
    x = output_hpos;
    y = output_vpos;
    p[0] = point[0].h.to_units();
    p[1] = point[0].v.to_units();
    p[2] = point[1].h.to_units();
    p[3] = point[1].v.to_units();
    if (adjust_arc_center(p, c)) {
      check_output_arc_limits(x, y,
			      p[0], p[1], p[2], p[3],
			      c[0], c[1],
			      &minx, &maxx, &miny, &maxy);
      check_output_limits(minx, miny);
      check_output_limits(maxx, maxy);
      break;
    }
    // fall through
  case 'l':
    x = output_hpos;
    y = output_vpos;
    check_output_limits(x, y);
    for (i = 0; i < npoints; i++) {
      x += point[i].h.to_units();
      y += point[i].v.to_units();
      check_output_limits(x, y);
    }
    break;
  default:
    x = output_hpos;
    y = output_vpos;
    for (i = 0; i < npoints; i++) {
      x += point[i].h.to_units();
      y += point[i].v.to_units();
      check_output_limits(x, y);
    }
  }
}

void troff_output_file::draw(char code, hvpair *point, int npoints,
			     font_size fsize, color *gcol, color *fcol)
{
  int i;
  stroke_color(gcol);
  fill_color(fcol);
  flush_tbuf();
  do_motion();
  if (is_on()) {
    int size = fsize.to_scaled_points();
    if (current_size != size) {
      put('s');
      put(size);
      put('\n');
      current_size = size;
      current_tfont = 0;
    }
    put('D');
    put(code);
    if (code == 'c') {
      put(' ');
      put(point[0].h.to_units());
    }
    else
      for (i = 0; i < npoints; i++) {
	put(' ');
	put(point[i].h.to_units());
	put(' ');
	put(point[i].v.to_units());
      }
    determine_line_limits(code, point, npoints);
  }

  for (i = 0; i < npoints; i++)
    output_hpos += point[i].h.to_units();
  hpos = output_hpos;
  if (code != 'e') {
    for (i = 0; i < npoints; i++)
      output_vpos += point[i].v.to_units();
    vpos = output_vpos;
  }
  if (is_on())
    put('\n');
}

void troff_output_file::really_on()
{
  flush_tbuf();
  must_update_drawing_position = true;
  do_motion();
}

void troff_output_file::really_off()
{
  flush_tbuf();
}

void troff_output_file::really_put_filename(const char *filename,
					    int po)
{
  flush_tbuf();
  put("x F ");
  if (po)
    put("<");
  put(filename);
  if (po)
    put(">");
  put('\n');
}

void troff_output_file::really_begin_page(int pageno,
					  vunits page_length)
{
  flush_tbuf();
  if (has_page_begun) {
    if (page_length > V0) {
      put('V');
      put(page_length.to_units());
      put('\n');
    }
  }
  else
    has_page_begun = true;
  current_tfont = 0;
  current_font_number = FONT_NOT_MOUNTED;
  current_size = 0;
  // current_height = 0;
  // current_slant = 0;
  hpos = 0;
  vpos = 0;
  output_hpos = 0;
  output_vpos = 0;
  must_update_drawing_position = true;
  for (int i = 0; i < nfont_positions; i++)
    font_position[i] = NULL_SYMBOL;
  put('p');
  put(pageno);
  put('\n');
}

void troff_output_file::really_copy_file(hunits x, vunits y,
					 const char *filename)
{
  moveto(x, y);
  flush_tbuf();
  do_motion();
  errno = 0;
  FILE *ifp = include_search_path.open_file_cautiously(filename);
  if (0 /* nullptr */ == ifp)
    error("cannot open '%1': %2", filename, strerror(errno));
  else {
    int c;
    while ((c = getc(ifp)) != EOF)
      put(char(c));
    fclose(ifp);
  }
  must_update_drawing_position = true;
  current_size = 0;
  current_tfont = 0;
  current_font_number = FONT_NOT_MOUNTED;
  for (int i = 0; i < nfont_positions; i++)
    font_position[i] = NULL_SYMBOL;
}

void troff_output_file::really_transparent_char(unsigned char c)
{
  put(c);
}

troff_output_file::~troff_output_file()
{
  delete[] font_position;
}

void troff_output_file::trailer(vunits page_length)
{
  flush_tbuf();
  if (was_any_page_in_output_list) {
    if (page_length > V0) {
      put("x trailer\n");
      put('V');
      put(page_length.to_units());
      put('\n');
    }
  }
  else
    warning(WARN_RANGE, "no pages match output page selection list");
  put("x stop\n");
}

troff_output_file::troff_output_file()
: current_slant(0), current_height(0), current_fill_color(0),
  current_stroke_color(0), nfont_positions(10), tbuf_len(0),
  has_page_begun(false), cur_div_level(0)
{
  font_position = new symbol[nfont_positions];
  put("x T ");
  put(device);
  put('\n');
  put("x res ");
  put(units_per_inch);
  put(' ');
  put(hresolution);
  put(' ');
  put(vresolution);
  put('\n');
  put("x init\n");
}

void troff_output_file::flush()
{
  flush_tbuf();
  real_output_file::flush();
}

/* output_file */

output_file *the_output = 0 /* nullptr */;

output_file::output_file()
{
	is_dying = false;
}

output_file::~output_file()
{
}

void output_file::trailer(vunits)
{
}

void output_file::put_filename(const char *, int)
{
}

void output_file::on()
{
}

void output_file::off()
{
}

real_output_file::real_output_file()
: want_page_printed(true), is_output_on(true)
{
  if (pipe_command) {
    if ((fp = popen(pipe_command, POPEN_WT)) != 0 /* nullptr */) {
      is_output_piped = true;
      return;
    }
    error("pipe open failed: %1", strerror(errno));
  }
  is_output_piped = false;
  fp = stdout;
}

real_output_file::~real_output_file()
{
  if (0 /* nullptr */ == fp)
    return;
  // Prevent destructor from recursing; see
  // div.cpp:write_any_trailer_and_exit().
  is_dying = true;
  // To avoid looping, set fp to 0 before calling fatal().
  if (ferror(fp)) {
    fp = 0 /* nullptr */;
    fatal("error on output file stream");
  }
  else if (fflush(fp) < 0) {
    fp = 0 /* nullptr */;
    fatal("unable to flush output file: %1", strerror(errno));
  }
  if (is_output_piped) {
    int result = pclose(fp);
    fp = 0 /* nullptr */;
    if (result < 0)
      fatal("unable to close pipe: %1", strerror(errno));
    if (!WIFEXITED(result))
      error("output process '%1' got fatal signal %2",
	    pipe_command,
	    WIFSIGNALED(result) ? WTERMSIG(result) : WSTOPSIG(result));
    else {
      int exit_status = WEXITSTATUS(result);
      if (exit_status != 0)
	error("output process '%1' exited with status %2",
	      pipe_command, exit_status);
    }
  }
  else
  if (fclose(fp) < 0) {
    fp = 0 /* nullptr */;
    fatal("unable to close output file: %1", strerror(errno));
  }
}

void real_output_file::flush()
{
  // To avoid looping, set fp to 0 before calling fatal().
  if (fflush(fp) < 0) {
    fp = 0 /* nullptr */;
    fatal("unable to flush output file: %1", strerror(errno));
  }
}

bool real_output_file::is_selected_for_printing()
{
  return want_page_printed;
}

void real_output_file::begin_page(int pageno, vunits page_length)
{
  want_page_printed = in_output_page_list(pageno);
  if (want_page_printed) {
    was_any_page_in_output_list = true;
    really_begin_page(pageno, page_length);
  }
}

void real_output_file::copy_file(hunits x, vunits y,
				 const char *filename)
{
  if (want_page_printed && is_output_on)
    really_copy_file(x, y, filename);
  check_output_limits(x.to_units(), y.to_units());
}

void real_output_file::transparent_char(unsigned char c)
{
  if (want_page_printed && is_output_on)
    really_transparent_char(c);
}

void real_output_file::print_line(hunits x, vunits y, node *n,
			     vunits before, vunits after, hunits width)
{
  if (want_page_printed)
    really_print_line(x, y, n, before, after, width);
  delete_node_list(n);
}

void real_output_file::really_copy_file(hunits, vunits, const char *)
{
  // do nothing
}

void real_output_file::put_filename(const char *filename, int po)
{
  really_put_filename(filename, po);
}

void real_output_file::really_put_filename(const char *, int)
{
}

void real_output_file::on()
{
  really_on();
  // XXX: Assertion fails when generating pic.html.  Find out why.
  //assert(!is_output_on);
  is_output_on = true;
}

void real_output_file::off()
{
  really_off();
  // XXX: Assertion fails when generating ms.html.  Find out why.
  //assert(is_output_on);
  is_output_on = false;
}

bool real_output_file::is_on()
{
  return is_output_on;
}

void real_output_file::really_on()
{
}

void real_output_file::really_off()
{
}

/* ascii_output_file */

void ascii_output_file::really_transparent_char(unsigned char c)
{
  if (fp != 0 /* nullptr */)
    putc(c, fp);
}

void ascii_output_file::really_print_line(hunits, vunits, node *n,
					  vunits, vunits, hunits)
{
  while (n != 0 /* nullptr */) {
    n->ascii_print(this);
    n = n->next;
  }
  if (fp != 0 /* nullptr */)
    fputc('\n', fp);
}

void ascii_output_file::really_begin_page(int /* pageno */,
					  vunits /* page_length */)
{
  fputs("<beginning of page>\n", fp);
}

ascii_output_file::ascii_output_file()
{
}

/* suppress_output_file */

suppress_output_file::suppress_output_file()
{
}

void suppress_output_file::really_print_line(hunits, vunits, node *,
					     vunits, vunits, hunits)
{
}

void suppress_output_file::really_begin_page(int, vunits)
{
}

void suppress_output_file::really_transparent_char(unsigned char)
{
}

/* glyphs, ligatures, kerns, discretionary breaks */

// abstract
class charinfo_node : public node {
protected:
  charinfo *ci;
public:
  charinfo_node(charinfo *, statem *, int, node * = 0 /* nullptr */);
  virtual int ends_sentence();
  virtual bool overlaps_vertically();
  virtual bool overlaps_horizontally();
  virtual void dump_properties();
};

charinfo_node::charinfo_node(charinfo *c, statem *s, int divlevel,
			     node *x)
: node(x, s, divlevel), ci(c)
{
}

int charinfo_node::ends_sentence()
{
  if (ci->ends_sentence())
    return 1;
  else if (ci->is_transparent_to_end_of_sentence())
    return 2;
  return 0;
}

bool charinfo_node::overlaps_horizontally()
{
  return ci->overlaps_horizontally();
}

bool charinfo_node::overlaps_vertically()
{
  return ci->overlaps_vertically();
}

void charinfo_node::dump_properties()
{
  node::dump_properties();
  // GNU troff multiplexes the distinction of ordinary vs. special
  // characters though the special character code zero.
  unsigned char c = ci->get_ascii_code();
  if (c != 0U) {
    fputs(", \"character\": ", stderr);
    // It's not a `string` or `symbol` we can `.json_dump()`, so we have
    // to write the quotation marks ourselves.
    fputc('\"', stderr);
    json_char jc = json_encode_char(c);
    // Write out its JSON representation by character by character to
    // keep libc string functions from interpreting C escape sequences.
    for (size_t i = 0; i < jc.len; i++)
      fputc(jc.buf[i], stderr);
    fputc('\"', stderr);
  }
  else {
    fputs(", \"special character\": ", stderr);
    ci->nm.json_dump();
  }
  fflush(stderr);
}

// A glyph node corresponds to a glyph supplied by a device font.

class glyph_node : public charinfo_node {
protected:
  tfont *tf;
  color *gcol;
  color *fcol;		/* this is needed for grotty */
#ifdef STORE_WIDTH
  hunits wid;
  glyph_node(charinfo *, tfont *, color *, color *, hunits,
	     statem *, int, node * = 0 /* nullptr */);
#endif
public:
  glyph_node(charinfo *, tfont *, color *, color *,
	     statem *, int, node * = 0 /* nullptr */);
  ~glyph_node() {}
  node *copy();
  node *merge_glyph_node(glyph_node *);
  node *merge_self(node *);
  hunits width();
  node *last_char_node();
  units size();
  void vertical_extent(vunits *, vunits *);
  hunits subscript_correction();
  hunits italic_correction();
  hunits left_italic_correction();
  hunits skew();
  hyphenation_type get_hyphenation_type();
  tfont *get_tfont();
  color *get_stroke_color();
  color *get_fill_color();
  void tprint(troff_output_file *);
  void zero_width_tprint(troff_output_file *);
  hyphen_list *get_hyphen_list(hyphen_list *, int *);
  node *add_self(node *, hyphen_list **);
  void ascii_print(ascii_output_file *);
  void asciify(macro *);
  int character_type();
  bool is_same_as(node *);
  const char *type();
  bool causes_tprint();
  bool is_tag();
};

// Not derived from `container_node`; implements custom double container
// dumper in dump_node().
class ligature_node : public glyph_node {
  node *n1;
  node *n2;
#ifdef STORE_WIDTH
  ligature_node(charinfo *, tfont *, color *, color *, hunits,
		node *, node *, statem *, int,
		node * = 0 /* nullptr */);
#endif
public:
  void *operator new(size_t);
  void operator delete(void *);
  ligature_node(charinfo *, tfont *, color *, color *,
		node *, node *, statem *, int,
		node * = 0 /* nullptr */);
  ~ligature_node();
  node *copy();
  node *add_self(node *, hyphen_list **);
  hyphen_list *get_hyphen_list(hyphen_list *, int *);
  void ascii_print(ascii_output_file *);
  void asciify(macro *);
  bool is_same_as(node *);
  const char *type();
  bool causes_tprint();
  bool is_tag();
  void dump_node();
};

// Not derived from `container_node`; implements custom double container
// dumper in dump_node().
class kern_pair_node : public node {
  hunits amount;
  node *n1;
  node *n2;
public:
  kern_pair_node(hunits, node *, node *, statem *, int,
		 node * = 0 /* nullptr */);
  ~kern_pair_node();
  node *copy();
  node *merge_glyph_node(glyph_node *);
  node *add_self(node *, hyphen_list **);
  hyphen_list *get_hyphen_list(hyphen_list *, int *);
  node *add_discretionary_hyphen();
  hunits width();
  node *last_char_node();
  hunits italic_correction();
  hunits subscript_correction();
  void tprint(troff_output_file *);
  hyphenation_type get_hyphenation_type();
  int ends_sentence();
  void ascii_print(ascii_output_file *);
  void asciify(macro *);
  bool is_same_as(node *);
  const char *type();
  bool causes_tprint();
  bool is_tag();
  void vertical_extent(vunits *, vunits *);
  void dump_properties();
  void dump_node();
};

// Not derived from `container_node`; implements custom triple container
// dumper in dump_node().
class dbreak_node : public node {
  node *none;
  node *pre;
  node *post;
public:
  dbreak_node(node *, node *, statem *, int, node * = 0 /* nullptr */);
  ~dbreak_node();
  node *copy();
  node *merge_glyph_node(glyph_node *);
  node *add_discretionary_hyphen();
  hunits width();
  node *last_char_node();
  hunits italic_correction();
  hunits subscript_correction();
  void tprint(troff_output_file *);
  breakpoint *get_breakpoints(hunits /* width */, int /* ns */,
			      breakpoint * /* rest */ = 0 /* nullptr */,
			      bool /* is_inner */ = false);
  int nbreaks();
  int ends_sentence();
  void split(int, node **, node **);
  hyphenation_type get_hyphenation_type();
  void ascii_print(ascii_output_file *);
  void asciify(macro *);
  bool is_same_as(node *);
  const char *type();
  bool causes_tprint();
  bool is_tag();
  void dump_node();
};

void *ligature_node::operator new(size_t n)
{
  return new char[n];
}

void ligature_node::operator delete(void *p)
{
  delete[] (char *)p;
}

glyph_node::glyph_node(charinfo *c, tfont *t, color *gc, color *fc,
		       statem *s, int divlevel, node *x)
: charinfo_node(c, s, divlevel, x), tf(t), gcol(gc), fcol(fc)
{
#ifdef STORE_WIDTH
  wid = tf->get_width(ci);
#endif
}

#ifdef STORE_WIDTH
glyph_node::glyph_node(charinfo *c, tfont *t,
		       color *gc, color *fc, hunits w,
		       statem *s, int divlevel, node *x)
: charinfo_node(c, s, divlevel, x), tf(t), gcol(gc), fcol(fc), wid(w)
{
}
#endif

node *glyph_node::copy()
{
#ifdef STORE_WIDTH
  return new glyph_node(ci, tf, gcol, fcol, wid, state, div_nest_level);
#else
  return new glyph_node(ci, tf, gcol, fcol, state, div_nest_level);
#endif
}

node *glyph_node::merge_self(node *nd)
{
  return nd->merge_glyph_node(this);
}

int glyph_node::character_type()
{
  return tf->get_character_type(ci);
}

node *glyph_node::add_self(node *n, hyphen_list **p)
{
  assert(ci->get_hyphenation_code() == (*p)->hyphenation_code);
  next = 0 /* nullptr */;
  node *nn = 0 /* nullptr */;
  if ((0 /* nullptr */ == n) ||
      (0 /* nullptr */ == (nn = n->merge_glyph_node(this)))) {
    next = n;
    nn = this;
  }
  if ((*p)->is_hyphen)
    nn = nn->add_discretionary_hyphen();
  hyphen_list *pp = *p;
  *p = (*p)->next;
  delete pp;
  assert(nn != 0 /* nullptr */);
  return nn;
}

units glyph_node::size()
{
  return tf->get_size().to_units();
}

hyphen_list *glyph_node::get_hyphen_list(hyphen_list *tail, int *count)
{
  (*count)++;
  return new hyphen_list(ci->get_hyphenation_code(), tail);
}

tfont *node::get_tfont()
{
  return 0 /* nullptr */;
}

tfont *glyph_node::get_tfont()
{
  return tf;
}

color *node::get_stroke_color()
{
  return 0 /* nullptr */;
}

color *glyph_node::get_stroke_color()
{
  return gcol;
}

color *node::get_fill_color()
{
  return 0 /* nullptr */;
}

color *glyph_node::get_fill_color()
{
  return fcol;
}

node *node::merge_glyph_node(glyph_node *)
{
  return 0 /* nullptr */;
}

node *glyph_node::merge_glyph_node(glyph_node *gn)
{
  if (tf == gn->tf && gcol == gn->gcol && fcol == gn->fcol) {
    charinfo *lig;
    if ((lig = tf->get_lig(ci, gn->ci)) != 0 /* nullptr */) {
      node *next1 = next;
      next = 0 /* nullptr */;
      return new ligature_node(lig, tf, gcol, fcol, this, gn, state,
			       gn->div_nest_level, next1);
    }
    hunits kern;
    if (tf->is_kerned(ci, gn->ci, &kern)) {
      node *next1 = next;
      next = 0 /* nullptr */;
      return new kern_pair_node(kern, this, gn, state,
				gn->div_nest_level, next1);
    }
  }
  return 0 /* nullptr */;
}

#ifdef STORE_WIDTH
inline
#endif
hunits glyph_node::width()
{
#ifdef STORE_WIDTH
  return wid;
#else
  return tf->get_width(ci);
#endif
}

node *glyph_node::last_char_node()
{
  return this;
}

void glyph_node::vertical_extent(vunits *min, vunits *max)
{
  *min = -tf->get_char_height(ci);
  *max = tf->get_char_depth(ci);
}

hunits glyph_node::skew()
{
  return tf->get_char_skew(ci);
}

hunits glyph_node::subscript_correction()
{
  return tf->get_subscript_correction(ci);
}

hunits glyph_node::italic_correction()
{
  return tf->get_italic_correction(ci);
}

hunits glyph_node::left_italic_correction()
{
  return tf->get_left_italic_correction(ci);
}

hyphenation_type glyph_node::get_hyphenation_type()
{
  return HYPHENATION_PERMITTED;
}

void glyph_node::ascii_print(ascii_output_file *ascii)
{
  unsigned char c = ci->get_ascii_code();
  if (c != 0U)
    ascii->outc(c);
  else
    ascii->outs(ci->nm.contents());
}
ligature_node::ligature_node(charinfo *c, tfont *t,
			     color *gc, color *fc,
			     node *gn1, node *gn2, statem *s,
			     int divlevel, node *x)
: glyph_node(c, t, gc, fc, s, divlevel, x), n1(gn1), n2(gn2)
{
}

#ifdef STORE_WIDTH
ligature_node::ligature_node(charinfo *c, tfont *t,
			     color *gc, color *fc, hunits w,
			     node *gn1, node *gn2, statem *s,
			     int divlevel, node *x)
: glyph_node(c, t, gc, fc, w, s, divlevel, x), n1(gn1), n2(gn2)
{
}
#endif

ligature_node::~ligature_node()
{
  delete n1;
  delete n2;
}

node *ligature_node::copy()
{
#ifdef STORE_WIDTH
  return new ligature_node(ci, tf, gcol, fcol, wid, n1->copy(),
			   n2->copy(), state, div_nest_level);
#else
  return new ligature_node(ci, tf, gcol, fcol, n1->copy(), n2->copy(),
			   state, div_nest_level);
#endif
}

void ligature_node::ascii_print(ascii_output_file *ascii)
{
  n1->ascii_print(ascii);
  n2->ascii_print(ascii);
}

hyphen_list *ligature_node::get_hyphen_list(hyphen_list *tail,
					    int *count)
{
  hyphen_list *hl = n2->get_hyphen_list(tail, count);
  return n1->get_hyphen_list(hl, count);
}

node *ligature_node::add_self(node *n, hyphen_list **p)
{
  n = n1->add_self(n, p);
  n = n2->add_self(n, p);
  n1 = n2 = 0 /* nullptr */;
  delete this;
  return n;
}

void ligature_node::dump_node()
{
  fputc('{', stderr);
  // Flush so that in case something goes wrong with property dumping,
  // we know that we traversed to a new node.
  fflush(stderr);
  node::dump_properties();
  if (n1 != 0 /* nullptr */) {
    fputs(", \"n1\": ", stderr);
    n1->dump_node();
  }
  if (n2 != 0 /* nullptr */) {
    fputs(", \"n2\": ", stderr);
    n2->dump_node();
  }
  fputc('}', stderr);
  fflush(stderr);
}

kern_pair_node::kern_pair_node(hunits n, node *first, node *second,
			       statem* s, int divlevel, node *x)
: node(x, s, divlevel), amount(n), n1(first), n2(second)
{
}

void kern_pair_node::dump_properties()
{
  node::dump_properties();
  fprintf(stderr, ", \"amount\": %d", amount.to_units());
  fflush(stderr);
}

void kern_pair_node::dump_node()
{
  fputc('{', stderr);
  // Flush so that in case something goes wrong with property dumping,
  // we know that we traversed to a new node.
  fflush(stderr);
  dump_properties();
  if (n1 != 0 /* nullptr */) {
    fputs(", \"n1\": ", stderr);
    n1->dump_node();
  }
  if (n2 != 0 /* nullptr */) {
    fputs(", \"n2\": ", stderr);
    n2->dump_node();
  }
  fputc('}', stderr);
  fflush(stderr);
}

dbreak_node::dbreak_node(node *n, node *p, statem *s, int divlevel,
			 node *x)
: node(x, s, divlevel), none(n), pre(p), post(0 /* nullptr */)
{
}

node *dbreak_node::merge_glyph_node(glyph_node *gn)
{
  glyph_node *gn2 = static_cast<glyph_node *>(gn->copy());
  node *new_none = none ? none->merge_glyph_node(gn) : 0 /* nullptr */;
  node *new_post = post ? post->merge_glyph_node(gn2) : 0 /* nullptr */;
  if ((0 /* nullptr */ == new_none) && (0 /* nullptr */ == new_post)) {
    delete gn2;
    return 0 /* nullptr */;
  }
  if (new_none != 0 /* nullptr */)
    none = new_none;
  else {
    gn->next = none;
    none = gn;
  }
  if (new_post != 0 /* nullptr */)
    post = new_post;
  else {
    gn2->next = post;
    post = gn2;
  }
  return this;
}

node *kern_pair_node::merge_glyph_node(glyph_node *gn)
{
  node *nd = n2->merge_glyph_node(gn);
  if (0 /* nullptr */ == nd)
    return 0 /* nullptr */;
  n2 = nd;
  nd = n2->merge_self(n1);
  if (nd != 0 /* nullptr */) {
    nd->next = next;
    n1 = n2 = 0 /* nullptr */;
    delete this;
    return nd;
  }
  return this;
}

hunits kern_pair_node::italic_correction()
{
  return n2->italic_correction();
}

hunits kern_pair_node::subscript_correction()
{
  return n2->subscript_correction();
}

void kern_pair_node::vertical_extent(vunits *min, vunits *max)
{
  n1->vertical_extent(min, max);
  vunits min2, max2;
  n2->vertical_extent(&min2, &max2);
  if (min2 < *min)
    *min = min2;
  if (max2 > *max)
    *max = max2;
}

node *kern_pair_node::add_discretionary_hyphen()
{
  tfont *tf = n1->get_tfont();
  if (tf != 0 /* nullptr */) {
    if (tf->contains(soft_hyphen_char)) {
      color *gcol = n2->get_stroke_color();
      color *fcol = n2->get_fill_color();
      node *next1 = next;
      next = 0 /* nullptr */;
      node *n = copy();
      glyph_node *gn = new glyph_node(soft_hyphen_char, tf, gcol, fcol,
				      state, div_nest_level);
      node *nn = n->merge_glyph_node(gn);
      if (0 /* nullptr */ == nn) {
	gn->next = n;
	nn = gn;
      }
      return new dbreak_node(this, nn, state, div_nest_level, next1);
    }
  }
  return this;
}

kern_pair_node::~kern_pair_node()
{
  if (n1 != 0 /* nullptr */)
    delete n1;
  if (n2 != 0 /* nullptr */)
    delete n2;
}

dbreak_node::~dbreak_node()
{
  delete_node_list(pre);
  delete_node_list(post);
  delete_node_list(none);
}

node *kern_pair_node::copy()
{
  return new kern_pair_node(amount, n1->copy(), n2->copy(), state,
			    div_nest_level);
}

node *copy_node_list(node *n)
{
  node *p = 0 /* nullptr */;
  while (n != 0 /* nullptr */) {
    node *nn = n->copy();
    nn->next = p;
    p = nn;
    n = n->next;
  }
  while (p != 0 /* nullptr */) {
    node *pp = p->next;
    p->next = n;
    n = p;
    p = pp;
  }
  return n;
}

void delete_node_list(node *n)
{
  while (n != 0 /* nullptr */) {
    node *tem = n;
    n = n->next;
    delete tem;
  }
}

void dump_node_list(node *n)
{
  bool need_comma = false;
  fputc('[', stderr);
  while (n != 0 /* nullptr */) {
    if (need_comma)
      fputs(", ", stderr);
    n->dump_node();
    need_comma = true;
    n = n->next;
  }
  // !need_comma implies that the list was empty.  JSON convention is to
  // put a space between an empty pair of square brackets.
  if (!need_comma)
    fputc(' ', stderr);
  fputc(']', stderr);
  fflush(stderr);
}

node *dbreak_node::copy()
{
  dbreak_node *p = new dbreak_node(copy_node_list(none),
				   copy_node_list(pre), state,
				   div_nest_level);
  p->post = copy_node_list(post);
  return p;
}

hyphen_list *node::get_hyphen_list(hyphen_list *tail, int *)
{
  return tail;
}

hyphen_list *kern_pair_node::get_hyphen_list(hyphen_list *tail,
					     int *count)
{
  hyphen_list *hl = n2->get_hyphen_list(tail, count);
  return n1->get_hyphen_list(hl, count);
}

class hyphen_inhibitor_node : public node {
public:
  hyphen_inhibitor_node(node * = 0 /* nullptr */);
  void asciify(macro *);
  node *copy();
  bool causes_tprint();
  bool is_tag();
  bool is_same_as(node *);
  const char *type();
  hyphenation_type get_hyphenation_type();
};

hyphen_inhibitor_node::hyphen_inhibitor_node(node *nd) : node(nd)
{
}

node *hyphen_inhibitor_node::copy()
{
  return new hyphen_inhibitor_node;
}

bool hyphen_inhibitor_node::causes_tprint()
{
  return false;
}

bool hyphen_inhibitor_node::is_tag()
{
  return false;
}

bool hyphen_inhibitor_node::is_same_as(node *)
{
  return true;
}

const char *hyphen_inhibitor_node::type()
{
  return "hyphenation inhibitor node";
}

hyphenation_type hyphen_inhibitor_node::get_hyphenation_type()
{
  return HYPHENATION_INHIBITED;
}

/* add_discretionary_hyphen methods */

node *dbreak_node::add_discretionary_hyphen()
{
  if (post)
    post = post->add_discretionary_hyphen();
  if (none)
    none = none->add_discretionary_hyphen();
  return this;
}

node *node::add_discretionary_hyphen()
{
  tfont *tf = get_tfont();
  if (0 /* nullptr */ == tf)
    return new hyphen_inhibitor_node(this);
  if (tf->contains(soft_hyphen_char)) {
    color *gcol = get_stroke_color();
    color *fcol = get_fill_color();
    node *next1 = next;
    next = 0 /* nullptr */;
    node *n = copy();
    glyph_node *gn = new glyph_node(soft_hyphen_char, tf, gcol, fcol,
				    state, div_nest_level);
    node *n1 = n->merge_glyph_node(gn);
    if (0 /* nullptr */ == n1) {
      gn->next = n;
      n1 = gn;
    }
    return new dbreak_node(this, n1, state, div_nest_level, next1);
  }
  return this;
}

node *node::merge_self(node *)
{
  return 0 /* nullptr */;
}

node *node::add_self(node *n, hyphen_list ** /*p*/)
{
  next = n;
  return this;
}

node *kern_pair_node::add_self(node *n, hyphen_list **p)
{
  n = n1->add_self(n, p);
  n = n2->add_self(n, p);
  n1 = n2 = 0 /* nullptr */;
  delete this;
  return n;
}

hunits node::width()
{
  return H0;
}

node *node::last_char_node()
{
  return 0 /* nullptr */;
}

bool node::causes_tprint()
{
  return false;
}

bool node::is_tag()
{
  return false;
}

// TODO: Figure out what a hyphenation code means in the UTF-8 future,
// where a "grochar" is a vector of NFD decomposed code points.  Can it
// be a scalar--a 32-bit int?
unsigned char node::get_break_code()
{
  return 0U;
}

hunits hmotion_node::width()
{
  return n;
}

units node::size()
{
  return points_to_units(10);
}

void node::dump_properties()
{
  fprintf(stderr, "\"type\": \"%s\"", type());
  fprintf(stderr, ", \"diversion level\": %d", div_nest_level);
  fprintf(stderr, ", \"is_special_node\": %s",
	  is_special ? "true" : "false");
  if (push_state) {
    fputs(", \"push_state\": ", stderr);
    push_state->display_state();
  }
  if (state) {
    fputs(", \"state\": ", stderr);
    state->display_state();
  }
  fflush(stderr);
}

void node::dump_node()
{
  fputc('{', stderr);
  // Flush so that in case something goes wrong with property dumping,
  // we know that we traversed to a new node.
  fflush(stderr);
  dump_properties();
  fputc('}', stderr);
  fflush(stderr);
}

container_node::container_node(node *contents)
: node(), nodes(contents)
{
}

container_node::container_node(node *nxt, node *contents)
: node(nxt), nodes(contents)
{
}

// `left_italic_corrected_node` uses an initially empty container.
container_node::container_node(node *nxt, statem *s, int divl)
: node(nxt, s, divl), nodes(0 /* nullptr */)
{
}

#if 0
container_node::container_node(node *nxt, statem *s, node *contents)
: node(nxt, s), nodes(contents)
{
}
#endif

container_node::container_node(node *nxt, statem *s, int divl,
			       node *contents)
: node(nxt, s, divl), nodes(contents)
{
}

container_node::container_node(node *nxt, statem *s, int divl,
			       bool special, node *contents)
: node(nxt, s, divl, special), nodes(contents)
{
}

container_node::~container_node()
{
  delete_node_list(nodes);
}

void container_node::dump_node()
{
  fputc('{', stderr);
  dump_properties();
  fputs(", \"contains\": ", stderr);
  dump_node_list(nodes);
  fputc('}', stderr);
  fflush(stderr);
}

void dump_node_list_in_reverse(node *nlist)
{
  // It's stored in reverse order already; this puts it forward again.
  std::stack<node *> reversed_node_list;
  node *n = nlist;

  while (n != 0 /* nullptr */) {
    reversed_node_list.push(n);
    n = n->next;
  }
  fputc('[', stderr);
  bool need_comma = false;
  while (!reversed_node_list.empty()) {
    if (need_comma)
      fputs(",\n", stderr);
    reversed_node_list.top()->dump_node();
    reversed_node_list.pop();
    need_comma = true;
  }
  // !need_comma implies that the list was empty.  JSON convention is to
  // put a space between an empty pair of square brackets.
  if (!need_comma)
    fputc(' ', stderr);
  fputs("]\n", stderr);
  fflush(stderr);
}

hunits kern_pair_node::width()
{
  return n1->width() + n2->width() + amount;
}

node *kern_pair_node::last_char_node()
{
  node *nd = n2->last_char_node();
  if (nd != 0 /* nullptr */)
    return nd;
  return n1->last_char_node();
}

hunits dbreak_node::width()
{
  hunits x = H0;
  for (node *n = none; n != 0 /* nullptr */; n = n->next)
    x += n->width();
  return x;
}

node *dbreak_node::last_char_node()
{
  for (node *n = none; n != 0 /* nullptr */; n = n->next) {
    node *last_node = n->last_char_node();
    if (last_node)
      return last_node;
  }
  return 0 /* nullptr */;
}

hunits dbreak_node::italic_correction()
{
  return none ? none->italic_correction() : H0;
}

hunits dbreak_node::subscript_correction()
{
  return none ? none->subscript_correction() : H0;
}

class italic_corrected_node : public container_node {
  hunits x;
public:
  italic_corrected_node(node *, hunits, statem *, int,
			node * = 0 /* nullptr */);
  node *copy();
  void ascii_print(ascii_output_file *);
  void asciify(macro *);
  hunits width();
  node *last_char_node();
  void vertical_extent(vunits *, vunits *);
  int ends_sentence();
  bool overlaps_horizontally();
  bool overlaps_vertically();
  bool is_same_as(node *);
  hyphenation_type get_hyphenation_type();
  tfont *get_tfont();
  hyphen_list *get_hyphen_list(hyphen_list *, int *);
  int character_type();
  void tprint(troff_output_file *);
  hunits subscript_correction();
  hunits skew();
  node *add_self(node *, hyphen_list **);
  const char *type();
  bool causes_tprint();
  bool is_tag();
  void dump_properties();
};

node *node::add_italic_correction(hunits *wd)
{
  hunits ic = italic_correction();
  if (ic.is_zero())
    return this;
  else {
    node *next1 = next;
    next = 0 /* nullptr */;
    *wd += ic;
    return new italic_corrected_node(this, ic, state, div_nest_level,
				     next1);
  }
}

italic_corrected_node::italic_corrected_node(node *nn, hunits xx,
					     statem *s, int divlevel,
					     node *p)
: container_node(p, s, divlevel, nn), x(xx)
{
  assert(nodes != 0 /* nullptr */);
}

void italic_corrected_node::dump_properties()
{
  node::dump_properties();
  fprintf(stderr, ", \"hunits\": %d", x.to_units());
  fflush(stderr);
}

node *italic_corrected_node::copy()
{
  return new italic_corrected_node(nodes->copy(), x, state,
				   div_nest_level);
}

hunits italic_corrected_node::width()
{
  return nodes->width() + x;
}

void italic_corrected_node::vertical_extent(vunits *min, vunits *max)
{
  nodes->vertical_extent(min, max);
}

void italic_corrected_node::tprint(troff_output_file *out)
{
  nodes->tprint(out);
  out->right(x);
}

hunits italic_corrected_node::skew()
{
  return nodes->skew() - (x / 2);
}

hunits italic_corrected_node::subscript_correction()
{
  return nodes->subscript_correction() - x;
}

void italic_corrected_node::ascii_print(ascii_output_file *out)
{
  nodes->ascii_print(out);
}

int italic_corrected_node::ends_sentence()
{
  return nodes->ends_sentence();
}

bool italic_corrected_node::overlaps_horizontally()
{
  return nodes->overlaps_horizontally();
}

bool italic_corrected_node::overlaps_vertically()
{
  return nodes->overlaps_vertically();
}

node *italic_corrected_node::last_char_node()
{
  return nodes->last_char_node();
}

tfont *italic_corrected_node::get_tfont()
{
  return nodes->get_tfont();
}

hyphenation_type italic_corrected_node::get_hyphenation_type()
{
  return nodes->get_hyphenation_type();
}

node *italic_corrected_node::add_self(node *nd, hyphen_list **p)
{
  nd = nodes->add_self(nd, p);
  hunits not_interested;
  nd = nd->add_italic_correction(&not_interested);
  nodes = 0 /* nullptr */;
  delete this;
  return nd;
}

hyphen_list *italic_corrected_node::get_hyphen_list(hyphen_list *tail,
						    int *count)
{
  return nodes->get_hyphen_list(tail, count);
}

int italic_corrected_node::character_type()
{
  return nodes->character_type();
}

class break_char_node : public container_node {
  // TODO: Figure out what a hyphenation code means in the UTF-8 future,
  // where a "grochar" is a vector of NFD decomposed code points.  Can
  // it be a scalar--a 32-bit int?
  unsigned char break_code;
  unsigned char prev_break_code;
  color *col;
public:
  break_char_node(node *, int, int, color *, node * = 0 /* nullptr */);
  break_char_node(node *, int, int, color *, statem *, int,
		  node * = 0 /* nullptr */);
  node *copy();
  hunits width();
  vunits vertical_width();
  node *last_char_node();
  int character_type();
  int ends_sentence();
  node *add_self(node *, hyphen_list **);
  hyphen_list *get_hyphen_list(hyphen_list *, int *);
  void tprint(troff_output_file *);
  void zero_width_tprint(troff_output_file *);
  void ascii_print(ascii_output_file *);
  void asciify(macro *);
  hyphenation_type get_hyphenation_type();
  bool overlaps_vertically();
  bool overlaps_horizontally();
  units size();
  tfont *get_tfont();
  bool is_same_as(node *);
  const char *type();
  bool causes_tprint();
  bool is_tag();
  unsigned char get_break_code();
  void dump_properties();
};

break_char_node::break_char_node(node *n, int bc, int pbc, color *c,
				 node *x)
: container_node(x, n), break_code(bc), prev_break_code(pbc), col(c)
{
}

break_char_node::break_char_node(node *n, int bc, int pbc, color *c,
				 statem *s, int divlevel, node *x)
: container_node(x, s, divlevel, n), break_code(bc),
  prev_break_code(pbc), col(c)
{
}

void break_char_node::dump_properties()
{
  node::dump_properties();
  fprintf(stderr, ", \"break code before\": %d", break_code);
  fprintf(stderr, ", \"break code after\": %d", prev_break_code);
  fputs(", \"terminal_color\": ", stderr);
  col->nm.json_dump();
  fflush(stderr);
}

node *break_char_node::copy()
{
  return new break_char_node(nodes->copy(), break_code, prev_break_code,
			     col, state, div_nest_level);
}

hunits break_char_node::width()
{
  return nodes->width();
}

vunits break_char_node::vertical_width()
{
  return nodes->vertical_width();
}

node *break_char_node::last_char_node()
{
  return nodes->last_char_node();
}

int break_char_node::character_type()
{
  return nodes->character_type();
}

int break_char_node::ends_sentence()
{
  return nodes->ends_sentence();
}

// Keep these symbol names in sync with the superset used in an
// anonymous `enum` in "charinfo.h".
enum break_char_type {
  ALLOWS_BREAK_BEFORE = 0x01,
  ALLOWS_BREAK_AFTER = 0x02,
  IGNORES_SURROUNDING_HYPHENATION_CODES = 0x04,
  PROHIBITS_BREAK_BEFORE = 0x08,
  PROHIBITS_BREAK_AFTER = 0x10,
  IS_INTERWORD_SPACE = 0x20
};

node *break_char_node::add_self(node *n, hyphen_list **p)
{
  bool have_space_node = false;
  assert(0U == (*p)->hyphenation_code);
  if (break_code & ALLOWS_BREAK_BEFORE) {
    if (((*p)->is_breakable)
	|| (break_code & IGNORES_SURROUNDING_HYPHENATION_CODES)) {
      n = new space_node(H0, col, n);
      n->freeze_space();
      have_space_node = true;
    }
  }
  if (!have_space_node) {
    if ((prev_break_code & IS_INTERWORD_SPACE)
	|| (prev_break_code & PROHIBITS_BREAK_AFTER)) {
      if (break_code & PROHIBITS_BREAK_BEFORE)
	// stretchable zero-width space not implemented yet
	;
      else {
	// breakable, stretchable zero-width space not implemented yet
	n = new space_node(H0, col, n);
	n->freeze_space();
      }
    }
  }
  next = n;
  n = this;
  if (break_code & ALLOWS_BREAK_AFTER) {
    if (((*p)->is_breakable)
	|| (break_code & IGNORES_SURROUNDING_HYPHENATION_CODES)) {
      n = new space_node(H0, col, n);
      n->freeze_space();
    }
  }
  hyphen_list *pp = *p;
  *p = (*p)->next;
  delete pp;
  return n;
}

hyphen_list *break_char_node::get_hyphen_list(hyphen_list *tail, int *)
{
  return new hyphen_list(0, tail);
}

hyphenation_type break_char_node::get_hyphenation_type()
{
  return HYPHENATION_PERMITTED;
}

void break_char_node::ascii_print(ascii_output_file *ascii)
{
  nodes->ascii_print(ascii);
}

bool break_char_node::overlaps_vertically()
{
  return nodes->overlaps_vertically();
}

bool break_char_node::overlaps_horizontally()
{
  return nodes->overlaps_horizontally();
}

units break_char_node::size()
{
  return nodes->size();
}

tfont *break_char_node::get_tfont()
{
  return nodes->get_tfont();
}

node *extra_size_node::copy()
{
  return new extra_size_node(n, state, div_nest_level);
}

extra_size_node::extra_size_node(vunits i, statem *s, int divlevel)
: node(0 /* nullptr */, s, divlevel), n(i)
{
}

extra_size_node::extra_size_node(vunits i)
: n(i)
{
}

void extra_size_node::dump_properties()
{
  node::dump_properties();
  fprintf(stderr, ", \"vunits\": %d", n.to_units());
  fflush(stderr);
}

node *vertical_size_node::copy()
{
  return new vertical_size_node(n, state, div_nest_level);
}

vertical_size_node::vertical_size_node(vunits i, statem *s,
				       int divlevel)
: node(0 /* nullptr */, s, divlevel), n(i)
{
}

vertical_size_node::vertical_size_node(vunits i)
: n(i)
{
}

void vertical_size_node::dump_properties()
{
  node::dump_properties();
  fprintf(stderr, ", \"vunits\": %d", n.to_units());
  fflush(stderr);
}

void hmotion_node::dump_properties()
{
  node::dump_properties();
  fprintf(stderr, ", \"hunits\": %d", n.to_units());
  fprintf(stderr, ", \"was_tab\": %s", was_tab ? "true" : "false");
  fprintf(stderr, ", \"unformat\": %s", unformat ? "true" : "false");
  fputs(", \"terminal_color\": ", stderr);
  col->nm.json_dump();
  fflush(stderr);
}

node *hmotion_node::copy()
{
  return new hmotion_node(n, was_tab, unformat, col, state,
			  div_nest_level);
}

node *space_char_hmotion_node::copy()
{
  return new space_char_hmotion_node(n, col, state, div_nest_level);
}

vmotion_node::vmotion_node(vunits i, color *c)
: n(i), col(c)
{
}

vmotion_node::vmotion_node(vunits i, color *c, statem *s, int divlevel)
: node(0 /* nullptr */, s, divlevel), n(i), col(c)
{
}

void vmotion_node::dump_properties()
{
  node::dump_properties();
  fprintf(stderr, ", \"vunits\": %d", n.to_units());
  fputs(", \"terminal_color\": ", stderr);
  col->nm.json_dump();
  fflush(stderr);
}

node *vmotion_node::copy()
{
  return new vmotion_node(n, col, state, div_nest_level);
}

node *dummy_node::copy()
{
  return new dummy_node;
}

node *transparent_dummy_node::copy()
{
  return new transparent_dummy_node;
}

hline_node::hline_node(hunits i, node *c, node *nxt)
: container_node(nxt, c), x(i)
{
}

hline_node::hline_node(hunits i, node *c, statem *s, int divlevel,
		       node *nxt)
: container_node(nxt, s, divlevel, c), x(i)
{
}

void hline_node::dump_properties()
{
  node::dump_properties();
  fprintf(stderr, ", \"hunits\": %d", x.to_units());
  fflush(stderr);
}

node *hline_node::copy()
{
  return new hline_node(x, (nodes != 0 /* nullptr */) ? nodes->copy()
						      : 0 /* nullptr */,
			state, div_nest_level);
}

hunits hline_node::width()
{
  return x < H0 ? H0 : x;
}

vline_node::vline_node(vunits i, node *c, node *nxt)
: container_node(nxt, c), x(i)
{
}

vline_node::vline_node(vunits i, node *c, statem *s,
		       int divlevel, node *nxt)
: container_node(nxt, s, divlevel, c), x(i)
{
}

void vline_node::dump_properties()
{
  node::dump_properties();
  fprintf(stderr, ", \"vunits\": %d", x.to_units());
  fflush(stderr);
}

node *vline_node::copy()
{
  return new vline_node(x, (nodes != 0 /* nullptr */) ? nodes->copy()
						      : 0 /* nullptr */,
			state, div_nest_level);
}

hunits vline_node::width()
{
  return (0 /* nullptr */ == nodes) ? H0 : nodes->width();
}

zero_width_node::zero_width_node(node *nd, statem *s, int divlevel)
: container_node(0 /* nullptr */, s, divlevel, nd)
{
}

zero_width_node::zero_width_node(node *nd)
: container_node(nd)
{
}

node *zero_width_node::copy()
{
  return new zero_width_node(copy_node_list(nodes), state,
			     div_nest_level);
}

int node_list_character_type(node *p)
{
  int t = 0;
  for (; p != 0 /* nullptr */; p = p->next)
    t |= p->character_type();
  return t;
}

int zero_width_node::character_type()
{
  return node_list_character_type(nodes);
}

void node_list_vertical_extent(node *p, vunits *min, vunits *max)
{
  *min = V0;
  *max = V0;
  vunits cur_vpos = V0;
  vunits v1, v2;
  for (; p != 0 /* nullptr */; p = p->next) {
    p->vertical_extent(&v1, &v2);
    v1 += cur_vpos;
    if (v1 < *min)
      *min = v1;
    v2 += cur_vpos;
    if (v2 > *max)
      *max = v2;
    cur_vpos += p->vertical_width();
  }
}

void zero_width_node::vertical_extent(vunits *min, vunits *max)
{
  node_list_vertical_extent(nodes, min, max);
}

overstrike_node::overstrike_node()
: container_node(0 /* nullptr */), max_width(H0)
{
}

overstrike_node::overstrike_node(statem *s, int divlevel)
: container_node(0 /* nullptr */, s, divlevel), max_width(H0)
{
}

void overstrike_node::dump_properties()
{
  node::dump_properties();
  fprintf(stderr, ", \"max_width\": %d", max_width.to_units());
  fflush(stderr);
}

node *overstrike_node::copy()
{
  overstrike_node *on = new overstrike_node(state, div_nest_level);
  for (node *tem = nodes; tem != 0 /* nullptr */; tem = tem->next)
    on->overstrike(tem->copy());
  return on;
}

void overstrike_node::overstrike(node *n)
{
  if (0 /* nullptr */ == n)
    return;
  hunits w = n->width();
  if (w > max_width)
    max_width = w;
  node **p;
  for (p = &nodes; *p != 0 /* nullptr */; p = &(*p)->next)
    ;
  n->next = 0 /* nullptr */;
  *p = n;
}

hunits overstrike_node::width()
{
  return max_width;
}

bracket_node::bracket_node()
: container_node(0 /* nullptr */), max_width(H0)
{
}

bracket_node::bracket_node(statem *s, int divlevel)
: container_node(0 /* nullptr */, s, divlevel), max_width(H0)
{
}

void bracket_node::dump_properties()
{
  node::dump_properties();
  fprintf(stderr, ", \"max_width\": %d", max_width.to_units());
  fflush(stderr);
}

node *bracket_node::copy()
{
  bracket_node *on = new bracket_node(state, div_nest_level);
  node *last_node = 0 /* nullptr */;
  node *tem;
  if (nodes != 0 /* nullptr */)
    nodes->last = 0 /* nullptr */;
  for (tem = nodes; tem != 0 /* nullptr */; tem = tem->next) {
    if (tem->next)
      tem->next->last = tem;
    last_node = tem;
  }
  for (tem = last_node; tem != 0 /* nullptr */; tem = tem->last)
    on->bracket(tem->copy());
  return on;
}

void bracket_node::bracket(node *n)
{
  if (0 /* nullptr */ == n)
    return;
  hunits w = n->width();
  if (w > max_width)
    max_width = w;
  n->next = nodes;
  nodes = n;
}

hunits bracket_node::width()
{
  return max_width;
}

int node::nspaces()
{
  return 0;
}

bool node::did_space_merge(hunits, hunits, hunits)
{
  return false;
}


space_node::space_node(hunits nn, color *c, node *p)
: node(p, 0 /* nullptr */, 0), n(nn), set('\0'),
  was_escape_colon(false), col(c)
{
}

space_node::space_node(hunits nn, int s, int flag, color *c, statem *st,
		       int divlevel, node *p)
: node(p, st, divlevel), n(nn), set(s), was_escape_colon(flag), col(c)
{
}

void space_node::dump_properties()
{
  node::dump_properties();
  fprintf(stderr, ", \"hunits\": %d", n.to_units());
  fprintf(stderr, ", \"undiscardable\": %s", set ? "true" : "false");
  fprintf(stderr, ", \"is hyphenless breakpoint\": %s",
	  was_escape_colon ? "true" : "false");
  fputs(", \"terminal_color\": ", stderr);
  col->nm.json_dump();
  fflush(stderr);
}

#if 0
space_node::~space_node()
{
}
#endif

node *space_node::copy()
{
  return new space_node(n, set, was_escape_colon, col, state,
			div_nest_level);
}

bool space_node::causes_tprint()
{
  return false;
}

bool space_node::is_tag()
{
  return false;
}

int space_node::nspaces()
{
  return set ? 0 : 1;
}

bool space_node::did_space_merge(hunits h, hunits, hunits)
{
  n += h;
  return true;
}

hunits space_node::width()
{
  return n;
}

void node::spread_space(int*, hunits*)
{
}

void space_node::spread_space(int *n_spaces, hunits *desired_space)
{
  if (!set) {
    assert(*n_spaces > 0);
    if (*n_spaces == 1) {
      n += *desired_space;
      *desired_space = H0;
    }
    else {
      hunits extra = *desired_space / *n_spaces;
      *desired_space -= extra;
      n += extra;
    }
    *n_spaces -= 1;
    set = true;
  }
}

void node::freeze_space()
{
}

void space_node::freeze_space()
{
  set = true;
}

void node::is_escape_colon()
{
}

void space_node::is_escape_colon()
{
  was_escape_colon = true;
}

diverted_space_node::diverted_space_node(vunits d, statem *s,
					 int divlevel, node *p)
: node(p, s, divlevel), n(d)
{
}

diverted_space_node::diverted_space_node(vunits d, node *p)
: node(p), n(d)
{
}

void diverted_space_node::dump_properties()
{
  node::dump_properties();
  fprintf(stderr, ", \"vunits\": %d", n.to_units());
  fflush(stderr);
}

node *diverted_space_node::copy()
{
  return new diverted_space_node(n, state, div_nest_level);
}

diverted_copy_file_node::diverted_copy_file_node(symbol s, statem *st,
						 int divlevel, node *p)
: node(p, st, divlevel), filename(s)
{
}

diverted_copy_file_node::diverted_copy_file_node(symbol s, node *p)
: node(p), filename(s)
{
}

void diverted_copy_file_node::dump_properties()
{
  node::dump_properties();
  fputs(", \"filename\": ", stderr);
  filename.json_dump();
  fflush(stderr);
}

node *diverted_copy_file_node::copy()
{
  return new diverted_copy_file_node(filename, state, div_nest_level);
}

int node::ends_sentence()
{
  return 0;
}

int kern_pair_node::ends_sentence()
{
  switch (n2->ends_sentence()) {
  case 0:
    return 0;
  case 1:
    return 1;
  case 2:
    break;
  default:
    assert(0 == "unhandled case of sentence ending status");
  }
  return n1->ends_sentence();
}

int node_list_ends_sentence(node *n)
{
  for (; n != 0 /* nullptr */; n = n->next)
    switch (n->ends_sentence()) {
    case 0:
      return 0;
    case 1:
      return 1;
    case 2:
      break;
    default:
      assert(0 == "unhandled case of sentence ending status");
    }
  return 2;
}

int dbreak_node::ends_sentence()
{
  return node_list_ends_sentence(none);
}

bool node::overlaps_horizontally()
{
  return false;
}

bool node::overlaps_vertically()
{
  return false;
}

bool node::discardable()
{
  return false;
}

bool space_node::discardable()
{
  return !set;
}

vunits node::vertical_width()
{
  return V0;
}

vunits vline_node::vertical_width()
{
  return x;
}

vunits vmotion_node::vertical_width()
{
  return n;
}

bool node::set_unformat_flag()
{
  return true;
}

int node::character_type()
{
  return 0;
}

hunits node::subscript_correction()
{
  return H0;
}

hunits node::italic_correction()
{
  return H0;
}

hunits node::left_italic_correction()
{
  return H0;
}

hunits node::skew()
{
  return H0;
}

/* vertical_extent methods */

void node::vertical_extent(vunits *min, vunits *max)
{
  vunits v = vertical_width();
  if (v < V0) {
    *min = v;
    *max = V0;
  }
  else {
    *max = v;
    *min = V0;
  }
}

void vline_node::vertical_extent(vunits *min, vunits *max)
{
  if (0 /* nullptr */ == nodes)
    node::vertical_extent(min, max);
  else {
    vunits cmin, cmax;
    nodes->vertical_extent(&cmin, &cmax);
    vunits h = nodes->size();
    if (x < V0) {
      if (-x < h) {
	*min = x;
	*max = V0;
      }
      else {
	// we print the first character and then move up, so
	*max = cmax;
	// we print the last character and then move up h
	*min = cmin + h;
	if (*min > V0)
	  *min = V0;
	*min += x;
      }
    }
    else {
      if (x < h) {
	*max = x;
	*min = V0;
      }
      else {
	// we move down by h and then print the first character, so
	*min = cmin + h;
	if (*min > V0)
	  *min = V0;
	*max = x + cmax;
      }
    }
  }
}

// `ascii_print()` represents a node for `troff -a`

static void ascii_print_node_list(ascii_output_file *ascii, node *n)
{
  node *nn = n;
  while(nn != 0 /* nullptr */) {
    nn->ascii_print(ascii);
    nn = nn->next;
  }
}

static void ascii_print_reverse_node_list(ascii_output_file *ascii,
					  node *n)
{
  if (0 /* nullptr */ == n)
    return;
  ascii_print_reverse_node_list(ascii, n->next);
  n->ascii_print(ascii);
}

void bracket_node::ascii_print(ascii_output_file *ascii)
{
  ascii_print_reverse_node_list(ascii, nodes);
}

void dbreak_node::ascii_print(ascii_output_file *ascii)
{
  ascii_print_reverse_node_list(ascii, none);
}

void kern_pair_node::ascii_print(ascii_output_file *ascii)
{
  n1->ascii_print(ascii);
  n2->ascii_print(ascii);
}

void node::ascii_print(ascii_output_file *)
{
}

void overstrike_node::ascii_print(ascii_output_file *ascii)
{
  ascii_print_node_list(ascii, nodes);
}

void space_node::ascii_print(ascii_output_file *ascii)
{
  if (!n.is_zero())
    ascii->outc(' ');
}

void hmotion_node::ascii_print(ascii_output_file *ascii)
{
  // this is pretty arbitrary
  if (n >= points_to_units(2))
    ascii->outc(' ');
}

void space_char_hmotion_node::ascii_print(ascii_output_file *ascii)
{
  ascii->outc(' ');
}

void zero_width_node::ascii_print(ascii_output_file *out)
{
  ascii_print_node_list(out, nodes);
}

// `asciify()` extracts the simple character content of a node; this is
// a plain text (but not necessarily plain "ASCII") representation
// suitable for storage in a groff string or embedding in a device
// extension command escape sequence (as for PDF metadata).

void glyph_node::asciify(macro *m)
{
  if (!is_output_supressed) {
    unsigned char c = ci->get_asciify_code();
    if (c != 0U)
      m->append(c);
    else {
      c = ci->get_ascii_code();
      if (c != 0U)
	m->append(c);
      else {
	// Also see input.cpp::charinfo::dump().
	int unicode_mapping = ci->get_unicode_mapping();
	if (unicode_mapping >= 0) {
	  // We must write out an escape sequence.  Use the default
	  // escape character.  TODO: Make `escape_char` global?
	  //
	  // First, handle the Basic Latin characters that don't map to
	  // themselves.
	  switch (unicode_mapping) {
	  case 34:
	    m->append_str("\\[dq]");
	    break;
	  case 39:
	    m->append_str("\\[aq]");
	    break;
	  case 45:
	    m->append_str("\\[-]");
	    break;
	  case 92:
	    m->append_str("\\[rs]");
	    break;
	  case 94:
	    m->append_str("\\[ha]");
	    break;
	  case 96:
	    m->append_str("\\[ga]");
	    break;
	  case 126:
	    m->append_str("\\[ti]");
	    break;
	  default:
	    m->append_str("\\[u");
	    const size_t buflen = sizeof "10FFFF";
	    char hexbuf[buflen];
	    (void) memset(hexbuf, '\0', buflen);
	    (void) snprintf(hexbuf, buflen, "%.4X", unicode_mapping);
	    m->append_str(hexbuf);
	    m->append(']');
	    break;
	  }
	}
	else {
	  error("unable to asciify glyph; charinfo data follows");
	  // This is garrulous as hell, but by the time we have hold of
	  // a glyph's charinfo, it no longer has a "name"--it's already
	  // been looked up in the dictionary.  (Also, multiple names
	  // can refer to the same charinfo datum.)  And this racket
	  // beats telling the user nothing at all about the glyph: if
	  // the character was defined by request (`char` et al.), this
	  // dump reports the file name and line number of that request.
	  ci->dump();
	}
      }
    }
  }
}

void kern_pair_node::asciify(macro *m)
{
  if (!is_output_supressed) {
    if (n1 != 0 /* nullptr */)
      n1->asciify(m);
    if (n2 != 0 /* nullptr */)
      n2->asciify(m);
  }
}

void dbreak_node::asciify(macro *m)
{
  assert(m != 0 /* nullptr */);
  if (!is_output_supressed) {
    if (m != 0 /* nullptr */)
      none->asciify(m);
    none = 0 /* nullptr */;
  }
}

void ligature_node::asciify(macro *m)
{
  assert(n1 != 0 /* nullptr */);
  assert(n2 != 0 /* nullptr */);
  if (!is_output_supressed) {
    if (n1 != 0 /* nullptr */)
      n1->asciify(m);
    if (n2 != 0 /* nullptr */)
      n2->asciify(m);
  }
}

void break_char_node::asciify(macro *m)
{
  assert(nodes != 0 /* nullptr */);
  if (!is_output_supressed && (nodes != 0 /* nullptr */))
    nodes->asciify(m);
  nodes = 0 /* nullptr */;
}

void italic_corrected_node::asciify(macro *m)
{
  assert(nodes != 0 /* nullptr */);
  if (!is_output_supressed && (nodes != 0 /* nullptr */))
    nodes->asciify(m);
  nodes = 0 /* nullptr */;
}

void left_italic_corrected_node::asciify(macro *m)
{
  assert(nodes != 0 /* nullptr */);
  if (!is_output_supressed && (nodes != 0 /* nullptr */))
    nodes->asciify(m);
  nodes = 0 /* nullptr */;
}

void hmotion_node::asciify(macro *)
{
}

space_char_hmotion_node::space_char_hmotion_node(hunits i, color *c,
						 statem *s,
						 int divlevel,
						 node *nxt)
: hmotion_node(i, c, s, divlevel, nxt)
{
}

space_char_hmotion_node::space_char_hmotion_node(hunits i, color *c,
						 node *nxt)
: hmotion_node(i, c, 0 /* nullptr */, 0, nxt)
{
}

void space_char_hmotion_node::asciify(macro *m)
{
  if (!is_output_supressed)
    m->append(' ');
}

void space_node::asciify(macro *)
{
}

void word_space_node::asciify(macro *m)
{
  if (!is_output_supressed) {
    for (width_list *w = orig_width; w != 0 /* nullptr */; w = w->next)
      m->append(' ');
  }
}

void unbreakable_space_node::asciify(macro *m)
{
  if (!is_output_supressed)
    m->append(' ');
}

void line_start_node::asciify(macro *)
{
}

void vertical_size_node::asciify(macro *)
{
}

void dummy_node::asciify(macro *)
{
}

void transparent_dummy_node::asciify(macro *)
{
}

void tag_node::asciify(macro *)
{
}

void device_extension_node::asciify(macro *)
{
}

void vmotion_node::asciify(macro *)
{
}

void bracket_node::asciify(macro *)
{
}

void diverted_copy_file_node::asciify(macro *)
{
}

void diverted_space_node::asciify(macro *)
{
}

void draw_node::asciify(macro *)
{
}

void extra_size_node::asciify(macro *)
{
}

void hline_node::asciify(macro *)
{
}

void hyphen_inhibitor_node::asciify(macro *)
{
}

void overstrike_node::asciify(macro *)
{
}

void suppress_node::asciify(macro *)
{
  is_output_supressed = (is_on == 0); // it's a three-valued Boolean :-/
}

void vline_node::asciify(macro *)
{
}

// We probably would asciify zero-width nodes as nothing, but they're
// used internally to represent some forms of combining character, as
// with \[u015E] -> S<ac>.
void zero_width_node::asciify(macro *m)
{
  assert(nodes != 0 /* nullptr */);
  if (!is_output_supressed) {
    node *n = nodes;
    while (n != 0 /* nullptr */) {
      n->asciify(m);
      n = n->next;
    }
    nodes = 0 /* nullptr */;
  }
}

breakpoint *node::get_breakpoints(hunits /* width */, int /* nspaces */,
				  breakpoint *rest, bool /* is_inner */)
{
  return rest;
}

int node::nbreaks()
{
  return 0;
}

breakpoint *space_node::get_breakpoints(hunits wd, int ns,
					breakpoint *rest, bool is_inner)
{
  if (next && next->discardable())
    return rest;
  breakpoint *bp = new breakpoint;
  bp->next = rest;
  bp->width = wd;
  bp->nspaces = ns;
  bp->hyphenated = 0;
  if (is_inner) {
    assert(rest != 0);
    bp->index = rest->index + 1;
    bp->nd = rest->nd;
  }
  else {
    bp->nd = this;
    bp->index = 0;
  }
  return bp;
}

int space_node::nbreaks()
{
  if (next && next->discardable())
    return 0;
  else
    return 1;
}

static breakpoint *node_list_get_breakpoints(node *p, hunits *widthp,
					     int ns, breakpoint *rest)
{
  if (p != 0 /* nullptr */) {
    rest = p->get_breakpoints(*widthp,
			      ns,
			      node_list_get_breakpoints(p->next, widthp,
							ns, rest),
			      true);
    *widthp += p->width();
  }
  return rest;
}

breakpoint *dbreak_node::get_breakpoints(hunits wd, int ns,
					 breakpoint *rest,
					 bool is_inner)
{
  breakpoint *bp = new breakpoint;
  bp->next = rest;
  bp->width = wd;
  for (node *tem = pre; tem != 0 /* nullptr */; tem = tem->next)
    bp->width += tem->width();
  bp->nspaces = ns;
  bp->hyphenated = 1;
  if (is_inner) {
    assert(rest != 0);
    bp->index = rest->index + 1;
    bp->nd = rest->nd;
  }
  else {
    bp->nd = this;
    bp->index = 0;
  }
  return node_list_get_breakpoints(none, &wd, ns, bp);
}

int dbreak_node::nbreaks()
{
  int i = 1;
  for (node *tem = none; tem != 0 /* nullptr */; tem = tem->next)
    i += tem->nbreaks();
  return i;
}

void node::split(int /*where*/, node ** /*prep*/, node ** /*postp*/)
{
  assert(0 == "node::split() unimplemented");
}

void space_node::split(int where, node **pre, node **post)
{
  assert(0 == where);
  *pre = next;
  *post = 0 /* nullptr */;
  delete this;
}

static void node_list_split(node *p, int *wherep,
			    node **prep, node **postp)
{
  if (0 /* nullptr */ == p)
    return;
  int nb = p->nbreaks();
  node_list_split(p->next, wherep, prep, postp);
  if (*wherep < 0) {
    p->next = *postp;
    *postp = p;
  }
  else if (*wherep < nb) {
    p->next = *prep;
    p->split(*wherep, prep, postp);
  }
  else {
    p->next = *prep;
    *prep = p;
  }
  *wherep -= nb;
}

void dbreak_node::split(int where, node **prep, node **postp)
{
  assert(where >= 0);
  if (0 == where) {
    *postp = post;
    post = 0 /* nullptr */;
    if (0 /* nullptr */ == pre)
      *prep = next;
    else {
      node *tem;
      for (tem = pre; tem->next != 0 /* nullptr */; tem = tem->next)
	;
      tem->next = next;
      *prep = pre;
    }
    pre = 0 /* nullptr */;
    delete this;
  }
  else {
    *prep = next;
    where -= 1;
    node_list_split(none, &where, prep, postp);
    none = 0 /* nullptr */;
    delete this;
  }
}

// TODO: Make this member function pure virtual to force consideration
// of this question for each node type.
hyphenation_type node::get_hyphenation_type()
{
  return HYPHENATION_UNNECESSARY;
}

hyphenation_type dbreak_node::get_hyphenation_type()
{
  return HYPHENATION_INHIBITED;
}

hyphenation_type kern_pair_node::get_hyphenation_type()
{
  return HYPHENATION_PERMITTED;
}

hyphenation_type dummy_node::get_hyphenation_type()
{
  return HYPHENATION_PERMITTED;
}

hyphenation_type transparent_dummy_node::get_hyphenation_type()
{
  return HYPHENATION_PERMITTED;
}

hyphenation_type hmotion_node::get_hyphenation_type()
{
  return HYPHENATION_PERMITTED;
}

hyphenation_type space_char_hmotion_node::get_hyphenation_type()
{
  return HYPHENATION_PERMITTED;
}

hyphenation_type overstrike_node::get_hyphenation_type()
{
  return HYPHENATION_PERMITTED;
}

hyphenation_type space_node::get_hyphenation_type()
{
  if (was_escape_colon)
    return HYPHENATION_PERMITTED;
  return HYPHENATION_UNNECESSARY;
}

hyphenation_type unbreakable_space_node::get_hyphenation_type()
{
  return HYPHENATION_PERMITTED;
}

bool node::interpret(macro *)
{
  return false;
}

device_extension_node::device_extension_node(const macro &m, bool b)
: node(0 /* nullptr */, 0 /* nullptr */, 0, true), mac(m),
  lacks_command_prefix(b)
{
  font_size fs = curenv->get_font_size();
  int char_height = curenv->get_char_height();
  int char_slant = curenv->get_char_slant();
  int fontno = env_resolve_font(curenv);
  tf = font_table[fontno]->get_tfont(fs, char_height, char_slant,
				     fontno);
  if (curenv->is_composite())
    tf = tf->get_plain();
  gcol = curenv->get_stroke_color();
  fcol = curenv->get_fill_color();
}

device_extension_node::device_extension_node(const macro &m, tfont *t,
			   color *gc, color *fc,
			   statem *s, int divlevel,
			   bool b)
: node(0 /* nullptr */, s, divlevel, true), mac(m), tf(t), gcol(gc),
  fcol(fc), lacks_command_prefix(b)
{
}

void device_extension_node::dump_properties()
{
  node::dump_properties();
  fputs(", \"macro\": ", stderr);
  mac.json_dump();
  fputs(", \"tfont\": ", stderr);
  tf->get_name().json_dump();
  fputs(", \"stroke_color\": ", stderr);
  gcol->nm.json_dump();
  fputs(", \"fill_color\": ", stderr);
  fcol->nm.json_dump();
  fflush(stderr);
}

bool device_extension_node::is_same_as(node *n)
{
  return ((mac == static_cast<device_extension_node *>(n)->mac)
	  && (tf == static_cast<device_extension_node *>(n)->tf)
	  && (gcol == static_cast<device_extension_node *>(n)->gcol)
	  && (fcol == static_cast<device_extension_node *>(n)->fcol)
	  && (lacks_command_prefix
	      == static_cast<device_extension_node *>(n)
		 ->lacks_command_prefix));
}

const char *device_extension_node::type()
{
  return "device extension command node";
}

int device_extension_node::ends_sentence()
{
  return 2;
}

bool device_extension_node::causes_tprint()
{
  return false;
}

hyphenation_type device_extension_node::get_hyphenation_type()
{
  return HYPHENATION_PERMITTED;
}

bool device_extension_node::is_tag()
{
  return false;
}

node *device_extension_node::copy()
{
  return new device_extension_node(mac, tf, gcol, fcol, state,
				   div_nest_level,
				   lacks_command_prefix);
}

void device_extension_node::tprint_start(troff_output_file *out)
{
  out->start_device_extension(tf, gcol, fcol, lacks_command_prefix);
}

void device_extension_node::tprint_char(troff_output_file *out,
					unsigned char c)
{
  out->write_device_extension_char(c);
}

void device_extension_node::tprint_end(troff_output_file *out)
{
  out->end_device_extension();
}

tfont *device_extension_node::get_tfont()
{
  return tf;
}

/* suppress_node */

suppress_node::suppress_node(int on_or_off, int issue_limits)
: is_on(on_or_off), emit_limits(issue_limits), filename(0), position(0),
  image_id(0)
{
}

suppress_node::suppress_node(symbol f, char p, int id)
: node(0 /* nullptr */, 0 /* nullptr */, 0, true), is_on(2),
  emit_limits(false), filename(f), position(p), image_id(id)
{
}

suppress_node::suppress_node(int issue_limits, int on_or_off,
			     symbol f, char p, int id,
			     statem *s, int divlevel)
: node(0 /* nullptr */, s, divlevel), is_on(on_or_off),
  emit_limits(issue_limits), filename(f), position(p), image_id(id)
{
}

void suppress_node::dump_properties()
{
  node::dump_properties();
  fprintf(stderr, ", \"is_on\": %d", is_on);
  fprintf(stderr, ", \"emit_limits\": %s",
	  emit_limits ? "true" : "false");
  if (filename.contents() != 0 /* nullptr */) {
    fputs(", \"filename\": ", stderr);
    filename.json_dump();
  }
  fputs(", \"position\": \"", stderr);
  json_char jc = json_encode_char(position);
  // Write out its JSON representation by character by character to
  // keep libc string functions from interpreting C escape sequences.
  for (size_t i = 0; i < jc.len; i++)
    fputc(jc.buf[i], stderr);
  fputc('\"', stderr);
  fprintf(stderr, ", \"image_id\": %d", image_id);
  fflush(stderr);
}

bool suppress_node::is_same_as(node *n)
{
  return ((is_on == static_cast<suppress_node *>(n)->is_on)
	  && (emit_limits == static_cast<suppress_node *>(n)->emit_limits)
	  && (filename == static_cast<suppress_node *>(n)->filename)
	  && (position == static_cast<suppress_node *>(n)->position)
	  && (image_id == static_cast<suppress_node *>(n)->image_id));
}

const char *suppress_node::type()
{
  return "suppressed output node";
}

node *suppress_node::copy()
{
  return new suppress_node(emit_limits, is_on, filename, position,
			   image_id, state, div_nest_level);
}

/* tag_node */

tag_node::tag_node()
: node(0 /* nullptr */, 0 /* nullptr */, 0, true), delayed(false)
{
}

tag_node::tag_node(string s, int delay)
: tag_string(s), delayed(delay)
{
  is_special = !delay;
}

tag_node::tag_node(string s, statem *st, int divlevel, int delay)
: node(0 /* nullptr */, st, divlevel), tag_string(s), delayed(delay)
{
  is_special = !delay;
}

void tag_node::dump_properties()
{
  node::dump_properties();
  fputs(", \"string\": ", stderr);
  tag_string.json_dump();
  fprintf(stderr, ", \"delayed\": %s", delayed ? "true" : "false");
  fflush(stderr);
}

node *tag_node::copy()
{
  return new tag_node(tag_string, state, div_nest_level, delayed);
}

void tag_node::tprint(troff_output_file *out)
{
  if (delayed)
    out->add_to_tag_list(tag_string);
  else
    out->state.add_tag(out->fp, tag_string);
}

bool tag_node::is_same_as(node *nd)
{
  return ((tag_string == static_cast<tag_node *>(nd)->tag_string)
	  && (delayed == static_cast<tag_node *>(nd)->delayed));
}

const char *tag_node::type()
{
  return "tag node";
}

bool tag_node::causes_tprint()
{
  return !delayed;
}

bool tag_node::is_tag()
{
  return !delayed;
}

int tag_node::ends_sentence()
{
  return 2;
}

// Get contents of register `p` as integer.
// Used only by suppress_node::tprint().
static int get_register(const char *p)
{
  assert(p != 0 /* nullptr */);
  reg *r = static_cast<reg *>(register_dictionary.lookup(p));
  assert(r != 0 /* nullptr */);
  units value;
  assert(r->get_value(&value));
  return int(value);
}

// Get contents of register `p` as string.
// Used only by suppress_node::tprint().
static const char *get_string(const char *p)
{
  assert(p != 0 /* nullptr */);
  reg *r = static_cast<reg *>(register_dictionary.lookup(p));
  assert(r != 0 /* nullptr */);
  return r->get_string();
}

void suppress_node::put(troff_output_file *out, const char *s)
{
  int i = 0;
  while (s[i] != '\0') {
    out->write_device_extension_char(s[i]);
    i++;
  }
}

/*
 *  We need to remember the start of the image and its name (\O5).  But
 *  we won't always need this information; for instance, \O2 is used to
 *  produce a bounding box with no associated image or position thereof.
 */

static char last_position = 'i';
static const char *image_filename = "";
static size_t image_filename_len = 0;
static int subimage_counter = 0;

/*
 *  tprint - if (is_on == 2)
 *               remember current position (l, r, c, i) and filename
 *           else
 *               if (emit_limits)
 *                   if (html)
 *                      emit image tag
 *                   else
 *                      emit postscript bounds for image
 *               else
 *                  if (suppress boolean differs from current state)
 *                      alter state
 *                  reset registers
 *                  record current page
 *                  set low water mark.
 */

void suppress_node::tprint(troff_output_file *out)
{
  int page_number = topdiv->get_page_number();
  // Does the node have an associated position and file name?
  if (is_on == 2) {
    // Save them for future bounding box limits.
    last_position = position;
    image_filename = strsave(filename.contents());
    image_filename_len = strlen(image_filename);
  }
  else { // is_on = 0 or 1
    // Now check whether the suppress node requires us to issue limits.
    if (emit_limits) {
      const size_t namebuflen = 8192;
      char name[namebuflen] = { '\0' };
      // Jump through a flaming hoop to avoid a "format nonliteral"
      // warning from blindly using sprintf...and avoid trouble from
      // mischievous image stems.
      //
      // Keep this format string synced with pre-html:makeFileName().
      const char format[] = "%d";
      const size_t format_len = strlen(format);
      const char *percent_position = strstr(image_filename, format);
      if (percent_position) {
	subimage_counter++;
	assert(sizeof subimage_counter <= 8);
	// A 64-bit signed int produces up to 19 decimal digits.
	const size_t ndigits = 19;
	// Reserve enough for that plus null terminator.
	char *subimage_number
	  = static_cast<char *>(malloc(ndigits + 1));
	if (0 == subimage_number)
	  fatal("memory allocation failure");
	// Replace the %d in the filename with this number.
	size_t enough = image_filename_len + ndigits - format_len;
	char *new_name = static_cast<char *>(malloc(enough));
	if (0 == new_name)
	  fatal("memory allocation failure");
	ptrdiff_t prefix_length = percent_position - image_filename;
	strncpy(new_name, image_filename, prefix_length);
	sprintf(subimage_number, "%d", subimage_counter);
	size_t number_length = strlen(subimage_number);
	strcpy(new_name + prefix_length, subimage_number);
	// Skip over the format in the source string.
	const char *suffix_src = image_filename + prefix_length
	  + format_len;
	char *suffix_dst = new_name + prefix_length + number_length;
	strcpy(suffix_dst, suffix_src);
	// Ensure the new string fits with room for a terminal '\0'.
	const size_t len = strlen(new_name);
	if (len > (namebuflen - 1))
	  error("constructed file name in suppressed output escape"
		" sequence is too long (>= %1 bytes); skipping image",
		int(namebuflen));
	else
	  strncpy(name, new_name, (namebuflen - 1));
	free(new_name);
	free(subimage_number);
      }
      else {
	if (image_filename_len > (namebuflen - 1))
	  error("file name in suppressed output escape sequence is too"
		" long (>= %1 bytes); skipping image", int(namebuflen));
	else
	  strcpy(name, image_filename);
      }
      if (is_writing_html) {
	switch (last_position) {
	case 'c':
	  out->start_device_extension();
	  put(out, "devtag:.centered-image");
	  break;
	case 'r':
	  out->start_device_extension();
	  put(out, "devtag:.right-image");
	  break;
	case 'l':
	  out->start_device_extension();
	  put(out, "devtag:.left-image");
	  break;
	case 'i':
	  ;
	default:
	  ;
	}
	out->end_device_extension();
	out->start_device_extension();
	put(out, "devtag:.auto-image ");
	put(out, name);
	out->end_device_extension();
      }
      else {
	// postscript (or other device)
	if ((suppression_starting_page_number > 0)
	    && (page_number != suppression_starting_page_number))
	  error("suppression limit registers span more than a page;"
		" grohtml-info for image %1 will be wrong", image_no);
	//if (topdiv->get_page_number()
	//    != suppression_starting_page_number)
	//  fprintf(stderr, "end of image and topdiv page = %d   and"
	//	  " suppression_starting_page_number = %d\n",
	//	  topdiv->get_page_number(),
	//	  suppression_starting_page_number);
	// `name` will contain a "%d" in which the image_no is placed.
	fprintf(stderr,
		"grohtml-info:page %d  %d  %d  %d  %d  %d  %s  %d  %d"
		"  %s:%s\n",
		topdiv->get_page_number(),
		get_register("opminx"), get_register("opminy"),
		get_register("opmaxx"), get_register("opmaxy"),
		// page offset + line length
		get_register(".o") + get_register(".l"),
		name, hresolution, vresolution, get_string(".F"),
		get_string(".c"));
	fflush(stderr);
      }
    }
    else { // We are not emitting limits.
      if (is_on) {
	out->on();
	reset_output_registers();
      }
      else
	out->off();
      suppression_starting_page_number = page_number;
    }
  } // is_on
}

bool suppress_node::causes_tprint()
{
  return is_on;
}

bool suppress_node::is_tag()
{
  return is_on;
}

hunits suppress_node::width()
{
  return H0;
}

/* composite_node */

// A composite (glyph) node corresponds to a user-defined GNU troff
// character with a macro definition.

// Not derived from `container_node`; implements custom contained node
// dumper in dump_node().
class composite_node : public charinfo_node {
  node *nodes;
  tfont *tf;
public:
  composite_node(node *, charinfo *, tfont *, statem *, int,
		 node * = 0 /* nullptr */);
  ~composite_node();
  node *copy();
  hunits width();
  node *last_char_node();
  units size();
  void tprint(troff_output_file *);
  hyphenation_type get_hyphenation_type();
  void ascii_print(ascii_output_file *);
  void asciify(macro *);
  hyphen_list *get_hyphen_list(hyphen_list *, int *);
  node *add_self(node *, hyphen_list **);
  tfont *get_tfont();
  bool is_same_as(node *);
  const char *type();
  bool causes_tprint();
  bool is_tag();
  void vertical_extent(vunits *, vunits *);
  vunits vertical_width();
  void dump_properties();
  void dump_node();
};

composite_node::composite_node(node *p, charinfo *c, tfont *t,
			       statem *s, int divlevel, node *x)
: charinfo_node(c, s, divlevel, x), nodes(p), tf(t)
{
}

composite_node::~composite_node()
{
  delete_node_list(nodes);
}

node *composite_node::copy()
{
  return new composite_node(copy_node_list(nodes), ci, tf, state,
			    div_nest_level);
}

hunits composite_node::width()
{
  hunits x;
  if (tf->is_constantly_spaced(&x))
    return x;
  x = H0;
  for (node *tem = nodes; tem != 0 /* nullptr */; tem = tem->next)
    x += tem->width();
  hunits offset;
  if (tf->is_emboldened(&offset))
    x += offset;
  x += tf->get_track_kern();
  return x;
}

node *composite_node::last_char_node()
{
  return this;
}

vunits composite_node::vertical_width()
{
  vunits v = V0;
  for (node *tem = nodes; tem != 0 /* nullptr */; tem = tem->next)
    v += tem->vertical_width();
  return v;
}

units composite_node::size()
{
  return tf->get_size().to_units();
}

hyphenation_type composite_node::get_hyphenation_type()
{
  return HYPHENATION_PERMITTED;
}

void composite_node::asciify(macro *m)
{
  if (!is_output_supressed) {
    unsigned char c = ci->get_asciify_code();
    if (0U == c)
      c = ci->get_ascii_code();
    if (c != 0U)
      m->append(c);
    else
      m->append(this);
  }
}

void composite_node::ascii_print(ascii_output_file *ascii)
{
  ascii_print_reverse_node_list(ascii, nodes);
}

hyphen_list *composite_node::get_hyphen_list(hyphen_list *tail,
					     int *count)
{
  (*count)++;
  return new hyphen_list(ci->get_hyphenation_code(), tail);
}

node *composite_node::add_self(node *nn, hyphen_list **p)
{
  assert(ci->get_hyphenation_code() == (*p)->hyphenation_code);
  next = nn;
  nn = this;
  if ((*p)->is_hyphen)
    nn = nn->add_discretionary_hyphen();
  hyphen_list *pp = *p;
  *p = (*p)->next;
  delete pp;
  return nn;
}

tfont *composite_node::get_tfont()
{
  return tf;
}

node *reverse_node_list(node *n)
{
  node *r = 0 /* nullptr */;
  while (n != 0 /* nullptr */) {
    node *tem = n;
    n = n->next;
    tem->next = r;
    r = tem;
  }
  return r;
}

void composite_node::vertical_extent(vunits *minimum, vunits *maximum)
{
  nodes = reverse_node_list(nodes);
  node_list_vertical_extent(nodes, minimum, maximum);
  nodes = reverse_node_list(nodes);
}

width_list::width_list(hunits w, hunits s)
: width(w), sentence_width(s), next(0 /* nullptr */)
{
}

width_list::width_list(width_list *w)
: width(w->width), sentence_width(w->sentence_width), next(0)
{
}

void width_list::dump()
{
  fputc('[', stderr);
  bool need_comma = false;
  fprintf(stderr, "{ \"width\": %d", width.to_units());
  fprintf(stderr, ", \"sentence_width\": %d }",
	  sentence_width.to_units());
  fflush(stderr);
  width_list *n = this;
  while (n->next != 0 /* nullptr */) {
    if (need_comma)
      fputs(", ", stderr);
    need_comma = true;
    n = n->next;
  }
  fputc(']', stderr);
  fflush(stderr);
}

word_space_node::word_space_node(hunits d, color *c, width_list *w,
				 node *x)
: space_node(d, c, x), orig_width(w), unformat(false)
{
}

word_space_node::word_space_node(hunits d, int s, color *c,
				 width_list *w, bool flag, statem *st,
				 int divlevel, node *x)
: space_node(d, s, 0, c, st, divlevel, x), orig_width(w), unformat(flag)
{
}

void word_space_node::dump_properties()
{
  space_node::dump_properties();
  if (orig_width != 0 /* nullptr */) {
    fputs(", \"width_list\": ", stderr);
    orig_width->dump();
  }
  fprintf(stderr, ", \"unformat\": %s", unformat ? "true" : "false");
  fflush(stderr);
}

word_space_node::~word_space_node()
{
  width_list *w = orig_width;
  while (w != 0) {
    width_list *tmp = w;
    w = w->next;
    delete tmp;
  }
}

node *word_space_node::copy()
{
  assert(orig_width != 0);
  width_list *w_old_curr = orig_width;
  width_list *w_new_curr = new width_list(w_old_curr);
  width_list *w_new = w_new_curr;
  w_old_curr = w_old_curr->next;
  while (w_old_curr != 0) {
    w_new_curr->next = new width_list(w_old_curr);
    w_new_curr = w_new_curr->next;
    w_old_curr = w_old_curr->next;
  }
  return new word_space_node(n, set, col, w_new, unformat, state,
			     div_nest_level);
}

bool word_space_node::set_unformat_flag()
{
  unformat = true;
  return true;
}

void word_space_node::tprint(troff_output_file *out)
{
  out->fill_color(col);
  out->word_marker();
  out->right(n);
}

bool word_space_node::did_space_merge(hunits h, hunits sw, hunits ssw)
{
  n += h;
  assert(orig_width != 0);
  width_list *w = orig_width;
  for (; w->next != 0 /* nullptr */; w = w->next)
    ;
  w->next = new width_list(sw, ssw);
  return true;
}

unbreakable_space_node::unbreakable_space_node(hunits d, color *c,
					       node *x)
: word_space_node(d, c, 0, x)
{
}

unbreakable_space_node::unbreakable_space_node(hunits d, int s,
					       color *c, statem *st,
					       int divlevel,
					       node *x)
: word_space_node(d, s, c, 0, 0, st, divlevel, x)
{
}

node *unbreakable_space_node::copy()
{
  return new unbreakable_space_node(n, set, col, state, div_nest_level);
}

bool unbreakable_space_node::causes_tprint()
{
  return false;
}

bool unbreakable_space_node::is_tag()
{
  return false;
}

breakpoint *unbreakable_space_node::get_breakpoints(hunits, int,
						    breakpoint *rest,
						    bool /* is_inner */)
{
  return rest;
}

int unbreakable_space_node::nbreaks()
{
  return 0;
}

void unbreakable_space_node::split(int, node **, node **)
{
  assert(0 == "unbreakable_space_node::split() unimplemented");
}

bool unbreakable_space_node::did_space_merge(hunits, hunits, hunits)
{
  return false;
}

hvpair::hvpair()
{
}

draw_node::draw_node(char c, hvpair *p, int np, font_size s,
		     color *gc, color *fc)
: npoints(np), sz(s), gcol(gc), fcol(fc), code(c)
{
  point = new hvpair[npoints];
  for (int i = 0; i < npoints; i++)
    point[i] = p[i];
}

draw_node::draw_node(char c, hvpair *p, int np, font_size s,
		     color *gc, color *fc, statem *st, int divlevel)
: node(0 /* nullptr */, st, divlevel), npoints(np), sz(s), gcol(gc),
  fcol(fc), code(c)
{
  point = new hvpair[npoints];
  for (int i = 0; i < npoints; i++)
    point[i] = p[i];
}

void draw_node::dump_properties()
{
  node::dump_properties();
  fprintf(stderr, ", \"code\": \"%c\"", code);
  fprintf(stderr, ", \"npoints\": %d", npoints);
  fprintf(stderr, ", \"font_size\": %d", sz.to_units());
  fputs(", \"stroke color\": ", stderr);
  gcol->nm.json_dump();
  fputs(", \"fill color\": ", stderr);
  fcol->nm.json_dump();
  fprintf(stderr, ", \"point\": \"(%d, %d)\"",
	  point->h.to_units(), point->v.to_units());
  fflush(stderr);
}

bool draw_node::is_same_as(node *n)
{
  draw_node *nd = static_cast<draw_node *>(n);
  if (code != nd->code || npoints != nd->npoints || sz != nd->sz
      || gcol != nd->gcol || fcol != nd->fcol)
    return false;
  for (int i = 0; i < npoints; i++)
    if (point[i].h != nd->point[i].h || point[i].v != nd->point[i].v)
      return false;
  return true;
}

const char *draw_node::type()
{
  return "drawing command node";
}

bool draw_node::causes_tprint()
{
  return false;
}

bool draw_node::is_tag()
{
  return false;
}

draw_node::~draw_node()
{
  if (point)
    delete[] point;
}

hunits draw_node::width()
{
  hunits x = H0;
  for (int i = 0; i < npoints; i++)
    x += point[i].h;
  return x;
}

vunits draw_node::vertical_width()
{
  if (code == 'e')
    return V0;
  vunits x = V0;
  for (int i = 0; i < npoints; i++)
    x += point[i].v;
  return x;
}

node *draw_node::copy()
{
  return new draw_node(code, point, npoints, sz, gcol, fcol, state,
		       div_nest_level);
}

void draw_node::tprint(troff_output_file *out)
{
  out->draw(code, point, npoints, sz, gcol, fcol);
}

/* tprint methods */

void glyph_node::tprint(troff_output_file *out)
{
  tfont *ptf = tf->get_plain();
  if (ptf == tf)
    out->put_char_width(ci, ptf, gcol, fcol, width(), H0);
  else {
    hunits offset;
    bool is_emboldened = tf->is_emboldened(&offset);
    hunits w = ptf->get_width(ci);
    hunits k = H0;
    hunits x;
    bool is_constantly_spaced = tf->is_constantly_spaced(&x);
    if (is_constantly_spaced) {
      x -= w;
      if (is_emboldened)
	x -= offset;
      hunits x2 = (x / 2);
      out->right(x2);
      k = x - x2;
    }
    else
      k = tf->get_track_kern();
    if (is_emboldened) {
      out->put_char(ci, ptf, gcol, fcol);
      out->right(offset);
    }
    out->put_char_width(ci, ptf, gcol, fcol, w, k);
  }
}

void glyph_node::zero_width_tprint(troff_output_file *out)
{
  tfont *ptf = tf->get_plain();
  hunits offset;
  bool is_emboldened = tf->is_emboldened(&offset);
  hunits x;
  bool is_constantly_spaced = tf->is_constantly_spaced(&x);
  if (is_constantly_spaced) {
    x -= ptf->get_width(ci);
    if (is_emboldened)
      x -= offset;
    x = (x / 2);
    out->right(x);
  }
  out->put_char(ci, ptf, gcol, fcol);
  if (is_emboldened) {
    out->right(offset);
    out->put_char(ci, ptf, gcol, fcol);
    out->right(-offset);
  }
  if (is_constantly_spaced)
    out->right(-x);
}

void break_char_node::tprint(troff_output_file *t)
{
  nodes->tprint(t);
}

void break_char_node::zero_width_tprint(troff_output_file *t)
{
  nodes->zero_width_tprint(t);
}

void hline_node::tprint(troff_output_file *out)
{
  if (x < H0) {
    out->right(x);
    x = -x;
  }
  if (0 /* nullptr */ == nodes) {
    out->right(x);
    return;
  }
  hunits w = nodes->width();
  if (w <= H0) {
    error("horizontal line drawing character must have positive width");
    out->right(x);
    return;
  }
  int i = int(x / w);
  if (0 == i) {
    hunits xx = x - w;
    hunits xx2 = (xx / 2);
    out->right(xx2);
    if (out->is_on())
      nodes->tprint(out);
    out->right(xx - xx2);
  }
  else {
    hunits rem = x - (w * i);
    if (rem > H0) {
      if (nodes->overlaps_horizontally()) {
	if (out->is_on())
	  nodes->tprint(out);
	out->right(rem - w);
      }
      else
	out->right(rem);
    }
    while (--i >= 0)
      if (out->is_on())
	nodes->tprint(out);
  }
}

void vline_node::tprint(troff_output_file *out)
{
  if (0 /* nullptr */ == nodes) {
    out->down(x);
    return;
  }
  vunits h = nodes->size();
  bool overlaps = nodes->overlaps_vertically();
  vunits y = x;
  if (y < V0) {
    y = -y;
    int i = y / h;
    vunits rem = y - i*h;
    if (0 == i) {
      out->right(nodes->width());
      out->down(-rem);
    }
    else {
      while (--i > 0) {
	nodes->zero_width_tprint(out);
	out->down(-h);
      }
      if (overlaps) {
	nodes->zero_width_tprint(out);
	out->down(-rem);
	if (out->is_on())
	  nodes->tprint(out);
	out->down(-h);
      }
      else {
	if (out->is_on())
	  nodes->tprint(out);
	out->down(-h - rem);
      }
    }
  }
  else {
    int i = y / h;
    vunits rem = y - i*h;
    if (0 == i) {
      out->down(rem);
      out->right(nodes->width());
    }
    else {
      out->down(h);
      if (overlaps)
	nodes->zero_width_tprint(out);
      out->down(rem);
      while (--i > 0) {
	nodes->zero_width_tprint(out);
	out->down(h);
      }
      if (out->is_on())
	nodes->tprint(out);
    }
  }
}

void zero_width_node::tprint(troff_output_file *out)
{
  if (0 /* nullptr */ == nodes)
    return;
  if (0 /* nullptr */ == nodes->next) {
    nodes->zero_width_tprint(out);
    return;
  }
  int hpos = out->get_hpos();
  int vpos = out->get_vpos();
  node *tem = nodes;
  while (tem) {
    tem->tprint(out);
    tem = tem->next;
  }
  out->moveto(hpos, vpos);
}

void overstrike_node::tprint(troff_output_file *out)
{
  hunits pos = H0;
  for (node *tem = nodes; tem != 0 /* nullptr */; tem = tem->next) {
    hunits x = (max_width - tem->width()) / 2;
    out->right(x - pos);
    pos = x;
    tem->zero_width_tprint(out);
  }
  out->right(max_width - pos);
}

void bracket_node::tprint(troff_output_file *out)
{
  if (0 /* nullptr */ == nodes)
    return;
  int npieces = 0;
  node *tem;
  for (tem = nodes; tem != 0 /* nullptr */; tem = tem->next)
    ++npieces;
  vunits h = nodes->size();
  vunits totalh = h*npieces;
  vunits y = (totalh - h) / 2;
  out->down(y);
  for (tem = nodes; tem != 0 /* nullptr */; tem = tem->next) {
    tem->zero_width_tprint(out);
    out->down(-h);
  }
  out->right(max_width);
  out->down(totalh - y);
}

void node::tprint(troff_output_file *)
{
}

void node::zero_width_tprint(troff_output_file *out)
{
  int hpos = out->get_hpos();
  int vpos = out->get_vpos();
  tprint(out);
  out->moveto(hpos, vpos);
}

void space_node::tprint(troff_output_file *out)
{
  out->fill_color(col);
  out->right(n);
}

void hmotion_node::tprint(troff_output_file *out)
{
  out->fill_color(col);
  out->right(n);
}

void space_char_hmotion_node::tprint(troff_output_file *out)
{
  out->fill_color(col);
  if (is_writing_html) {
    // we emit the space width as a negative glyph index
    out->flush_tbuf();
    out->do_motion();
    out->put('N');
    out->put(-n.to_units());
    out->put('\n');
  }
  out->right(n);
}

void vmotion_node::tprint(troff_output_file *out)
{
  out->fill_color(col);
  out->down(n);
}

void kern_pair_node::tprint(troff_output_file *out)
{
  n1->tprint(out);
  out->right(amount);
  n2->tprint(out);
}

static void tprint_reverse_node_list(troff_output_file *out, node *n)
{
  if (0 /* nullptr */ == n)
    return;
  tprint_reverse_node_list(out, n->next);
  n->tprint(out);
}

void dbreak_node::tprint(troff_output_file *out)
{
  tprint_reverse_node_list(out, none);
}

void composite_node::tprint(troff_output_file *out)
{
  hunits bold_offset;
  bool is_emboldened = tf->is_emboldened(&bold_offset);
  hunits track_kern = tf->get_track_kern();
  hunits constant_space;
  bool is_constantly_spaced = tf->is_constantly_spaced(&constant_space);
  hunits x = H0;
  if (is_constantly_spaced) {
    x = constant_space;
    for (node *tem = nodes; tem != 0 /* nullptr */; tem = tem->next)
      x -= tem->width();
    if (is_emboldened)
      x -= bold_offset;
    hunits x2 = x / 2;
    out->right(x2);
    x -= x2;
  }
  if (is_emboldened) {
    int hpos = out->get_hpos();
    int vpos = out->get_vpos();
    tprint_reverse_node_list(out, nodes);
    out->moveto(hpos, vpos);
    out->right(bold_offset);
  }
  tprint_reverse_node_list(out, nodes);
  if (is_constantly_spaced)
    out->right(x);
  else
    out->right(track_kern);
}

void composite_node::dump_properties()
{
  node::dump_properties();
  // GNU troff multiplexes the distinction of ordinary vs. special
  // characters though the special character code zero.
  unsigned char c = ci->get_ascii_code();
  if (c != 0U) {
    fputs(", \"character\": ", stderr);
    // It's not a `string` or `symbol` we can `.json_dump()`, so we have
    // to write the quotation marks ourselves.
    fputc('\"', stderr);
    json_char jc = json_encode_char(c);
    // Write out its JSON representation by character by character to
    // keep libc string functions from interpreting C escape sequences.
    for (size_t i = 0; i < jc.len; i++)
      fputc(jc.buf[i], stderr);
    fputc('\"', stderr);
  }
  else {
    fputs(", \"special character\": ", stderr);
    ci->nm.json_dump();
  }
  fflush(stderr);
}

void composite_node::dump_node()
{
  fputc('{', stderr);
  dump_properties();
  fputs(", \"contents\": ", stderr);
  dump_node_list_in_reverse(nodes);
  fputc('}', stderr);
  fflush(stderr);
}

static node *make_composite_node(charinfo *s, environment *env)
{
  int fontno = env_resolve_font(env);
  if (fontno < 0) {
    error("cannot format composite glyph: no current font");
    return 0 /* nullptr */;
  }
  assert((fontno < font_table_size)
	 && font_table[fontno] != 0 /* nullptr*/);
  node *n = charinfo_to_node_list(s, env);
  font_size fs = env->get_font_size();
  int char_height = env->get_char_height();
  int char_slant = env->get_char_slant();
  tfont *tf = font_table[fontno]->get_tfont(fs, char_height, char_slant,
					    fontno);
  if (env->is_composite())
    tf = tf->get_plain();
  return new composite_node(n, s, tf, 0, 0, 0);
}

static node *make_glyph_node(charinfo *s, environment *env,
			     bool want_warnings = true)
{
  int fontno = env_resolve_font(env);
  if (fontno < 0) {
    error("cannot format glyph: no current font");
    return 0 /* nullptr */;
  }
  assert((fontno < font_table_size)
	 && font_table[fontno] != 0 /* nullptr*/);
  int fn = fontno;
  bool found = font_table[fontno]->contains(s);
  if (!found) {
    macro *mac = s->get_macro();
    if ((mac != 0 /* nullptr */) && s->is_fallback())
      return make_composite_node(s, env);
    if (s->is_numbered()) {
      if (want_warnings)
	warning(WARN_CHAR, "character code %1 not defined in current"
		" font", s->get_number());
      return 0 /* nullptr */;
    }
    special_font_list *sf = font_table[fontno]->sf;
    while ((sf != 0 /* nullptr */) && !found) {
      fn = sf->n;
      if (font_table[fn])
	found = font_table[fn]->contains(s);
      sf = sf->next;
    }
    if (!found) {
      symbol f = font_table[fontno]->get_name();
      string gl(f.contents());
      gl += ' ';
      gl += s->nm.contents();
      gl += '\0';
      charinfo *ci = lookup_charinfo(symbol(gl.contents()));
      if (ci && ci->get_macro())
	return make_composite_node(ci, env);
    }
    if (!found) {
      sf = global_special_fonts;
      while ((sf != 0 /* nullptr */) && !found) {
	fn = sf->n;
	if (font_table[fn])
	  found = font_table[fn]->contains(s);
	sf = sf->next;
      }
    }
    if (!found)
      if (mac && s->is_special())
	return make_composite_node(s, env);
    if (!found) {
      for (fn = 0; fn < font_table_size; fn++)
	if (font_table[fn]
	    && font_table[fn]->is_special()
	    && font_table[fn]->contains(s)) {
	  found = true;
	  break;
	}
    }
    if (!found) {
      if (want_warnings && s->first_time_not_found()) {
	unsigned char input_code = s->get_ascii_code();
	if (input_code != 0U) {
	  if (csgraph(input_code))
	    warning(WARN_CHAR, "character '%1' not defined",
		    input_code);
	  else
	    warning(WARN_CHAR, "character with input code %1 not"
		    " defined", int(input_code));
	}
	else if (s->nm.contents()) {
	  const char *nm = s->nm.contents();
	  // If the contents are empty, get_char_for_escape_parameter()
	  // should already have thrown an error.
	  if (nm[0] != '\0') {
	    const char *backslash = (nm[1] == '\0') ? "\\" : "";
	    warning(WARN_CHAR, "special character '%1%2' not defined",
		    backslash, nm);
	  }
	}
      }
      return 0 /* nullptr */;
    }
  }
  font_size fs = env->get_font_size();
  int char_height = env->get_char_height();
  int char_slant = env->get_char_slant();
  tfont *tf = font_table[fontno]->get_tfont(fs, char_height, char_slant,
					    fn);
  if (env->is_composite())
    tf = tf->get_plain();
  color *gcol = env->get_stroke_color();
  color *fcol = env->get_fill_color();
  return new glyph_node(s, tf, gcol, fcol, 0 /* nullptr */,
			0 /* nullptr */);
}

node *make_node(charinfo *ci, environment *env)
{
  switch (ci->get_special_translation()) {
  case charinfo::TRANSLATE_SPACE:
    return new space_char_hmotion_node(env->get_space_width(),
				       env->get_fill_color());
  case charinfo::TRANSLATE_STRETCHABLE_SPACE:
    return new unbreakable_space_node(env->get_space_width(),
				      env->get_fill_color());
  case charinfo::TRANSLATE_DUMMY:
    return new dummy_node;
  case charinfo::TRANSLATE_HYPHEN_INDICATOR:
    error("translation to \\%% ignored in this context");
    break;
  }
  charinfo *tem = ci->get_translation();
  if (tem != 0 /* nullptr */)
    ci = tem;
  macro *mac = ci->get_macro();
  if (mac && ci->is_normal())
    return make_composite_node(ci, env);
  else
    return make_glyph_node(ci, env);
}

bool character_exists(charinfo *ci, environment *env)
{
  if (ci->get_special_translation() != charinfo::TRANSLATE_NONE)
    return true;
  charinfo *tem = ci->get_translation();
  if (tem != 0 /* nullptr */)
    ci = tem;
  if (ci->get_macro())
    return true;
  node *nd = make_glyph_node(ci, env, false /* don't want warnings */);
  if (nd) {
    delete nd;
    return true;
  }
  return false;
}

node *node::add_char(charinfo *ci, environment *env,
		     hunits *widthp, int *spacep, node **glyph_comp_np)
{
  node *res;
  switch (ci->get_special_translation()) {
  case charinfo::TRANSLATE_SPACE:
    res = new space_char_hmotion_node(env->get_space_width(),
				      env->get_fill_color(), this);
    *widthp += res->width();
    return res;
  case charinfo::TRANSLATE_STRETCHABLE_SPACE:
    res = new unbreakable_space_node(env->get_space_width(),
				     env->get_fill_color(), this);
    res->freeze_space();
    *widthp += res->width();
    *spacep += res->nspaces();
    return res;
  case charinfo::TRANSLATE_DUMMY:
    return new dummy_node(this);
  case charinfo::TRANSLATE_HYPHEN_INDICATOR:
    return add_discretionary_hyphen();
  }
  charinfo *tem = ci->get_translation();
  if (tem != 0 /* nullptr */)
    ci = tem;
  macro *mac = ci->get_macro();
  if (mac && ci->is_normal()) {
    res = make_composite_node(ci, env);
    if (res) {
      res->next = this;
      *widthp += res->width();
      if (glyph_comp_np)
	*glyph_comp_np = res;
    }
    else {
      if (glyph_comp_np)
	*glyph_comp_np = res;
      return this;
    }
  }
  else {
    node *gn = make_glyph_node(ci, env);
    if (0 /* nullptr */ == gn)
      return this;
    else {
      hunits old_width = width();
      node *p = gn->merge_self(this);
      if (0 /* nullptr */ == p) {
	*widthp += gn->width();
	gn->next = this;
	res = gn;
      }
      else {
	*widthp += p->width() - old_width;
	res = p;
      }
      if (glyph_comp_np)
	*glyph_comp_np = res;
    }
  }
  int break_code = 0;
  if (ci->allows_break_before())
    break_code = ALLOWS_BREAK_BEFORE;
  if (ci->allows_break_after())
    break_code |= ALLOWS_BREAK_AFTER;
  if (ci->ignores_surrounding_hyphenation_codes())
    break_code |= IGNORES_SURROUNDING_HYPHENATION_CODES;
  if (ci->prohibits_break_before())
    break_code = PROHIBITS_BREAK_BEFORE;
  if (ci->prohibits_break_after())
    break_code |= PROHIBITS_BREAK_AFTER;
  if (ci->is_interword_space())
    break_code |= IS_INTERWORD_SPACE;
  if (break_code != 0) {
    node *next1 = res->next;
    res->next = 0 /* nullptr */;
    res = new break_char_node(res, break_code, get_break_code(),
			      env->get_fill_color(), next1);
  }
  return res;
}

static inline int same_node(node *n1, node *n2)
{
  if (n1 != 0 /* nullptr */) {
    if (n2 != 0 /* nullptr */)
      return n1->type() == n2->type() && n1->is_same_as(n2);
    else
      return false;
  }
  else
    return 0 /* nullptr */ == n2;
}

int same_node_list(node *n1, node *n2)
{
  while ((n1 != 0 /* nullptr */) && (n2 != 0 /* nullptr */)) {
    if (n1->type() != n2->type() || !n1->is_same_as(n2))
      return 0;
    n1 = n1->next;
    n2 = n2->next;
  }
  return !n1 && !n2;
}

bool extra_size_node::is_same_as(node *nd)
{
  return (n == static_cast<extra_size_node *>(nd)->n);
}

const char *extra_size_node::type()
{
  return "extra vertical spacing node";
}

bool extra_size_node::causes_tprint()
{
  return false;
}

bool extra_size_node::is_tag()
{
  return false;
}

bool vertical_size_node::is_same_as(node *nd)
{
  return (n == static_cast<vertical_size_node *>(nd)->n);
}

const char *vertical_size_node::type()
{
  return "vertical spacing node";
}

bool vertical_size_node::set_unformat_flag()
{
  return false;
}

bool vertical_size_node::causes_tprint()
{
  return false;
}

bool vertical_size_node::is_tag()
{
  return false;
}

bool hmotion_node::is_same_as(node *nd)
{
  return ((n == static_cast<hmotion_node *>(nd)->n)
	  && (col == static_cast<hmotion_node *>(nd)->col));
}

const char *hmotion_node::type()
{
  return "horizontal motion node";
}

bool hmotion_node::set_unformat_flag()
{
  unformat = true;
  return true;
}

bool hmotion_node::causes_tprint()
{
  return false;
}

bool hmotion_node::is_tag()
{
  return false;
}

node *hmotion_node::add_self(node *nd, hyphen_list **p)
{
  next = nd;
  hyphen_list *pp = *p;
  *p = (*p)->next;
  delete pp;
  return this;
}

hyphen_list *hmotion_node::get_hyphen_list(hyphen_list *tail, int *)
{
  return new hyphen_list(0, tail);
}

bool space_char_hmotion_node::is_same_as(node *nd)
{
  return ((n == static_cast<space_char_hmotion_node *>(nd)->n
	  && col == static_cast<space_char_hmotion_node *>(nd)->col));
}

const char *space_char_hmotion_node::type()
{
  return "space character horizontal motion node";
}

bool space_char_hmotion_node::causes_tprint()
{
  return false;
}

bool space_char_hmotion_node::is_tag()
{
  return false;
}

node *space_char_hmotion_node::add_self(node *nd, hyphen_list **p)
{
  next = nd;
  hyphen_list *pp = *p;
  *p = (*p)->next;
  delete pp;
  return this;
}

hyphen_list *space_char_hmotion_node::get_hyphen_list(hyphen_list *tail,
						      int *)
{
  return new hyphen_list(0, tail);
}

bool vmotion_node::is_same_as(node *nd)
{
  return ((n == static_cast<vmotion_node *>(nd)->n)
	  && col == static_cast<vmotion_node *>(nd)->col);
}

const char *vmotion_node::type()
{
  return "vertical motion node";
}

bool vmotion_node::causes_tprint()
{
  return false;
}

bool vmotion_node::is_tag()
{
  return false;
}

bool hline_node::is_same_as(node *nd)
{
  return ((x == static_cast<hline_node *>(nd)->x)
	  && same_node(nodes, static_cast<hline_node *>(nd)->nodes));
}

const char *hline_node::type()
{
  return "horizontal rule node";
}

bool hline_node::causes_tprint()
{
  return false;
}

bool hline_node::is_tag()
{
  return false;
}

bool vline_node::is_same_as(node *nd)
{
  return ((x == static_cast<vline_node *>(nd)->x)
	  && same_node(nodes, static_cast<vline_node *>(nd)->nodes));
}

const char *vline_node::type()
{
  return "vertical rule node";
}

bool vline_node::causes_tprint()
{
  return false;
}

bool vline_node::is_tag()
{
  return false;
}

bool dummy_node::is_same_as(node *)
{
  return true;
}

const char *dummy_node::type()
{
  return "dummy node";
}

bool dummy_node::causes_tprint()
{
  return false;
}

bool dummy_node::is_tag()
{
  return false;
}

bool transparent_dummy_node::is_same_as(node *)
{
  return true;
}

const char *transparent_dummy_node::type()
{
  return "transparent dummy node";
}

bool transparent_dummy_node::causes_tprint()
{
  return false;
}

bool transparent_dummy_node::is_tag()
{
  return false;
}

int transparent_dummy_node::ends_sentence()
{
  return 2;
}

bool zero_width_node::is_same_as(node *nd)
{
  return same_node_list(nodes, ((zero_width_node *)nd)->nodes);
}

const char *zero_width_node::type()
{
  return "zero-width output node";
}

bool zero_width_node::causes_tprint()
{
  return false;
}

bool zero_width_node::is_tag()
{
  return false;
}

bool italic_corrected_node::is_same_as(node *nd)
{
  return ((x == static_cast<italic_corrected_node *>(nd)->x)
	  && same_node(nodes,
	       static_cast<italic_corrected_node *>(nd)->nodes));
}

const char *italic_corrected_node::type()
{
  return "italic-corrected node";
}

bool italic_corrected_node::causes_tprint()
{
  return false;
}

bool italic_corrected_node::is_tag()
{
  return false;
}

left_italic_corrected_node::left_italic_corrected_node(node *xx)
: container_node(xx)
{
}

left_italic_corrected_node::left_italic_corrected_node(statem *s,
						       int divlevel,
						       node *xx)
: container_node(xx, s, divlevel)
{
}

void left_italic_corrected_node::dump_properties()
{
  node::dump_properties();
  fprintf(stderr, ", \"hunits\": %d", x.to_units());
  fflush(stderr);
}

node *left_italic_corrected_node::merge_glyph_node(glyph_node *gn)
{
  if (0 /* nullptr */ == nodes) {
    hunits lic = gn->left_italic_correction();
    if (!lic.is_zero()) {
      x = lic;
      nodes = gn;
      return this;
    }
  }
  else {
    node *nd = nodes->merge_glyph_node(gn);
    if (nd != 0 /* nullptr */) {
      nodes = nd;
      x = nodes->left_italic_correction();
      return this;
    }
  }
  return 0 /* nullptr */;
}

node *left_italic_corrected_node::copy()
{
  left_italic_corrected_node *nd =
    new left_italic_corrected_node(state, div_nest_level);
  if (nodes != 0 /* nullptr */) {
    nd->nodes = nodes->copy();
    nd->x = x;
  }
  return nd;
}

void left_italic_corrected_node::tprint(troff_output_file *out)
{
  if (nodes != 0 /* nullptr */) {
    out->right(x);
    nodes->tprint(out);
  }
}

const char *left_italic_corrected_node::type()
{
  return "left italic-corrected node";
}

bool left_italic_corrected_node::causes_tprint()
{
  return false;
}

bool left_italic_corrected_node::is_tag()
{
  return false;
}

bool left_italic_corrected_node::is_same_as(node *nd)
{
  return ((x == static_cast<left_italic_corrected_node *>(nd)->x)
	  && same_node(nodes,
	       static_cast<left_italic_corrected_node *>(nd)->nodes));
}

void left_italic_corrected_node::ascii_print(ascii_output_file *out)
{
  if (nodes != 0 /* nullptr */)
    nodes->ascii_print(out);
}

hunits left_italic_corrected_node::width()
{
  return (nodes != 0 /* nullptr */) ? (nodes->width() + x) : H0;
}

void left_italic_corrected_node::vertical_extent(vunits *minimum,
						 vunits *maximum)
{
  if (nodes != 0 /* nullptr */)
    nodes->vertical_extent(minimum, maximum);
  else
    node::vertical_extent(minimum, maximum);
}

hunits left_italic_corrected_node::skew()
{
  return (nodes != 0 /* nullptr */) ? (nodes->skew() + x / 2) : H0;
}

hunits left_italic_corrected_node::subscript_correction()
{
  return (nodes != 0 /* nullptr */) ? nodes->subscript_correction()
				    : H0;
}

hunits left_italic_corrected_node::italic_correction()
{
  return (nodes != 0 /* nullptr */) ? nodes->italic_correction() : H0;
}

int left_italic_corrected_node::ends_sentence()
{
  return (nodes != 0 /* nullptr */) ? nodes->ends_sentence() : 0;
}

bool left_italic_corrected_node::overlaps_horizontally()
{
  return (nodes != 0 /* nullptr */) ? nodes->overlaps_horizontally()
				    : false;
}

bool left_italic_corrected_node::overlaps_vertically()
{
  return (nodes != 0 /* nullptr */) ? nodes->overlaps_vertically()
				    : false;
}

node *left_italic_corrected_node::last_char_node()
{
  return (nodes != 0 /* nullptr */) ? nodes->last_char_node()
				    : 0 /* nullptr */;
}

tfont *left_italic_corrected_node::get_tfont()
{
  return (nodes != 0 /* nullptr */) ? nodes->get_tfont()
				    : 0 /* nullptr */;
}

hyphenation_type left_italic_corrected_node::get_hyphenation_type()
{
  if (nodes != 0 /* nullptr */)
    return nodes->get_hyphenation_type();
  else
    return HYPHENATION_PERMITTED;
}

hyphen_list *left_italic_corrected_node::get_hyphen_list(
    hyphen_list *tail, int *count)
{
  return (nodes != 0 /* nullptr */)
          ? nodes->get_hyphen_list(tail, count) : tail;
}

node *left_italic_corrected_node::add_self(node *nd, hyphen_list **p)
{
  if (nodes != 0 /* nullptr */) {
    nd = new left_italic_corrected_node(state, div_nest_level, nd);
    nd = nodes->add_self(nd, p);
    nodes = 0 /* nullptr */;
    delete this;
    return nd;
  }
  else {
    next = nd;
    return this;
  }
}

int left_italic_corrected_node::character_type()
{
  return (nodes != 0 /* nullptr */) ? nodes->character_type() : 0;
}

bool overstrike_node::is_same_as(node *nd)
{
  return same_node_list(nodes, ((overstrike_node *)nd)->nodes);
}

const char *overstrike_node::type()
{
  return "overstricken node";
}

bool overstrike_node::causes_tprint()
{
  return false;
}

bool overstrike_node::is_tag()
{
  return false;
}

node *overstrike_node::add_self(node *more_nodes, hyphen_list **p)
{
  next = more_nodes;
  hyphen_list *pp = *p;
  *p = (*p)->next;
  delete pp;
  return this;
}

hyphen_list *overstrike_node::get_hyphen_list(hyphen_list *tail, int *)
{
  return new hyphen_list(0U, tail);
}

bool bracket_node::is_same_as(node *nd)
{
  return same_node_list(nodes, ((bracket_node *)nd)->nodes);
}

const char *bracket_node::type()
{
  return "bracket-building node";
}

bool bracket_node::causes_tprint()
{
  return false;
}

bool bracket_node::is_tag()
{
  return false;
}

bool composite_node::is_same_as(node *nd)
{
  return ((ci == static_cast<composite_node *>(nd)->ci)
	  && same_node(nodes,
	               static_cast<composite_node *>(nd)->nodes));
}

const char *composite_node::type()
{
  return "composite node"; // XXX: composite WHAT? (character macro?)
}

bool composite_node::causes_tprint()
{
  return false;
}

bool composite_node::is_tag()
{
  return false;
}

bool glyph_node::is_same_as(node *nd)
{
  return ((ci == static_cast<glyph_node *>(nd)->ci)
	  && (tf == static_cast<glyph_node *>(nd)->tf)
	  && (gcol == static_cast<glyph_node *>(nd)->gcol)
	  && (fcol == static_cast<glyph_node *>(nd)->fcol));
}

const char *glyph_node::type()
{
  return "glyph node";
}

bool glyph_node::causes_tprint()
{
  return false;
}

bool glyph_node::is_tag()
{
  return false;
}

bool ligature_node::is_same_as(node *nd)
{
  return (same_node(n1, ((ligature_node *)nd)->n1)
	  && same_node(n2, ((ligature_node *)nd)->n2)
	  && glyph_node::is_same_as(nd));
}

const char *ligature_node::type()
{
  return "ligature node";
}

bool ligature_node::causes_tprint()
{
  return false;
}

bool ligature_node::is_tag()
{
  return false;
}

bool kern_pair_node::is_same_as(node *nd)
{
  return ((amount == static_cast<kern_pair_node *>(nd)->amount)
	  && same_node(n1, static_cast<kern_pair_node *>(nd)->n1)
	  && same_node(n2, static_cast<kern_pair_node *>(nd)->n2));
}

const char *kern_pair_node::type()
{
  return "kerned character pair node";
}

bool kern_pair_node::causes_tprint()
{
  return false;
}

bool kern_pair_node::is_tag()
{
  return false;
}

bool dbreak_node::is_same_as(node *nd)
{
  return (same_node_list(none, ((dbreak_node *)nd)->none)
	  && same_node_list(pre, ((dbreak_node *)nd)->pre)
	  && same_node_list(post, ((dbreak_node *)nd)->post));
}

const char *dbreak_node::type()
{
  return "discretionary breakpoint node";
}

bool dbreak_node::causes_tprint()
{
  return false;
}

bool dbreak_node::is_tag()
{
  return false;
}

void dbreak_node::dump_node()
{
  fputc('{', stderr);
  // Flush so that in case something goes wrong with property dumping,
  // we know that we traversed to a new node.
  fflush(stderr);
  node::dump_properties();
  if (none != 0 /* nullptr */) {
    fputs(", \"none\": ", stderr);
    none->dump_node();
  }
  if (pre != 0 /* nullptr */) {
    fputs(", \"pre\": ", stderr);
    pre->dump_node();
  }
  if (post != 0 /* nullptr */) {
    fputs(", \"post\": ", stderr);
    post->dump_node();
  }
  fputc('}', stderr);
  fflush(stderr);
}

bool break_char_node::is_same_as(node *nd)
{
  return (break_code == static_cast<break_char_node *>(nd)->break_code)
	 && (col == static_cast<break_char_node *>(nd)->col)
	 && (same_node(nodes,
	               static_cast<break_char_node *>(nd)->nodes));
}

const char *break_char_node::type()
{
  return "breakpoint node";
}

bool break_char_node::causes_tprint()
{
  return false;
}

bool break_char_node::is_tag()
{
  return false;
}

unsigned char break_char_node::get_break_code()
{
  return break_code;
}

bool line_start_node::is_same_as(node *)
{
  return true;
}

const char *line_start_node::type()
{
  return "output line start node";
}

bool line_start_node::causes_tprint()
{
  return false;
}

bool line_start_node::is_tag()
{
  return false;
}

bool space_node::is_same_as(node *nd)
{
  return ((n == static_cast<space_node *>(nd)->n)
	  && (set == static_cast<space_node *>(nd)->set)
	  && (col == static_cast<space_node *>(nd)->col));
}

const char *space_node::type()
{
  return "space node";
}

bool word_space_node::is_same_as(node *nd)
{
  return ((n == static_cast<word_space_node *>(nd)->n)
	  && (set == static_cast<word_space_node *>(nd)->set)
	  && (col == static_cast<word_space_node *>(nd)->col));
}

const char *word_space_node::type()
{
  return "word space node";
}

bool word_space_node::causes_tprint()
{
  return false;
}

bool word_space_node::is_tag()
{
  return false;
}

void unbreakable_space_node::tprint(troff_output_file *out)
{
  out->fill_color(col);
  out->word_marker();
  if (is_writing_html) {
    // we emit the space width as a negative glyph index
    out->flush_tbuf();
    out->do_motion();
    out->put('N');
    out->put(-n.to_units());
    out->put('\n');
  }
  out->right(n);
}

bool unbreakable_space_node::is_same_as(node *nd)
{
  return n == ((unbreakable_space_node *)nd)->n
	 && set == ((unbreakable_space_node *)nd)->set
	 && col == ((unbreakable_space_node *)nd)->col;
}

const char *unbreakable_space_node::type()
{
  return "unbreakable space node";
}

node *unbreakable_space_node::add_self(node *nd, hyphen_list **p)
{
  next = nd;
  hyphen_list *pp = *p;
  *p = (*p)->next;
  delete pp;
  return this;
}

hyphen_list *unbreakable_space_node::get_hyphen_list(hyphen_list *tail,
						     int *)
{
  return new hyphen_list(0, tail);
}

bool diverted_space_node::is_same_as(node *nd)
{
  return n == ((diverted_space_node *)nd)->n;
}

const char *diverted_space_node::type()
{
  return "diverted vertical space node";
}

bool diverted_space_node::causes_tprint()
{
  return false;
}

bool diverted_space_node::is_tag()
{
  return false;
}

bool diverted_copy_file_node::is_same_as(node *nd)
{
  return filename == ((diverted_copy_file_node *)nd)->filename;
}

const char *diverted_copy_file_node::type()
{
  return "diverted file throughput node";
}

bool diverted_copy_file_node::causes_tprint()
{
  return false;
}

bool diverted_copy_file_node::is_tag()
{
  return false;
}

// Grow the font_table so that its size is > n.

static void grow_font_table(int n)
{
  assert(n >= font_table_size);
  font_info **old_font_table = font_table;
  int old_font_table_size = font_table_size;
  font_table_size = font_table_size ? ((font_table_size * 3) / 2) : 10;
  if (font_table_size <= n)
    font_table_size = n + 10;
  font_table = new font_info *[font_table_size];
  if (old_font_table_size)
    memcpy(font_table, old_font_table,
	   old_font_table_size*sizeof(font_info *));
  delete[] old_font_table;
  for (int i = old_font_table_size; i < font_table_size; i++)
    font_table[i] = 0 /* nullptr */;
}

dictionary font_translation_dictionary(17);

static symbol get_font_translation(symbol nm)
{
  void *p = font_translation_dictionary.lookup(nm);
  return p ? symbol(static_cast<char *>(p)) : nm;
}

dictionary font_dictionary(50);
// We store the address of this font in `font_dictionary` to indicate
// that we've previously tried to mount the font and failed.
font nonexistent_font = font("\0");

// Mount font at position `n` with troff identifier `name` and
// description file name `filename` (these are often identical).  If
// `check_only`, just look up `name` in the existing list of mounted
// fonts.
static bool mount_font_no_translate(int n, symbol name, symbol filename,
				    bool check_only = false)
{
  assert(n >= 0);
  font *fm = 0 /* nullptr */;
  void *p = font_dictionary.lookup(filename);
  if (0 /* nullptr */ == p) {
    fm = font::load_font(filename.contents(), check_only);
    if (check_only)
      return fm != 0 /* nullptr */;
    if (0 /* nullptr */ == fm) {
      (void) font_dictionary.lookup(filename, &nonexistent_font);
      return false;
    }
    (void) font_dictionary.lookup(name, fm);
  }
  else if (&nonexistent_font == p)
    return false;
  else
    fm = static_cast<font *>(p);
  if (check_only)
    return true;
  if (n >= font_table_size) {
    if ((n - font_table_size) > 1000) {
      error("requested font mounting position %1 too much larger than"
	    " first unused position %2", n,
	    next_available_font_position());
      return false;
    }
    grow_font_table(n);
  }
  else if (font_table[n] != 0 /* nullptr */)
    delete font_table[n];
  font_table[n] = new font_info(name, n, filename, fm);
  font_family::invalidate_fontno(n);
  return true;
}

void print_font_mounting_position_request()
{
  for (int i = 0; i < font_table_size; i++) {
    font_info *fi = font_table[i];
    if (0 /* nullptr */ == fi) // No font mounted here.
      continue;
    errprint("%1\t%2", i, fi->get_name().contents());
    font *f = font_table[i]->get_font();
    if (f != 0 /* nullptr */) { // It's not an abstract style.
      errprint("\t%1", f->get_filename());
      if (f->get_internal_name() != 0 /* nullptr */)
	errprint("\t%1", f->get_internal_name());
    }
    errprint("\n");
    fflush(stderr);
  }
  skip_line();
}

bool mount_font(int n, symbol name, symbol external_name)
{
  assert(n >= 0);
  name = get_font_translation(name);
  if (external_name.is_null())
    external_name = name;
  else
    external_name = get_font_translation(external_name);
  return mount_font_no_translate(n, name, external_name);
}

// True for abstract styles and resolved font names.
bool is_font_name(symbol fam, symbol name)
{
  if (is_abstract_style(name))
    name = concat(fam, name);
  return mount_font_no_translate(0, name, name, true /* check only */);
}

bool is_abstract_style(symbol s)
{
  int i = symbol_fontno(s);
  return i < 0 ? 0 : font_table[i]->is_style();
}

bool mount_style(int n, symbol name)
{
  assert(n >= 0);
  if (n >= font_table_size) {
    if ((n - font_table_size) > 1000) {
      error("font position too much larger than first unused position");
      return false;
    }
    grow_font_table(n);
  }
  else if (font_table[n] != 0 /* nullptr */)
    delete font_table[n];
  font_table[n] = new font_info(get_font_translation(name), n,
				NULL_SYMBOL, 0);
  font_family::invalidate_fontno(n);
  return true;
}

// True for valid (not necessarily used) font mounting positions.
static bool is_nonnegative_integer(const char *str)
{
  return strspn(str, "0123456789") == strlen(str);
}

static void translate_font()
{
  if (!has_arg()) {
    warning(WARN_MISSING, "font translation request expects one or two"
	    " font name arguments");
    skip_line();
    return;
  }
  symbol from = read_identifier(true /* required */);
  // has_arg()+read_identifier() should ensure the assertion succeeds.
  assert(!from.is_null());
  if (is_nonnegative_integer(from.contents())) {
    error("cannot translate a font mounting position");
    skip_line();
    return;
  }
  symbol to = read_identifier();
  if ((!to.is_null()) && is_nonnegative_integer(to.contents())) {
    error("cannot translate to a font mounting position");
    skip_line();
    return;
  }
  if (to.is_null() || from == to)
    font_translation_dictionary.remove(from);
  else
    (void) font_translation_dictionary.lookup(from,
					      (void *)to.contents());
  skip_line();
}

static void print_font_translation_request()
{
  dictionary_iterator iter(font_translation_dictionary);
  symbol from, to;
  // We must use the nuclear `reinterpret_cast` operator because GNU
  // troff's dictionary types use a pre-STL approach to containers.
  while (iter.get(&from, reinterpret_cast<void **>(&to)))
    errprint("%1\t%2\n", from.contents(), to.contents());
  fflush(stderr);
  skip_line();
  return;
}

static void mount_font_at_position()
{
  if (!has_arg()) {
    warning(WARN_MISSING, "font mounting request expects arguments");
    skip_line();
    return;
  }
  int n;
  if (read_integer(&n)) {
    if (n < 0)
      error("font mounting position %1 is negative", n);
    else {
      symbol internal_name = read_identifier(true /* required */);
      if (!internal_name.is_null()) {
	symbol filename = read_long_identifier();
	if (!mount_font(n, internal_name, filename)) {
	  string msg;
	  if (filename != 0 /* nullptr */)
	    msg += string(" from file '") + filename.contents()
	      + string("'");
	  msg += '\0';
	  error("cannot load font description '%1'%2 for mounting",
		internal_name.contents(), msg.contents());
	}
      }
    }
  }
  skip_line();
}

font_family::font_family(symbol s)
: map_size(10), nm(s)
{
  map = new int[map_size];
  for (int i = 0; i < map_size; i++)
    map[i] = FONT_NOT_MOUNTED;
}

font_family::~font_family()
{
  delete[] map;
}

// Resolve a requested font mounting position to a mounting position
// usable by the output driver.  (Positions 1 through 4 are typically
// allocated to styles, and are not usable thus.)  A return value of
// `FONT_NOT_MOUNTED` indicates failure.
int font_family::resolve(int mounting_position)
{
  assert(mounting_position >= 0);
  int pos = mounting_position;
  assert((pos >= 0) || (FONT_NOT_MOUNTED == pos));
  if (pos < 0)
    return FONT_NOT_MOUNTED;
  if (pos < map_size && map[pos] >= 0)
    return map[pos];
  if (!((pos < font_table_size)
	&& (font_table[pos] != 0 /* nullptr */)))
    return FONT_NOT_MOUNTED;
  if (pos >= map_size) {
    int old_map_size = map_size;
    int *old_map = map;
    map_size *= 3;
    map_size /= 2;
    if (pos >= map_size)
      map_size = pos + 10;
    map = new int[map_size];
    memcpy(map, old_map, old_map_size * sizeof (int));
    delete[] old_map;
    for (int j = old_map_size; j < map_size; j++)
      map[j] = FONT_NOT_MOUNTED;
  }
  if (!(font_table[pos]->is_style()))
    return map[pos] = pos;
  symbol sty = font_table[pos]->get_name();
  symbol f = concat(nm, sty);
  int n;
  // Don't use symbol_fontno, because that might return a style and
  // because we don't want to translate the name.
  for (n = 0; n < font_table_size; n++)
    if ((font_table[n] != 0 /* nullptr */) && font_table[n]->is_named(f)
	&& !font_table[n]->is_style())
      break;
  if (n >= font_table_size) {
    n = next_available_font_position();
    if (!mount_font_no_translate(n, f, f))
      return FONT_NOT_MOUNTED;
  }
  return map[pos] = n;
}

dictionary family_dictionary(5);

font_family *lookup_family(symbol nm)
{
  font_family *f
    = static_cast<font_family *>(family_dictionary.lookup(nm));
  if (0 /* nullptr */ == f) {
    f = new font_family(nm);
    (void) family_dictionary.lookup(nm, f);
  }
  return f;
}

void font_family::invalidate_fontno(int n)
{
  assert(n >= 0 && n < font_table_size);
  dictionary_iterator iter(family_dictionary);
  symbol nam;
  font_family *fam;
  while (iter.get(&nam, (void **)&fam)) {
    int mapsize = fam->map_size;
    if (n < mapsize)
      fam->map[n] = FONT_NOT_MOUNTED;
    for (int i = 0; i < mapsize; i++)
      if (fam->map[i] == n)
	fam->map[i] = FONT_NOT_MOUNTED;
  }
}

static void associate_style_with_font_position()
{
  if (!has_arg()) {
    warning(WARN_MISSING, "abstract style configuration request expects"
	    " arguments");
    skip_line();
    return;
  }
  int n;
  if (read_integer(&n)) {
    if (n < 0)
      error("font mounting position %1 is negative", n);
    else {
      if (!has_arg())
	warning(WARN_MISSING, "abstract style configuration request"
		" expects a style name as second argument");
      else {
	symbol internal_name = read_identifier(true /* required */);
	if (!internal_name.is_null())
	  (void) mount_style(n, internal_name);
      }
    }
  }
  skip_line();
}

static void font_lookup_error(font_lookup_info& finfo,
			      const char *msg)
{
  if (finfo.requested_name)
    error("cannot select font '%1' %2", finfo.requested_name, msg);
  else if (finfo.position == FONT_NOT_MOUNTED)
    error("cannot select font %1", msg); // don't report position `-1`
  else
    error("cannot select font at position %1 %2",
	  finfo.requested_position, msg);
}

bool is_valid_font_mounting_position(int n)
{
  return ((n >= 0)
	  && (n < font_table_size)
	  && (font_table[n] != 0 /* nullptr */));
}

// Read the next token and look it up as a font name or position number.
// Return lookup success.  Store, in the supplied struct argument, the
// requested name or position, and the position actually resolved.
static bool read_font_identifier(font_lookup_info *finfo)
{
  int n;
  tok.skip_spaces();
  if (tok.is_usable_as_delimiter()) {
    symbol s = read_identifier(true /* required */);
    finfo->requested_name = const_cast<char *>(s.contents());
    if (!s.is_null()) {
      n = symbol_fontno(s);
      if (n < 0) {
	n = next_available_font_position();
	if (mount_font(n, s))
	  finfo->position = n;
      }
      finfo->position = curenv->get_family()->resolve(n);
    }
  }
  else if (read_integer(&n)) {
    finfo->requested_position = n;
    if (is_valid_font_mounting_position(n))
      finfo->position = curenv->get_family()->resolve(n);
  }
  return (finfo->position != FONT_NOT_MOUNTED);
}

static int underline_fontno = 2;

static void select_underline_font()
{
  if (!has_arg()) {
    warning(WARN_MISSING, "underline font selection request expects an"
	    " argument");
    skip_line();
    return;
  }
  font_lookup_info finfo;
  if (!read_font_identifier(&finfo))
    font_lookup_error(finfo, "to make it the underline font");
  else
    underline_fontno = finfo.position;
  skip_line();
}

int get_underline_fontno()
{
  return underline_fontno;
}

static void define_font_specific_character()
{
  if (!has_arg()) {
    warning(WARN_MISSING, "font-specific fallback character definition"
	    " request expects arguments");
    skip_line();
    return;
  }
  font_lookup_info finfo;
  if (!read_font_identifier(&finfo)) {
    font_lookup_error(finfo, "to define font-specific fallback"
		      " character");
    // Normally we skip the remainder of the line unconditionally at the
    // end of a request-implementing function, but define_character()
    // will eat the rest of it for us.
    skip_line();
  }
  else {
    symbol f = font_table[finfo.position]->get_name();
    define_character(CHAR_FONT_SPECIFIC_FALLBACK, f.contents());
  }
}

static void remove_font_specific_character()
{
  if (!has_arg()) {
    warning(WARN_MISSING, "font-specific fallback character removal"
	    " request expects arguments");
    skip_line();
    return;
  }
  font_lookup_info finfo;
  if (!read_font_identifier(&finfo))
    font_lookup_error(finfo, "to remove font-specific fallback"
		      " character");
  else {
    symbol f = font_table[finfo.position]->get_name();
    while (!tok.is_newline() && !tok.is_eof()) {
      if (!tok.is_space() && !tok.is_tab()) {
	charinfo *s = tok.get_charinfo(true /* required */);
	if (0 /* nullptr */ == s)
	  assert(0 == "attempted to use token without charinfo in"
		 " font-specific character removal request");
	else {
	  string gl(f.contents());
	  gl += ' ';
	  gl += s->nm.contents();
	  gl += '\0';
	  charinfo *ci = lookup_charinfo(symbol(gl.contents()));
	  // Expect a null pointer if the font-specific character was
	  // already removed or never existed.
	  if (ci != 0 /* nullptr */) {
	    macro *m = ci->set_macro(0 /* nullptr */);
	    if (m != 0 /* nullptr */)
	      delete m;
	  }
	}
      }
      tok.next();
    }
  }
  skip_line();
}

static void read_special_fonts(special_font_list **sp)
{
  special_font_list *s = *sp;
  *sp = 0 /* nullptr */;
  while (s != 0 /* nullptr */) {
    special_font_list *tem = s;
    s = s->next;
    delete tem;
  }
  special_font_list **p = sp;
  while (has_arg()) {
    font_lookup_info finfo;
    if (!read_font_identifier(&finfo))
      font_lookup_error(finfo, "to mark it as special");
    else {
      special_font_list *tem = new special_font_list;
      tem->n = finfo.position;
      tem->next = 0 /* nullptr */;
      *p = tem;
      p = &(tem->next);
    }
  }
}

static void set_font_specific_special_fonts()
{
  if (!has_arg()) {
    warning(WARN_MISSING, "font-specific special font selection request"
	    " expects at least one argument");
    skip_line();
    return;
  }
  font_lookup_info finfo;
  if (!read_font_identifier(&finfo))
    font_lookup_error(finfo, "to mark other fonts as special"
			     " contingently upon it"); // a mouthful :-/
  else
    read_special_fonts(&font_table[finfo.position]->sf);
  skip_line();
}

static void set_special_fonts()
{
  read_special_fonts(&global_special_fonts);
  skip_line();
}

static void zoom_font()
{
  if (!(has_arg())) {
    warning(WARN_MISSING, "font zoom factor request expects arguments");
    skip_line();
    return;
  }
  symbol font_name = read_identifier();
  // has_arg()+read_identifier() should ensure the assertion succeeds.
  assert(font_name != 0 /* nullptr */);
  if (is_nonnegative_integer(font_name.contents())) {
    warning(WARN_FONT, "cannot set zoom factor of a font mounting"
	    " position");
    skip_line();
    return;
  }
  if (!(has_arg())) {
    warning(WARN_MISSING, "font zoom factor request expects zoom factor"
	    " argument");
    skip_line();
    return;
  }
  int fpos = next_available_font_position();
  if (!(mount_font(fpos, font_name))) {
    error("cannot mount font '%1' to set a zoom factor for it",
	  font_name.contents());
    skip_line();
    return;
  }
#if 0
  // This would be a good diagnostic to have, but mount_font() is too
  // formally complex to make it easy.  Instead it will fail in the
  // above test on a font named "R", for instance, when that is
  // literally true but might not help users who don't understand that
  // "R", "I", "B", and "BI" are (by default) abstract styles, not fonts
  // in the GNU troff sense.  It is a shame that a lot of our validation
  // functions are willing only to handle arguments that they eat from
  // the input stream (i.e., you can't pass them information you
  // obtained elsewhere).  That design also forces us to validate
  // request arguments in the order they appear in the input, and seems
  // unnecessarily inflexible to me.  --GBR
  if (font_table[fpos]->is_style()) {
    warning(WARN_FONT, "ignoring request to set font zoom factor on an"
	    " abstract style");
    skip_line();
    return;
  }
#endif
  int zoom = 0;
  read_integer(&zoom);
  if (zoom < 0) {
    warning(WARN_RANGE, "ignoring negative font zoom factor '%1'",
	    zoom);
    skip_line();
    return;
  }
  font_table[fpos]->set_zoom(zoom);
  skip_line();
}

int next_available_font_position()
{
  int i;
  for (i = 1;
       (i < font_table_size) && (font_table[i] != 0 /* nullptr */);
       i++)
    ;
  return i;
}

int symbol_fontno(symbol s)
{
  s = get_font_translation(s);
  for (int i = 0; i < font_table_size; i++)
    if ((font_table[i] != 0 /* nullptr */)
	&& font_table[i]->is_named(s))
      return i;
  return FONT_NOT_MOUNTED;
}

hunits env_font_emboldening_offset(environment *env, int n)
{
  if (is_valid_font_mounting_position(n)) {
    hunits offset;
    int fontno = env->get_family()->resolve(env->get_font());
    if (font_table[fontno]->is_emboldened(&offset))
      return offset.to_units() + 1;
    else
      return H0;
  }
  else
    return H0;
}

hunits env_digit_width(environment *env)
{
  node *n = make_glyph_node(charset_table['0'], env);
  if (n != 0 /* nullptr */) {
    hunits x = n->width();
    delete n;
    return x;
  }
  else
    return H0;
}

hunits env_space_width(environment *env)
{
  int fn = env_resolve_font(env);
  font_size fs = env->get_font_size();
  if (!is_valid_font_mounting_position(fn))
    return scale(fs.to_units() / 3, env->get_space_size(), 12);
  else
    return font_table[fn]->get_space_width(fs, env->get_space_size());
}

hunits env_sentence_space_width(environment *env)
{
  int fn = env_resolve_font(env);
  font_size fs = env->get_font_size();
  units sss = env->get_sentence_space_size();
  if (!is_valid_font_mounting_position(fn))
    return scale(fs.to_units() / 3, sss, 12);
  else
    return font_table[fn]->get_space_width(fs, sss);
}

hunits env_half_narrow_space_width(environment *env)
{
  int fn = env_resolve_font(env);
  font_size fs = env->get_font_size();
  if (!is_valid_font_mounting_position(fn))
    return H0;
  else
    return font_table[fn]->get_half_narrow_space_width(fs);
}

hunits env_narrow_space_width(environment *env)
{
  int fn = env_resolve_font(env);
  font_size fs = env->get_font_size();
  if (!is_valid_font_mounting_position(fn))
    return H0;
  else
    return font_table[fn]->get_narrow_space_width(fs);
}

// XXX: We can only conditionally (un)embolden a font specified by name,
// not position.  Does ".bd 1 2" mean "embolden font position 1 by 2
// units" (really one unit), or "stop conditionally emboldening font 2
// when font 1 is selected"?
static void embolden_font()
{
  if (!(has_arg())) {
    warning(WARN_MISSING, "emboldening request expects arguments");
    skip_line();
    return;
  }
  if (in_nroff_mode) {
    skip_line();
    return;
  }
  // XXX: Here's where `is_ordinary_character()` would be useful.
  // (TOKEN_CHAR == type) means the same thing, but this file doesn't
  // root around in token::token_type...
  if ((!tok.is_any_character())
      || tok.is_special_character()
      || tok.is_indexed_character()) {
    error("emboldening request expects font name or mounting position"
	  " as first argument, got %1", tok.description());
    skip_line();
    return;
  }
  // If the 1st argument starts with a non-numeral, we must be prepared
  // to expect a third argument to specify an emboldening amount...
  // .bd S TB 3 \" embolden S when `TB` selected
  // ...or removal of conditional emboldening.
  // .bd S TB \" don't embolden S when `TB` selected
  //
  // XXX: Our approach forecloses the emboldening of fonts whose names
  // _start_ with numerals but _contain_ non-numerals, like '3foo'.  If
  // one agrees that Kernighan's grammar for the extended `bd` request
  // of device-independent troff is ambiguous, there may not be a
  // perfect solution.  (This approach could be improved by scanning
  // ahead in the input, but is it worth the trouble?)
  bool emboldening_may_be_conditional = false;
  if (!csdigit(tok.ch()))
    emboldening_may_be_conditional = true;
  font_lookup_info finfo;
  if (!read_font_identifier(&finfo)) {
    font_lookup_error(finfo, "for emboldening");
    skip_line();
    return;
  }
  int n = finfo.position;
  if (has_arg()) {
    // XXX: Here's another useful `is_ordinary_character()` spot.
    if ((!tok.is_any_character())
	|| tok.is_special_character()
	|| tok.is_indexed_character()) {
      error("emboldening request expects font name or emboldening"
	    " amount as second argument, got %1", tok.description());
      skip_line();
      return;
    }
    // If both arguments start with numerals, assume this form.
    // .bd 2 4 \" embolden font position 2 by 3 (yes, 3) units
    // ...otherwise...
    if (emboldening_may_be_conditional && !(csdigit(tok.ch()))) {
      font_lookup_info finfo2;
      if (!read_font_identifier(&finfo2)) {
	font_lookup_error(finfo2, "for conditional emboldening");
	skip_line();
	return;
      }
      int f = finfo2.position;
      units offset;
      if (has_arg()
	  && read_measurement(&offset, 'u')
	  && (offset >= 1))
	font_table[f]->set_conditional_bold(n, hunits(offset - 1));
      else
	font_table[f]->conditional_unbold(n);
    }
    else {
      // The second argument must be an emboldening amount.
      units offset;
      if (read_measurement(&offset, 'u') && (offset >= 1))
	font_table[n]->set_bold(hunits(offset - 1));
      else
	font_table[n]->unbold();
      if (has_arg())
	warning(WARN_FONT, "ignoring third argument to font emboldening"
		" request when first interpreted as mounting position"
		" and second as emboldening amount");
    }
  }
  else
    font_table[n]->unbold();
  skip_line();
}

track_kerning_function::track_kerning_function() : non_zero(0)
{
}

track_kerning_function::track_kerning_function(int min_s, hunits min_a,
					       int max_s, hunits max_a)
: non_zero(1), min_size(min_s), min_amount(min_a), max_size(max_s),
  max_amount(max_a)
{
}

int track_kerning_function::operator==(const track_kerning_function &tk)
{
  if (non_zero)
    return (tk.non_zero
	    && min_size == tk.min_size
	    && min_amount == tk.min_amount
	    && max_size == tk.max_size
	    && max_amount == tk.max_amount);
  else
    return !tk.non_zero;
}

int track_kerning_function::operator!=(const track_kerning_function &tk)
{
  if (non_zero)
    return (!tk.non_zero
	    || min_size != tk.min_size
	    || min_amount != tk.min_amount
	    || max_size != tk.max_size
	    || max_amount != tk.max_amount);
  else
    return tk.non_zero;
}

hunits track_kerning_function::compute(int size)
{
  if (non_zero) {
    if (max_size <= min_size)
      return min_amount;
    else if (size <= min_size)
      return min_amount;
    else if (size >= max_size)
      return max_amount;
    else
      return (scale(max_amount, size - min_size, max_size - min_size)
	      + scale(min_amount, max_size - size, max_size - min_size));
  }
  else
    return H0;
}

static void configure_track_kerning()
{
  if (!has_arg()) {
    warning(WARN_MISSING, "track kerning request expects arguments");
    skip_line();
    return;
  }
  font_lookup_info finfo;
  if (!read_font_identifier(&finfo))
    font_lookup_error(finfo, "for track kerning");
  else {
    int n = finfo.position, min_s, max_s;
    hunits min_a, max_a;
    if (has_arg()
	&& read_measurement(&min_s, 'z')
	&& read_hunits(&min_a, 'p')
	&& read_measurement(&max_s, 'z')
	&& read_hunits(&max_a, 'p')) {
      track_kerning_function tk(min_s, min_a, max_s, max_a);
      font_table[n]->set_track_kern(tk);
    }
    else {
      track_kerning_function tk;
      font_table[n]->set_track_kern(tk);
    }
  }
  skip_line();
}

static void constantly_space_font()
{
  if (!has_arg()) {
    warning(WARN_MISSING, "constant spacing request expects arguments");
    skip_line();
    return;
  }
  font_lookup_info finfo;
  if (!read_font_identifier(&finfo))
    font_lookup_error(finfo, "for constant spacing");
  else {
    int n = finfo.position, x, y;
    if (!has_arg() || !read_integer(&x))
      font_table[n]->set_constant_space(CONSTANT_SPACE_NONE);
    else {
      if (!has_arg() || !read_measurement(&y, 'z'))
	font_table[n]->set_constant_space(CONSTANT_SPACE_RELATIVE, x);
      else
	font_table[n]->set_constant_space(CONSTANT_SPACE_ABSOLUTE,
					  scale(y*x,
						units_per_inch,
						36 * 72 * sizescale));
    }
  }
  skip_line();
}

static void set_ligature_mode()
{
  int lig;
  if (has_arg() && read_integer(&lig) && lig >= 0 && lig <= 2)
    global_ligature_mode = lig;
  else
    global_ligature_mode = 1;
  skip_line();
}

static void set_kerning_mode()
{
  int k;
  if (has_arg() && read_integer(&k))
    global_kern_mode = (k > 0);
  else
    global_kern_mode = true;
  skip_line();
}

// Set (soft) hyphenation character, used to mark where a discretionary
// break ("dbreak") has occurred in formatted output, conventionally
// within a word at a syllable boundary.
//
// XXX: The soft hyphen character is global; shouldn't it be
// environmental?
static void soft_hyphen_character_request()
{
  soft_hyphen_char = read_character();
  if (0 /* nullptr */ == soft_hyphen_char)
    soft_hyphen_char = lookup_charinfo(HYPHEN_SYMBOL);
  skip_line();
}

void init_output()
{
  if (want_output_suppressed)
    the_output = new suppress_output_file;
  else if (want_abstract_output)
    the_output = new ascii_output_file;
  else
    the_output = new troff_output_file;
}

class next_available_font_position_reg : public reg {
public:
  const char *get_string();
};

const char *next_available_font_position_reg::get_string()
{
  return i_to_a(next_available_font_position());
}

class printing_reg : public reg {
public:
  const char *get_string();
};

const char *printing_reg::get_string()
{
  if (the_output)
    return the_output->is_selected_for_printing() ? "1" : "0";
  else
    return "0";
}

void init_node_requests()
{
  init_request("bd", embolden_font);
  init_request("cs", constantly_space_font);
  init_request("fp", mount_font_at_position);
  init_request("fschar", define_font_specific_character);
  init_request("fspecial", set_font_specific_special_fonts);
  init_request("fzoom", zoom_font);
  init_request("ftr", translate_font);
  init_request("kern", set_kerning_mode);
  init_request("lg", set_ligature_mode);
  init_request("pfp", print_font_mounting_position_request);
  init_request("pftr", print_font_translation_request);
  init_request("rfschar", remove_font_specific_character);
  init_request("shc", soft_hyphen_character_request);
  init_request("special", set_special_fonts);
  init_request("sty", associate_style_with_font_position);
  init_request("tkf", configure_track_kerning);
  init_request("uf", select_underline_font);
  register_dictionary.define(".fp",
			     new next_available_font_position_reg);
  register_dictionary.define(".kern",
      new readonly_boolean_register(&global_kern_mode));
  register_dictionary.define(".lg",
      new readonly_register(&global_ligature_mode));
  register_dictionary.define(".P", new printing_reg);
  soft_hyphen_char = lookup_charinfo(HYPHEN_SYMBOL);
}

// Local Variables:
// fill-column: 72
// mode: C++
// End:
// vim: set cindent noexpandtab shiftwidth=2 textwidth=72:

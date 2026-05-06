/* Copyright 1989-2025 Free Software Foundation, Inc.
             2020-2025 G. Branden Robinson

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

#include <errno.h> // errno
#include <math.h> // ceil(), fabs()

#include <vector>
#include <algorithm> // find()

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
#include "macropath.h"
#include "input.h"

symbol default_family("T");

// C++11: Use `enum : char`.
// TODO: Can we move this into a (possibly struct/class-implicit)
// namespace?
enum { ADJUST_LEFT = 0,
  ADJUST_BOTH = 1,
  ADJUST_CENTER = 3,
  ADJUST_RIGHT = 5,
  ADJUST_MAX = 5
};

// C++11: Use `enum : unsigned char`.
// TODO: Can we move this into a (possibly struct/class-implicit)
// namespace?
enum {
  // Not all combinations are valid; see hyphenate_request() below.
  HYPHEN_NONE = 0,
  HYPHEN_DEFAULT = 1,
  HYPHEN_NOT_LAST_LINE = 2,
  HYPHEN_NOT_LAST_CHARS = 4,
  HYPHEN_NOT_FIRST_CHARS = 8,
  HYPHEN_LAST_CHAR = 16,
  HYPHEN_FIRST_CHAR = 32,
  HYPHEN_MAX = 63
};

struct env_list_node {
  environment *env;
  env_list_node *next;
  env_list_node(environment *e, env_list_node *p) : env(e), next(p) {}
};

env_list_node *env_stack;
dictionary env_dictionary(10);
environment *curenv;
static int next_line_number = 0;
extern statem *get_diversion_state();

charinfo *field_delimiter_char;
charinfo *padding_indicator_char;

bool translate_space_to_dummy = false;

// forward declaration
static void hyphenate(hyphen_list *h, unsigned int flags);

class pending_output_line {
  node *nd;
  bool suppress_filling;
  bool was_centered;
  vunits vs;
  vunits post_vs;
  hunits width;
#ifdef WIDOW_CONTROL
  bool is_last_line;		// Is it the last line of the paragraph?
#endif /* WIDOW_CONTROL */
public:
  pending_output_line *next;

  pending_output_line(node *, bool, vunits, vunits, hunits, bool,
		      pending_output_line * = 0 /* nullptr */);
  ~pending_output_line();
  bool output();

#ifdef WIDOW_CONTROL
  friend void environment::mark_last_line();
  friend void environment::output(node *, bool, vunits, vunits, hunits,
				  bool);
#endif /* WIDOW_CONTROL */
};

pending_output_line::pending_output_line(node *nod, bool nf, vunits v,
					 vunits pv, hunits w, bool ce,
					 pending_output_line *p)
: nd(nod), suppress_filling(nf), was_centered(ce), vs(v), post_vs(pv),
  width(w),
#ifdef WIDOW_CONTROL
  is_last_line(false),
#endif /* WIDOW_CONTROL */
  next(p)
{
}

pending_output_line::~pending_output_line()
{
  delete_node_list(nd);
}

bool pending_output_line::output()
{
  if (was_trap_sprung)
    return false;
#ifdef WIDOW_CONTROL
  if (next && next->is_last_line && !suppress_filling) {
    curdiv->need(vs + post_vs + vunits(vresolution));
    if (was_trap_sprung) {
      next->is_last_line = false;	// Try to avoid infinite loops.
      return false;
    }
  }
#endif
  curenv->construct_format_state(nd, was_centered, !suppress_filling);
  curdiv->output(nd, suppress_filling, vs, post_vs, width);
  nd = 0 /* nullptr */;
  return true;
}

void environment::output(node *nd, bool suppress_filling,
			 vunits vs, vunits post_vs,
			 hunits width, bool was_centered)
{
#ifdef WIDOW_CONTROL
  while (pending_lines != 0 /* nullptr */) {
    if (want_widow_control && !pending_lines->suppress_filling
	&& !pending_lines->next)
      break;
    if (!pending_lines->output())
      break;
    pending_output_line *tem = pending_lines;
    pending_lines = pending_lines->next;
    delete tem;
  }
#else /* WIDOW_CONTROL */
  output_pending_lines();
#endif /* WIDOW_CONTROL */
  if (!was_trap_sprung && !pending_lines
#ifdef WIDOW_CONTROL
      && (!want_widow_control || suppress_filling)
#endif /* WIDOW_CONTROL */
      ) {
    curenv->construct_format_state(nd, was_centered, !suppress_filling);
    curdiv->output(nd, suppress_filling, vs, post_vs, width);
  }
  else {
    pending_output_line **p;
    for (p = &pending_lines; *p; p = &(*p)->next)
      ;
    *p = new pending_output_line(nd, suppress_filling, vs, post_vs,
				 width, was_centered);
  }
}

// a line from .tl goes at the head of the queue

void environment::output_title(node *nd, bool suppress_filling,
			       vunits vs, vunits post_vs,
			       hunits width)
{
  if (!was_trap_sprung)
    curdiv->output(nd, suppress_filling, vs, post_vs, width);
  else
    pending_lines = new pending_output_line(nd, suppress_filling, vs,
					    post_vs, width, 0,
					    pending_lines);
}

void environment::output_pending_lines()
{
  while ((pending_lines != 0 /* nullptr */) && pending_lines->output())
  {
    pending_output_line *tem = pending_lines;
    pending_lines = pending_lines->next;
    delete tem;
  }
}

#ifdef WIDOW_CONTROL

void environment::mark_last_line()
{
  if (!want_widow_control || !pending_lines)
    return;
  pending_output_line *p;
  for (p = pending_lines; p->next; p = p->next)
    ;
  if (!p->suppress_filling)
    p->is_last_line = true;
}

void widow_control_request()
{
  int n;
  if (has_arg() && read_integer(&n))
    curenv->want_widow_control = (n > 0);
  else
    curenv->want_widow_control = true;
  skip_line();
}

#endif /* WIDOW_CONTROL */

/* font_size functions */

size_range *font_size::size_list = 0 /* nullptr */;
int font_size::nranges = 0;

extern "C" {

int compare_ranges(const void *p1, const void *p2)
{
  return (  static_cast<size_range *>(const_cast<void *>(p1))->min
	  - static_cast<size_range *>(const_cast<void *>(p2))->min);
}

}

void font_size::init_size_list(int *sizes)
{
  nranges = 0;
  while (sizes[nranges * 2] != 0)
    nranges++;
  assert(nranges > 0);
  size_list = new size_range[nranges];
  for (int i = 0; i < nranges; i++) {
    size_list[i].min = sizes[i * 2];
    size_list[i].max = sizes[i * 2 + 1];
  }
  qsort(size_list, nranges, sizeof(size_range), compare_ranges);
}

void font_size::dump_size_list()
{
  int lo, hi;
  errprint("  valid type size list for selected font: ");
  if (nranges == 0)
    errprint(" empty!");
  else {
    bool need_comma = false;
    for (int i = 0; i < nranges; i++) {
      lo = size_list[i].min;
      hi = size_list[i].max;
      if (need_comma)
	errprint(", ");
      if (lo == hi)
	errprint("%1s", lo);
      else
	errprint("%1s-%2s", lo, hi);
      need_comma = true;
    }
  }
  errprint("\n");
  fflush(stderr);
}

font_size::font_size(int sp)
{
  for (int i = 0; i < nranges; i++) {
    if (sp < size_list[i].min) {
      if (i > 0 && size_list[i].min - sp >= sp - size_list[i - 1].max)
	p = size_list[i - 1].max;
      else
	p = size_list[i].min;
      return;
    }
    if (sp <= size_list[i].max) {
      p = sp;
      return;
    }
  }
  p = size_list[nranges - 1].max;
}

int font_size::to_units()
{
  return scale(p, units_per_inch, sizescale*72);
}

// we can't do this in a static constructor because various dictionaries
// have to get initialized first
static symbol default_environment_name("0");

void init_environments()
{
  curenv = new environment(default_environment_name);
  (void) env_dictionary.lookup(default_environment_name, curenv);
}

// Set tab character, used to fill out the remainder of a tab stop where
// a tab (TAB, U+0009) occurs in the input.  If a null pointer,
// horizontal motion "fills" the tab stop.
void tab_character_request()
{
  curenv->tab_char = read_character();
  skip_line();
}

// Set leader character, used to fill out the remainder of a tab stop
// where a leader (SOH, U+0001) occurs in the input.  If a null pointer,
// horizontal motion "fills" the tab stop.  Used when the behavior of a
// null pointer tab character is also desired on the same output line
// (or more generally).
void leader_character_request()
{
  curenv->leader_char = read_character();
  skip_line();
}

void environment::add_char(charinfo *ci)
{
  assert(!was_line_interrupted);
  node *gc_np = 0 /* nullptr */;
  if (was_line_interrupted)
    ;
  // don't allow fields in dummy environments
  else if (ci == field_delimiter_char && !is_dummy_env) {
    if (has_current_field)
      wrap_up_field();
    else
      start_field();
  }
  else if (has_current_field && ci == padding_indicator_char)
    add_padding();
  else if (current_tab != TAB_NONE) {
    if (tab_contents == 0 /* nullptr */)
      tab_contents = new line_start_node;
    if (ci != hyphen_indicator_char) {
      int s;
      tab_contents = tab_contents->add_char(ci, this, &tab_width, &s,
					    &gc_np);
    }
    else
      tab_contents = tab_contents->add_discretionary_hyphen();
  }
  else {
    if (line == 0 /* nullptr */)
      start_line();
#if 0
    fprintf(stderr, "current line is\n");
    dump_node_list_in_reverse(line);
#endif
    if (ci != hyphen_indicator_char)
      line = line->add_char(ci, this, &width_total, &space_total, &gc_np);
    else
      line = line->add_discretionary_hyphen();
  }
#if 0
  fprintf(stderr, "now after we have added character the line is\n");
  dump_node_list_in_reverse(line);
#endif
  if ((!suppress_push) && gc_np) {
    if (gc_np && (gc_np->state == 0 /* nullptr */)) {
      gc_np->state = construct_state(false);
      gc_np->push_state = get_diversion_state();
    }
    else if (line && (line->state == 0 /* nullptr */)) {
      line->state = construct_state(false);
      line->push_state = get_diversion_state();
    }
  }
#if 0
  fprintf(stderr, "now we have possibly added the state the line is\n");
  dump_node_list_in_reverse(line);
#endif
}

node *environment::make_char_node(charinfo *ci)
{
  return make_node(ci, this);
}

void environment::add_node(node *nd)
{
  assert(nd != 0 /* nullptr */);
  if (nd == 0 /* nullptr */)
    return;
  if (!suppress_push) {
    if (nd->is_special && nd->state == 0 /* nullptr */)
      nd->state = construct_state(false);
    nd->push_state = get_diversion_state();
  }

  if ((current_tab != TAB_NONE) || has_current_field)
    nd->freeze_space();
  if (was_line_interrupted) {
    delete nd;
  }
  else if (current_tab != TAB_NONE) {
    nd->next = tab_contents;
    tab_contents = nd;
    tab_width += nd->width();
  }
  else {
    if (line == 0 /* nullptr */) {
      if (is_discarding && nd->discardable()) {
	// XXX possibly: input_line_start -= nd->width();
	delete nd;
	return;
      }
      start_line();
    }
    width_total += nd->width();
    space_total += nd->nspaces();
    nd->next = line;
    line = nd;
    construct_new_line_state(line);
  }
}

void environment::add_hyphen_indicator()
{
  if ((current_tab != TAB_NONE)
      || was_line_interrupted
      || has_current_field
      || (hyphen_indicator_char != 0 /* nullptr */))
    return;
  if (line == 0 /* nullptr */)
    start_line();
  line = line->add_discretionary_hyphen();
}

unsigned int environment::get_hyphenation_mode()
{
  return hyphenation_mode;
}

unsigned int environment::get_hyphenation_mode_default()
{
  return hyphenation_mode_default;
}

int environment::get_hyphen_line_max()
{
  return hyphen_line_max;
}

int environment::get_hyphen_line_count()
{
  return hyphen_line_count;
}

int environment::get_centered_line_count()
{
  return centered_line_count;
}

int environment::get_right_aligned_line_count()
{
  return right_aligned_line_count;
}

int environment::get_no_number_count()
{
  return no_number_count;
}

int environment::get_input_trap_line_count()
{
  return input_trap_count;
}

int environment::get_input_trap_respects_continuation()
{
  return continued_input_trap;
}

const char *environment::get_input_trap_macro()
{
  return input_trap.contents();
}

void environment::add_italic_correction()
{
  if (current_tab != TAB_NONE) {
    if (tab_contents != 0 /* nullptr */)
      tab_contents = tab_contents->add_italic_correction(&tab_width);
  }
  else if (line)
    line = line->add_italic_correction(&width_total);
}

void environment::space_newline()
{
  assert(!was_line_interrupted);
  assert((current_tab == TAB_NONE) && !has_current_field);
  if (was_line_interrupted)
    return;
  hunits x = H0;
  hunits sw = env_space_width(this);
  hunits ssw = env_sentence_space_width(this);
  if (!translate_space_to_dummy) {
    x = sw;
    if (node_list_ends_sentence(line) == 1)
      x += ssw;
  }
  width_list *w = new width_list(sw, ssw);
  if (node_list_ends_sentence(line) == 1)
    w->next = new width_list(sw, ssw);
  if (line != 0 /* nullptr */ && line->did_space_merge(x, sw, ssw)) {
    width_total += x;
    return;
  }
  add_node(new word_space_node(x, get_fill_color(), w));
  possibly_break_line(false /* must break here */, is_spreading);
  is_spreading = false;
}

void environment::space()
{
  space(env_space_width(this), env_sentence_space_width(this));
}

void environment::space(hunits space_width, hunits sentence_space_width)
{
  if (was_line_interrupted)
    return;
  if (has_current_field && padding_indicator_char == 0 /* nullptr */) {
    add_padding();
    return;
  }
  hunits x = translate_space_to_dummy ? H0 : space_width;
  node *p = (current_tab != TAB_NONE) ? tab_contents : line;
  hunits *tp = (current_tab != TAB_NONE) ? &tab_width : &width_total;
  if (p && p->nspaces() == 1 && p->width() == x
      && node_list_ends_sentence(p->next) == 1) {
    hunits xx = translate_space_to_dummy ? H0 : sentence_space_width;
    if (p->did_space_merge(xx, space_width, sentence_space_width)) {
      *tp += xx;
      return;
    }
  }
  if (p && p->did_space_merge(x, space_width, sentence_space_width)) {
    *tp += x;
    return;
  }
  add_node(new word_space_node(x,
			       get_fill_color(),
			       new width_list(space_width,
					      sentence_space_width)));
  possibly_break_line(false /* must break here */, is_spreading);
  is_spreading = false;
}

static node *configure_space_underlining(bool b)
{
  macro m;
  m.append_str("x u ");
  m.append(b ? '1' : '0');
  return new device_extension_node(m, 1);
}

// TODO: Kill this off in groff 1.24.0 release + 2 years.  See input.cpp.
std::vector<symbol> deprecated_font_identifiers;
extern bool is_device_ps_or_pdf; // See input.cpp.

static void warn_if_font_name_deprecated(symbol nm)
{
  const char *name = nm.contents();
  std::vector<symbol>::iterator it
    = find(deprecated_font_identifiers.begin(),
	   deprecated_font_identifiers.end(), name);
  if (it != deprecated_font_identifiers.end()) {
    warning(WARN_FONT, "font name '%1' is deprecated", name);
    // Warn only once for each name.
    deprecated_font_identifiers.erase(it);
  }
}

bool environment::set_font(symbol nm)
{
  if (was_line_interrupted)
    return true; // "no operation" is successful
  // TODO: Kill this off in groff 1.24.0 release + 2 years.
  if (is_device_ps_or_pdf)
    warn_if_font_name_deprecated(nm);
  if (nm == symbol("P") || nm.is_empty()) {
    if (family->resolve(prev_fontno) == FONT_NOT_MOUNTED)
      return false;
    int tem = fontno;
    fontno = prev_fontno;
    prev_fontno = tem;
  }
  else {
    prev_fontno = fontno;
    int n = symbol_fontno(nm);
    if (n < 0) {
      n = next_available_font_position();
      if (!mount_font(n, nm))
	return false;
    }
    if (family->resolve(n) == FONT_NOT_MOUNTED)
      return false;
    fontno = n;
  }
  if (underline_spaces && fontno != prev_fontno) {
    if (fontno == get_underline_fontno())
      add_node(configure_space_underlining(true));
    if (prev_fontno == get_underline_fontno())
      add_node(configure_space_underlining(false));
  }
  return true;
}

bool environment::set_font(int n)
{
  if (was_line_interrupted)
    return false;
  if (is_valid_font_mounting_position(n)) {
    prev_fontno = fontno;
    fontno = n;
  }
  else {
    warning(WARN_FONT, "no font mounted at position %1", n);
    return false;
  }
  return true;
}

void environment::set_family(symbol fam)
{
  if (was_line_interrupted)
    return;
  if (fam.is_null() || fam.is_empty()) {
    int previous_mounting_position = prev_family->resolve(fontno);
    assert(previous_mounting_position >= 0);
    if (previous_mounting_position == FONT_NOT_MOUNTED)
      return;
    font_family *tem = family;
    family = prev_family;
    prev_family = tem;
  }
  else {
    font_family *f = lookup_family(fam);
    // If the family isn't already in the dictionary, looking it up will
    // create an entry for it.  That doesn't mean that it will be
    // resolvable to a real font when combined with a style name.
    assert((f != 0 /* nullptr */) &&
	   (0 != "font family dictionary lookup"));
    if (0 /* nullptr */ == f)
      return;
    if (f->resolve(fontno) == FONT_NOT_MOUNTED) {
      error("no font family named '%1' exists", fam.contents());
      return;
    }
    prev_family = family;
    family = f;
  }
}

void environment::set_size(int n)
{
  if (was_line_interrupted)
    return;
  if (0 == n) {
    font_size temp = prev_size;
    prev_size = size;
    size = temp;
    int temp2 = prev_requested_size;
    prev_requested_size = requested_size;
    requested_size = temp2;
  }
  else {
    prev_size = size;
    size = font_size(n);
    prev_requested_size = requested_size;
    requested_size = n;
  }
}

void environment::set_char_height(int n)
{
  if (was_line_interrupted)
    return;
  if (n == requested_size || n <= 0)
    char_height = 0;
  else
    char_height = n;
}

void environment::set_char_slant(int n)
{
  if (was_line_interrupted)
    return;
  char_slant = n;
}

color *environment::get_prev_stroke_color()
{
  return prev_stroke_color;
}

color *environment::get_stroke_color()
{
  return stroke_color;
}

color *environment::get_prev_fill_color()
{
  return prev_fill_color;
}

color *environment::get_fill_color()
{
  return fill_color;
}

void environment::set_stroke_color(color *c)
{
  if (was_line_interrupted)
    return;
  curenv->prev_stroke_color = curenv->stroke_color;
  curenv->stroke_color = c;
}

void environment::set_fill_color(color *c)
{
  if (was_line_interrupted)
    return;
  curenv->prev_fill_color = curenv->fill_color;
  curenv->fill_color = c;
}

environment::environment(symbol nm)
: is_dummy_env(false),
  prev_line_length((units_per_inch*13)/2),
  line_length((units_per_inch*13)/2),
  prev_title_length((units_per_inch*13)/2),
  title_length((units_per_inch*13)/2),
  prev_size(sizescale*10),
  size(sizescale*10),
  requested_size(sizescale*10),
  prev_requested_size(sizescale*10),
  char_height(0),
  char_slant(0),
  space_size(12),
  sentence_space_size(12),
  adjust_mode(ADJUST_BOTH),
  is_filling(true),
  was_line_interrupted(false),
  was_previous_line_interrupted(0),
  centered_line_count(0),
  right_aligned_line_count(0),
  prev_vertical_spacing(points_to_units(12)),
  vertical_spacing(points_to_units(12)),
  prev_post_vertical_spacing(0),
  post_vertical_spacing(0),
  prev_line_spacing(1),
  line_spacing(1),
  prev_indent(0),
  indent(0),
  temporary_indent(0),
  have_temporary_indent(false),
  underlined_line_count(0),
  underline_spaces(false),
  input_trap_count(-1),
  continued_input_trap(false),
  line(0 /* nullptr */),
  prev_text_length(0),
  width_total(0),
  space_total(0),
  input_line_start(0),
  using_line_tabs(false),
  current_tab(TAB_NONE),
  leader_node(0 /* nullptr */),
  tab_char(0 /* nullptr */),
  leader_char(charset_table['.']),
  has_current_field(false),
  is_discarding(false),
  is_spreading(false),
  margin_character_flags(0U),
  margin_character_node(0 /* nullptr */),
  margin_character_distance(points_to_units(10)),
  numbering_nodes(0 /* nullptr */),
  number_text_separation(1),
  line_number_indent(0),
  line_number_multiple(1),
  no_number_count(0),
  hyphenation_mode(1),
  hyphenation_mode_default(1),
  hyphen_line_count(0),
  hyphen_line_max(-1),
  hyphenation_space(H0),
  hyphenation_margin(H0),
  composite(false),
  pending_lines(0 /* nullptr */),
#ifdef WIDOW_CONTROL
  want_widow_control(false),
#endif /* WIDOW_CONTROL */
  stroke_color(&default_color),
  prev_stroke_color(&default_color),
  fill_color(&default_color),
  prev_fill_color(&default_color),
  control_character((unsigned char)('.')),
  no_break_control_character((unsigned char)('\'')),
  seen_space(false),
  seen_eol(false),
  suppress_next_eol(false),
  seen_break(false),
  tabs((units_per_inch / 2), TAB_LEFT),
  name(nm),
  hyphen_indicator_char(0)
{
  prev_family = family = lookup_family(default_family);
  prev_fontno = fontno = 1;
  if (!is_valid_font_mounting_position(1))
    fatal("font mounted at position 1 is not valid");
  if (family->resolve(1) == FONT_NOT_MOUNTED)
    fatal("invalid default font family '%1'",
	  default_family.contents());
  prev_fontno = fontno;
}

environment::environment(const environment *e)
: is_dummy_env(true),
  prev_line_length(e->prev_line_length),
  line_length(e->line_length),
  prev_title_length(e->prev_title_length),
  title_length(e->title_length),
  prev_size(e->prev_size),
  size(e->size),
  requested_size(e->requested_size),
  prev_requested_size(e->prev_requested_size),
  char_height(e->char_height),
  char_slant(e->char_slant),
  prev_fontno(e->prev_fontno),
  fontno(e->fontno),
  prev_family(e->prev_family),
  family(e->family),
  space_size(e->space_size),
  sentence_space_size(e->sentence_space_size),
  adjust_mode(e->adjust_mode),
  is_filling(e->is_filling),
  was_line_interrupted(false),
  was_previous_line_interrupted(0),
  centered_line_count(0),
  right_aligned_line_count(0),
  prev_vertical_spacing(e->prev_vertical_spacing),
  vertical_spacing(e->vertical_spacing),
  prev_post_vertical_spacing(e->prev_post_vertical_spacing),
  post_vertical_spacing(e->post_vertical_spacing),
  prev_line_spacing(e->prev_line_spacing),
  line_spacing(e->line_spacing),
  prev_indent(e->prev_indent),
  indent(e->indent),
  temporary_indent(0),
  have_temporary_indent(false),
  underlined_line_count(0),
  underline_spaces(false),
  input_trap_count(-1),
  continued_input_trap(false),
  line(0 /* nullptr */),
  prev_text_length(e->prev_text_length),
  width_total(0),
  space_total(0),
  input_line_start(0),
  using_line_tabs(e->using_line_tabs),
  current_tab(TAB_NONE),
  leader_node(0 /* nullptr */),
  tab_char(e->tab_char),
  leader_char(e->leader_char),
  has_current_field(false),
  is_discarding(false),
  is_spreading(false),
  margin_character_flags(e->margin_character_flags),
  margin_character_node(e->margin_character_node),
  margin_character_distance(e->margin_character_distance),
  numbering_nodes(0 /* nullptr */),
  number_text_separation(e->number_text_separation),
  line_number_indent(e->line_number_indent),
  line_number_multiple(e->line_number_multiple),
  no_number_count(e->no_number_count),
  hyphenation_mode(e->hyphenation_mode),
  hyphenation_mode_default(e->hyphenation_mode_default),
  hyphen_line_count(0),
  hyphen_line_max(e->hyphen_line_max),
  hyphenation_space(e->hyphenation_space),
  hyphenation_margin(e->hyphenation_margin),
  composite(false),
  pending_lines(0 /* nullptr */),
#ifdef WIDOW_CONTROL
  want_widow_control(e->want_widow_control),
#endif /* WIDOW_CONTROL */
  stroke_color(e->stroke_color),
  prev_stroke_color(e->prev_stroke_color),
  fill_color(e->fill_color),
  prev_fill_color(e->prev_fill_color),
  control_character(e->control_character),
  no_break_control_character(e->no_break_control_character),
  seen_space(e->seen_space),
  seen_eol(e->seen_eol),
  suppress_next_eol(e->suppress_next_eol),
  seen_break(e->seen_break),
  tabs(e->tabs),
  name(e->name),	// so that, e.g., '.if "\n[.ev]"0"' works
  hyphen_indicator_char(e->hyphen_indicator_char)
{
}

void environment::copy(const environment *e)
{
  prev_line_length = e->prev_line_length;
  line_length = e->line_length;
  prev_title_length = e->prev_title_length;
  title_length = e->title_length;
  prev_size = e->prev_size;
  size = e->size;
  prev_requested_size = e->prev_requested_size;
  requested_size = e->requested_size;
  char_height = e->char_height;
  char_slant = e->char_slant;
  space_size = e->space_size;
  sentence_space_size = e->sentence_space_size;
  adjust_mode = e->adjust_mode;
  is_filling = e->is_filling;
  was_line_interrupted = false;
  was_previous_line_interrupted = 0;
  centered_line_count = 0;
  right_aligned_line_count = 0;
  prev_vertical_spacing = e->prev_vertical_spacing;
  vertical_spacing = e->vertical_spacing;
  prev_post_vertical_spacing = e->prev_post_vertical_spacing,
  post_vertical_spacing = e->post_vertical_spacing,
  prev_line_spacing = e->prev_line_spacing;
  line_spacing = e->line_spacing;
  prev_indent = e->prev_indent;
  indent = e->indent;
  have_temporary_indent = false;
  temporary_indent = 0;
  underlined_line_count = 0;
  underline_spaces = false;
  input_trap_count = -1;
  continued_input_trap = false;
  prev_text_length = e->prev_text_length;
  width_total = 0;
  space_total = 0;
  input_line_start = 0;
  control_character = e->control_character;
  no_break_control_character = e->no_break_control_character;
  hyphen_indicator_char = e->hyphen_indicator_char;
  is_spreading = false;
  line = 0 /* nullptr */;
  pending_lines = 0 /* nullptr */;
  is_discarding = false;
  tabs = e->tabs;
  using_line_tabs = e->using_line_tabs;
  current_tab = TAB_NONE;
  has_current_field = false;
  margin_character_flags = e->margin_character_flags;
  if (e->margin_character_node)
    margin_character_node = e->margin_character_node->copy();
  margin_character_distance = e->margin_character_distance;
  numbering_nodes = 0 /* nullptr */;
  number_text_separation = e->number_text_separation;
  line_number_multiple = e->line_number_multiple;
  line_number_indent = e->line_number_indent;
  no_number_count = e->no_number_count;
  tab_char = e->tab_char;
  leader_char = e->leader_char;
  hyphenation_mode = e->hyphenation_mode;
  hyphenation_mode_default = e->hyphenation_mode_default;
  fontno = e->fontno;
  prev_fontno = e->prev_fontno;
  is_dummy_env = e->is_dummy_env;
  family = e->family;
  prev_family = e->prev_family;
  leader_node = 0 /* nullptr */;
#ifdef WIDOW_CONTROL
  want_widow_control = e->want_widow_control;
#endif /* WIDOW_CONTROL */
  hyphen_line_max = e->hyphen_line_max;
  hyphen_line_count = 0;
  hyphenation_space = e->hyphenation_space;
  hyphenation_margin = e->hyphenation_margin;
  composite = false;
  stroke_color= e->stroke_color;
  prev_stroke_color = e->prev_stroke_color;
  fill_color = e->fill_color;
  prev_fill_color = e->prev_fill_color;
}

environment::~environment()
{
  delete leader_node;
  delete_node_list(line);
  delete_node_list(numbering_nodes);
}

unsigned char environment::get_control_character()
{
  return control_character;
}

bool environment::set_control_character(unsigned char c)
{
  if (c == no_break_control_character)
    return false;
  control_character = c;
  return true;
}

unsigned char environment::get_no_break_control_character()
{
  return no_break_control_character;
}

bool environment::set_no_break_control_character(unsigned char c)
{
  if (c == control_character)
    return false;
  no_break_control_character = c;
  return true;
}

hunits environment::get_input_line_position()
{
  hunits n;
  if (line == 0 /* nullptr */)
    n = -input_line_start;
  else
    n = width_total - input_line_start;
  if (current_tab != TAB_NONE)
    n += tab_width;
  return n;
}

void environment::set_input_line_position(hunits n)
{
  input_line_start = line == 0 /* nullptr */ ? -n : width_total - n;
  if (current_tab != TAB_NONE)
    input_line_start += tab_width;
}

hunits environment::get_line_length()
{
  return line_length;
}

hunits environment::get_saved_line_length()
{
  if (line)
    return target_text_length + saved_indent;
  else
    return line_length;
}

vunits environment::get_vertical_spacing()
{
  return vertical_spacing;
}

vunits environment::get_post_vertical_spacing()
{
  return post_vertical_spacing;
}

int environment::get_line_spacing()
{
  return line_spacing;
}

vunits environment::total_post_vertical_spacing()
{
  vunits tem(post_vertical_spacing);
  if (line_spacing > 1)
    tem += (line_spacing - 1)*vertical_spacing;
  return tem;
}

hunits environment::get_emboldening_offset()
{
  return env_font_emboldening_offset(this, fontno);
}

hunits environment::get_digit_width()
{
  return env_digit_width(this);
}

unsigned int environment::get_adjust_mode()
{
  return adjust_mode;
}

int environment::get_fill()
{
  return is_filling;
}

hunits environment::get_indent()
{
  return indent;
}

hunits environment::get_saved_indent()
{
  if (line)
    return saved_indent;
  else if (have_temporary_indent)
    return temporary_indent;
  else
    return indent;
}

hunits environment::get_temporary_indent()
{
  return temporary_indent;
}

hunits environment::get_title_length()
{
  return title_length;
}

node *environment::get_prev_char()
{
  for (node *nd = (current_tab != TAB_NONE) ? tab_contents : line;
       nd != 0 /* nullptr */;
       nd = nd->next) {
    node *last = nd->last_char_node();
    if (last)
      return last;
  }
  return 0 /* nullptr */;
}

hunits environment::get_prev_char_width()
{
  node *last = get_prev_char();
  if (!last)
    return H0;
  return last->width();
}

hunits environment::get_prev_char_skew()
{
  node *last = get_prev_char();
  if (!last)
    return H0;
  return last->skew();
}

vunits environment::get_prev_char_height()
{
  node *last = get_prev_char();
  if (!last)
    return V0;
  vunits min, max;
  last->vertical_extent(&min, &max);
  return -min;
}

vunits environment::get_prev_char_depth()
{
  node *last = get_prev_char();
  if (!last)
    return V0;
  vunits min, max;
  last->vertical_extent(&min, &max);
  return max;
}

hunits environment::get_text_length()
{
  hunits n = line == 0 /* nullptr */ ? H0 : width_total;
  if (current_tab != TAB_NONE)
    n += tab_width;
  return n;
}

hunits environment::get_prev_text_length()
{
  return prev_text_length;
}


static int sb_reg_contents = 0;
static int st_reg_contents = 0;
static int ct_reg_contents = 0;
static int rsb_reg_contents = 0;
static int rst_reg_contents = 0;
static int skw_reg_contents = 0;
static int ssc_reg_contents = 0;

void environment::width_registers()
{
  // this is used to implement \w; it sets the st, sb, ct registers
  vunits min = 0, max = 0, cur = 0;
  int character_type = 0;
  ssc_reg_contents = line ? line->subscript_correction().to_units() : 0;
  skw_reg_contents = line ? line->skew().to_units() : 0;
  line = reverse_node_list(line);
  vunits real_min = V0;
  vunits real_max = V0;
  vunits v1, v2;
  for (node *tem = line; tem != 0 /* nullptr */; tem = tem->next) {
    tem->vertical_extent(&v1, &v2);
    v1 += cur;
    if (v1 < real_min)
      real_min = v1;
    v2 += cur;
    if (v2 > real_max)
      real_max = v2;
    if ((cur += tem->vertical_width()) < min)
      min = cur;
    else if (cur > max)
      max = cur;
    character_type |= tem->character_type();
  }
  line = reverse_node_list(line);
  st_reg_contents = -min.to_units();
  sb_reg_contents = -max.to_units();
  rst_reg_contents = -real_min.to_units();
  rsb_reg_contents = -real_max.to_units();
  ct_reg_contents = character_type;
}

node *environment::extract_output_line()
{
  if (current_tab != TAB_NONE)
    wrap_up_tab();
  node *nd = line;
  line = 0 /* nullptr */;
  return nd;
}

static void select_fill_color_request()
{
  symbol s = read_identifier();
  if (s.is_null())
    curenv->set_fill_color(curenv->get_prev_fill_color());
  else
    do_fill_color(s);
  skip_line();
}

static void select_stroke_color_request()
{
  symbol s = read_identifier();
  if (s.is_null())
    curenv->set_stroke_color(curenv->get_prev_stroke_color());
  else
    do_stroke_color(s);
  skip_line();
}

static symbol P_symbol("P");

// Select font with name or mounting position `s`.
void select_font(symbol s)
{
  bool is_number = true;
  if (s.is_null() || s.is_empty())
    s = P_symbol;
  if (s == P_symbol)
    is_number = false;
  else {
    const char *p = s.contents();
    assert(*p != 0 /* nullptr */);
    if ((csdigit(*p)) || ('-' == *p))
      p++;
    for (; p != 0 /* nullptr */ && *p != '\0'; p++)
      if (!csdigit(*p)) {
	is_number = false;
	break;
      }
  }
  // environment::set_font warns if an unused mounting position is
  // requested.  We must warn here if a bogus font name is selected.
  if (is_number) {
    errno = 0;
    long val = strtol(s.contents(), NULL, 10);
    if ((ERANGE == errno) || (val > INT_MAX) || (val < 0))
      warning(WARN_RANGE, "font mounting position must be in range"
	      " 0..%1, got %2", INT_MAX, s.contents());
    else
      (void) curenv->set_font(int(val));
  }
  else {
    if (s == "DESC")
      error("'%1' is not a valid font name", s.contents());
    else if (!curenv->set_font(s))
      warning(WARN_FONT, "cannot select font '%1'", s.contents());
  }
}

static void select_font_request()
{
  select_font(read_identifier());
  skip_line();
}

void family_change()
{
  if (in_nroff_mode) {
    skip_line();
    return;
  }
  symbol s = read_identifier();
  curenv->set_family(s);
  skip_line();
}

void point_size()
{
  if (in_nroff_mode) {
    skip_line();
    return;
  }
  int n;
  if (has_arg()
      && read_measurement(&n, 'z', curenv->get_requested_point_size()))
  {
    if (n <= 0)
      n = 1;
    curenv->set_size(n);
  }
  else
    curenv->set_size(0);
  skip_line();
}

static void override_available_type_sizes_request()
{
  if (!has_arg(true /* peek */)) {
    warning(WARN_MISSING, "available font sizes override request"
	    " expects at least one argument");
    skip_line();
    return;
  }
  if (in_nroff_mode) {
    skip_line();
    return;
  }
  int n = 16;
  int *sizes = new int[n]; // C++03: new int[n]();
  (void) memset(sizes, 0, (n * sizeof(int)));
  int i = 0;
  char *buf = read_rest_of_line_as_argument();
  if (0 /* nullptr */ == buf)
    return;
  char *p = strtok(buf, " \t");
  for (;;) {
    if (0 /* nullptr */ == p)
      break;
    int lower, upper;
    switch (sscanf(p, "%d-%d", &lower, &upper)) {
    case 1:
      upper = lower;
      // fall through
    case 2:
      if ((lower <= upper) && (lower >= 0))
	break;
      // fall through
    default:
      warning(WARN_RANGE, "invalid size range '%1'", p);
      return;
    }
    if ((i + 2) > n) {
      int *old_sizes = sizes;
      sizes = new int[n * 2]; // C++03: new int[n * 2]();
      (void) memset(sizes, 0, (n * 2 * sizeof(int)));
      memcpy(sizes, old_sizes, (n * sizeof(int)));
      n *= 2;
      delete[] old_sizes;
    }
    sizes[i++] = lower;
    if (0 == lower)
      break;
    sizes[i++] = upper;
    p = strtok(0 /* nullptr */, " \t");
  }
  font_size::init_size_list(sizes);
  tok.next();
}

void space_size()
{
  if (!has_arg()) {
    warning(WARN_MISSING, "space size configuration request expects"
	    " at least one argument");
    skip_line();
    return;
  }
  int n;
  if (read_integer(&n)) {
    if (n < 0)
      warning(WARN_RANGE, "ignoring negative word space size: '%1'", n);
    else
      curenv->space_size = n;
    if (has_arg() && read_integer(&n))
      if (n < 0)
	warning(WARN_RANGE, "ignoring negative sentence space size: "
		"'%1'", n);
      else
	curenv->sentence_space_size = n;
    else
      curenv->sentence_space_size = curenv->space_size;
  }
  skip_line();
}

void fill()
{
  while (!tok.is_newline() && !tok.is_eof())
    tok.next();
  if (was_invoked_with_regular_control_character)
    curenv->do_break();
  curenv->is_filling = true;
  tok.next();
}

void no_fill()
{
  while (!tok.is_newline() && !tok.is_eof())
    tok.next();
  if (was_invoked_with_regular_control_character)
    curenv->do_break();
  curenv->is_filling = false;
  curenv->suppress_next_eol = true;
  tok.next();
}

void center()
{
  int n;
  if (!has_arg() || !read_integer(&n))
    n = 1;
  else if (n < 0)
    n = 0;
  while (!tok.is_newline() && !tok.is_eof())
    tok.next();
  if (was_invoked_with_regular_control_character)
    curenv->do_break();
  curenv->right_aligned_line_count = 0;
  curenv->centered_line_count = n;
  curdiv->modified_tag.incl(MTSM_CE);
  tok.next();
}

void right_justify()
{
  int n;
  if (!has_arg() || !read_integer(&n))
    n = 1;
  else if (n < 0)
    n = 0;
  while (!tok.is_newline() && !tok.is_eof())
    tok.next();
  if (was_invoked_with_regular_control_character)
    curenv->do_break();
  curenv->centered_line_count = 0;
  curenv->right_aligned_line_count = n;
  curdiv->modified_tag.incl(MTSM_RJ);
  tok.next();
}

void line_length()
{
  hunits temp;
  if (has_arg() && read_hunits(&temp, 'm', curenv->line_length)) {
    if (temp < hresolution) {
      warning(WARN_RANGE, "setting computed line length %1u to device"
			  " horizontal motion quantum",
			  temp.to_units());
      temp = hresolution;
    }
  }
  else
    temp = curenv->prev_line_length;
  curenv->prev_line_length = curenv->line_length;
  curenv->line_length = temp;
  curdiv->modified_tag.incl(MTSM_LL);
  skip_line();
}

void title_length()
{
  hunits temp;
  if (has_arg() && read_hunits(&temp, 'm', curenv->title_length)) {
    if (temp < hresolution) {
      warning(WARN_RANGE, "setting computed title length %1u to device"
			  " horizontal motion quantum",
			  temp.to_units());
      temp = hresolution;
    }
  }
  else
    temp = curenv->prev_title_length;
  curenv->prev_title_length = curenv->title_length;
  curenv->title_length = temp;
  skip_line();
}

void vertical_spacing()
{
  vunits temp;
  if (has_arg() && read_vunits(&temp, 'p', curenv->vertical_spacing)) {
    if (temp < V0) {
      warning(WARN_RANGE, "vertical spacing must be nonnegative");
      temp = vresolution;
    }
  }
  else
    temp = curenv->prev_vertical_spacing;
  curenv->prev_vertical_spacing = curenv->vertical_spacing;
  curenv->vertical_spacing = temp;
  skip_line();
}

void post_vertical_spacing()
{
  vunits temp;
  if (has_arg() && read_vunits(&temp, 'p',
			      curenv->post_vertical_spacing)) {
    if (temp < V0) {
      warning(WARN_RANGE, "post-vertical spacing must be nonnegative");
      temp = V0;
    }
  }
  else
    temp = curenv->prev_post_vertical_spacing;
  curenv->prev_post_vertical_spacing = curenv->post_vertical_spacing;
  curenv->post_vertical_spacing = temp;
  skip_line();
}

void line_spacing()
{
  int temp;
  if (has_arg() && read_integer(&temp)) {
    if (temp < 1) {
      warning(WARN_RANGE, "line spacing value '%1' is out of range;"
	      " assuming '1'", temp);
      temp = 1;
    }
  }
  else
    temp = curenv->prev_line_spacing;
  curenv->prev_line_spacing = curenv->line_spacing;
  curenv->line_spacing = temp;
  skip_line();
}

void indent()
{
  hunits temp;
  if (has_arg() && read_hunits(&temp, 'm', curenv->indent)) {
    if (temp < H0) {
      warning(WARN_RANGE, "treating %1u indentation as zero",
	      temp.to_units());
      temp = H0;
    }
  }
  else
    temp = curenv->prev_indent;
  while (!tok.is_newline() && !tok.is_eof())
    tok.next();
  if (was_invoked_with_regular_control_character)
    curenv->do_break();
  curenv->have_temporary_indent = false;
  curenv->prev_indent = curenv->indent;
  curenv->indent = temp;
  curdiv->modified_tag.incl(MTSM_IN);
  tok.next();
}

void temporary_indent()
{
  bool is_valid = true;
  hunits temp = H0;
  if (!has_arg()) {
    warning(WARN_MISSING, "temporary indentation request expects"
	    " argument");
    skip_line();
    // _Don't_ return early; when invoked with the ordinary control
    // character this request still breaks the line.
  }
  else {
    if (!read_hunits(&temp, 'm', curenv->get_indent()))
      is_valid = false;
    while (!tok.is_newline() && !tok.is_eof())
      tok.next();
  }
  if (was_invoked_with_regular_control_character)
    curenv->do_break();
  if (temp < H0) {
    warning(WARN_RANGE, "treating total indentation %1u as zero",
	    temp.to_units());
    temp = H0;
  }
  if (is_valid) {
    curenv->temporary_indent = temp;
    curenv->have_temporary_indent = true;
    curdiv->modified_tag.incl(MTSM_TI);
  }
  tok.next();
}

void configure_underlining(bool want_spaces_underlined)
{
  int n;
  if (!has_arg() || !read_integer(&n))
    n = 1;
  if (n <= 0) {
    if (curenv->underlined_line_count > 0) {
      curenv->prev_fontno = curenv->fontno;
      curenv->fontno = curenv->pre_underline_fontno;
      if (want_spaces_underlined) {
	curenv->underline_spaces = false;
	curenv->add_node(configure_space_underlining(false));
      }
    }
    curenv->underlined_line_count = 0;
  }
  else {
    curenv->underlined_line_count = n;
    curenv->pre_underline_fontno = curenv->fontno;
    curenv->fontno = get_underline_fontno();
    if (want_spaces_underlined) {
      curenv->underline_spaces = true;
      curenv->add_node(configure_space_underlining(true));
    }
  }
  skip_line();
}

void continuous_underline()
{
  configure_underlining(true /* underline spaces */);
}

void underline()
{
  configure_underlining(false /* underline spaces */);
}

void margin_character()
{
  tok.skip_spaces();
  charinfo *ci = tok.get_charinfo();
  if (0 /* nullptr */ == ci) { // no argument
    tok.diagnose_non_character();
    curenv->margin_character_flags &= ~environment::MC_ON;
    if (curenv->margin_character_flags == 0U) {
      delete curenv->margin_character_node;
      curenv->margin_character_node = 0 /* nullptr */;
    }
  }
  else {
    // Call tok.next() only after making the node so that
    // .mc \s+9\(br\s0 works.
    node *nd = curenv->make_char_node(ci);
    tok.next();
    if (nd) {
      delete curenv->margin_character_node;
      curenv->margin_character_node = nd;
      curenv->margin_character_flags = environment::MC_ON
				       | environment::MC_NEXT;
      hunits d;
      if (has_arg() && read_hunits(&d, 'm'))
	curenv->margin_character_distance = d;
    }
  }
  skip_line();
}

void number_lines()
{
  delete_node_list(curenv->numbering_nodes);
  curenv->numbering_nodes = 0 /* nullptr */;
  if (has_arg()) {
    node *nd = 0 /* nullptr */;
    for (int i = '9'; i >= '0'; i--) {
      node *tem = make_node(charset_table[i], curenv);
      if (!tem) {
	skip_line();
	return;
      }
      tem->next = nd;
      nd = tem;
    }
    curenv->numbering_nodes = nd;
    curenv->line_number_digit_width = env_digit_width(curenv);
    int n;
    if (!tok.is_usable_as_delimiter()) { // XXX abuse of function
      if (read_integer(&n, next_line_number)) {
	next_line_number = n;
	if (next_line_number < 0) {
	  warning(WARN_RANGE, "output line number cannot be negative");
	  next_line_number = 0;
	}
      }
    }
    else
      while (!tok.is_space() && !tok.is_newline() && !tok.is_eof())
	tok.next();
    if (has_arg()) {
      if (!tok.is_usable_as_delimiter()) { // XXX abuse of function
	if (read_integer(&n)) {
	  if (n <= 0) {
	    warning(WARN_RANGE, "output line number multiple cannot"
		    "be nonpositive");
	  }
	  else
	    curenv->line_number_multiple = n;
	}
      }
      else
	while (!tok.is_space() && !tok.is_newline() && !tok.is_eof())
	  tok.next();
      if (has_arg()) {
	if (!tok.is_usable_as_delimiter()) { // XXX abuse of function
	  if (read_integer(&n))
	    curenv->number_text_separation = n;
	}
	else
	  while (!tok.is_space() && !tok.is_newline() && !tok.is_eof())
	    tok.next();
	if (has_arg() && !tok.is_usable_as_delimiter() // XXX abuse of function
	    && read_integer(&n))
	  curenv->line_number_indent = n;
      }
    }
  }
  skip_line();
}

void no_number()
{
  int n;
  if (has_arg() && read_integer(&n))
    curenv->no_number_count = n > 0 ? n : 0;
  else
    curenv->no_number_count = 1;
  skip_line();
}

void no_hyphenate()
{
  curenv->hyphenation_mode = 0;
  skip_line();
}

void hyphenate_request()
{
  int n;
  if (has_arg() && read_integer(&n)) {
    if (n < HYPHEN_NONE) {
      warning(WARN_RANGE, "ignoring negative hyphenation mode: %1", n);
    } else if (n > HYPHEN_MAX) {
      warning(WARN_RANGE, "hyphenation mode must be in range 0..%1, got"
	      " %2", HYPHEN_MAX, n);
    } else if (((n & HYPHEN_DEFAULT) && (n & ~HYPHEN_DEFAULT))
	|| ((n & HYPHEN_FIRST_CHAR) && (n & HYPHEN_NOT_FIRST_CHARS))
	|| ((n & HYPHEN_LAST_CHAR) && (n & HYPHEN_NOT_LAST_CHARS)))
      warning(WARN_SYNTAX, "ignoring self-contradictory hyphenation"
	      " mode: %1", n);
    else
      curenv->hyphenation_mode = n;
  }
  else
    curenv->hyphenation_mode = curenv->hyphenation_mode_default;
  skip_line();
}

void set_hyphenation_mode_default()
{
  if (!has_arg()) {
    warning(WARN_MISSING, "hyphenation mode default setting request"
	    " expects an argument");
    skip_line();
    return;
  }
  int n;
  if (!read_integer(&n)) { // throws a diagnostic if necessary
    skip_line();
    return;
  }
  if (n < 0) {
    warning(WARN_RANGE, "hyphenation mode default cannot be negative");
    skip_line();
    return;
  }
  curenv->hyphenation_mode_default = n;
  skip_line();
}

// Set hyphenation character, which the input uses to mark the position
// of a discretionary break ("dbreak") in a word.
void hyphenation_character_request()
{
  curenv->hyphen_indicator_char = read_character();
  // TODO?: If null pointer, set to ESCAPE_PERCENT, eliminating test(s)
  // while processing output line?
  skip_line();
}

void hyphen_line_max_request()
{
  int n;
  if (has_arg() && read_integer(&n))
    curenv->hyphen_line_max = n;
  else
    curenv->hyphen_line_max = -1;
  skip_line();
}

void environment::interrupt()
{
  if (!is_dummy_env) {
    add_node(new transparent_dummy_node);
    was_line_interrupted = true;
  }
}

void environment::newline()
{
  bool was_centered = false;
  if (underlined_line_count > 0) {
    if (--underlined_line_count == 0) {
      prev_fontno = fontno;
      fontno = pre_underline_fontno;
      if (underline_spaces) {
	underline_spaces = false;
	add_node(configure_space_underlining(false));
      }
    }
  }
  if (has_current_field)
    wrap_up_field();
  if (current_tab != TAB_NONE)
    wrap_up_tab();
  // strip trailing spaces
  while (line != 0 /* nullptr */ && line->discardable()) {
    width_total -= line->width();
    space_total -= line->nspaces();
    node *tem = line;
    line = line->next;
    delete tem;
  }
  node *to_be_output = 0 /* nullptr */;
  hunits to_be_output_width;
  was_previous_line_interrupted = 0;
  if (is_dummy_env)
    space_newline();
  else if (was_line_interrupted) {
    was_line_interrupted = false;
    // see environment::final_break
    was_previous_line_interrupted = is_exit_underway ? 2 : 1;
  }
  else if (centered_line_count > 0) {
    --centered_line_count;
    hunits x = target_text_length - width_total;
    if (x > H0)
      saved_indent += x/2;
    to_be_output = line;
    was_centered = true;
    to_be_output_width = width_total;
    line = 0; /* nullptr */
  }
  else if (right_aligned_line_count > 0) {
    --right_aligned_line_count;
    hunits x = target_text_length - width_total;
    if (x > H0)
      saved_indent += x;
    to_be_output = line;
    to_be_output_width = width_total;
    line = 0 /* nullptr */;
  }
  else if (is_filling)
    space_newline();
  else {
    to_be_output = line;
    to_be_output_width = width_total;
    line = 0 /* nullptr */;
  }
  input_line_start = line == 0 /* nullptr */ ? H0 : width_total;
  if (to_be_output) {
    if (is_writing_html && !is_filling) {
      curdiv->modified_tag.incl(MTSM_EOL);
      if (suppress_next_eol)
	suppress_next_eol = false;
      else
	seen_eol = true;
    }

    output_line(to_be_output, to_be_output_width, was_centered);
    hyphen_line_count = 0;
  }
  if (input_trap_count > 0) {
    if (!(continued_input_trap && (was_previous_line_interrupted > 0)))
      if (--input_trap_count == 0)
	spring_trap(input_trap);
  }
}

void environment::output_line(node *nd, hunits width, bool was_centered)
{
  prev_text_length = width;
  if (margin_character_flags > 0U) {
    hunits d = line_length + margin_character_distance - saved_indent
	       - width;
    if (d > 0) {
      nd = new hmotion_node(d, get_fill_color(), nd);
      width += d;
    }
    margin_character_flags &= ~MC_NEXT;
    node *tem;
    if (!margin_character_flags) {
      tem = margin_character_node;
      margin_character_node = 0 /* nullptr */;
    }
    else
      tem = margin_character_node->copy();
    tem->next = nd;
    nd = tem;
    width += tem->width();
  }
  node *nn = 0 /* nullptr */;
  while (nd != 0 /* nullptr */) {
    node *tem = nd->next;
    nd->next = nn;
    nn = nd;
    nd = tem;
  }
  if (!saved_indent.is_zero())
    nn = new hmotion_node(saved_indent, get_fill_color(), nn);
  width += saved_indent;
  if (no_number_count > 0)
    --no_number_count;
  else if (numbering_nodes) {
    hunits w = (line_number_digit_width
		* (3 + line_number_indent + number_text_separation));
    if ((next_line_number % line_number_multiple) != 0)
      nn = new hmotion_node(w, get_fill_color(), nn);
    else {
      hunits x = w;
      nn = new hmotion_node(number_text_separation
			    * line_number_digit_width,
			    get_fill_color(), nn);
      x -= number_text_separation*line_number_digit_width;
      char buf[UINT_DIGITS];
      (void) sprintf(buf, "%3u", next_line_number);
      for (char *p = strchr(buf, '\0') - 1; p >= buf && *p != ' '; --p)
      {
	node *gn = numbering_nodes;
	for (int count = *p - '0'; count > 0; count--)
	  gn = gn->next;
	gn = gn->copy();
	x -= gn->width();
	gn->next = nn;
	nn = gn;
      }
      nn = new hmotion_node(x, get_fill_color(), nn);
    }
    width += w;
    ++next_line_number;
  }
  output(nn, !is_filling, vertical_spacing,
	 total_post_vertical_spacing(), width, was_centered);
}

void environment::start_line()
{
  assert(line == 0 /* nullptr */);
  is_discarding = false;
  line = new line_start_node;
  if (have_temporary_indent) {
    saved_indent = temporary_indent;
    have_temporary_indent = false;
  }
  else
    saved_indent = indent;
  target_text_length = line_length - saved_indent;
  width_total = H0;
  space_total = 0;
}

hunits environment::get_hyphenation_space()
{
  return hyphenation_space;
}

void hyphenation_space_request()
{
  hunits n;
  if (read_hunits(&n, 'm')) {
    if (n < H0) {
      warning(WARN_RANGE, "hyphenation space cannot be negative");
      n = H0;
    }
    curenv->hyphenation_space = n;
  }
  skip_line();
}

hunits environment::get_hyphenation_margin()
{
  return hyphenation_margin;
}

void hyphenation_margin_request()
{
  hunits n;
  if (read_hunits(&n, 'm')) {
    if (n < H0) {
      warning(WARN_RANGE, "hyphenation margin cannot be negative");
      n = H0;
    }
    curenv->hyphenation_margin = n;
  }
  skip_line();
}

breakpoint *environment::choose_breakpoint()
{
  hunits x = width_total;
  int s = space_total;
  node *nd = line;
  breakpoint *best_bp = 0 /* nullptr */; // the best breakpoint so far
  bool best_bp_fits = false;
  while (nd != 0 /* nullptr */) {
    x -= nd->width();
    s -= nd->nspaces();
    breakpoint *bp = nd->get_breakpoints(x, s);
    while (bp != 0 /* nullptr */) {
      if (bp->width <= target_text_length) {
	if (!bp->hyphenated) {
	  breakpoint *tem = bp->next;
	  bp->next = 0 /* nullptr */;
	  while (tem != 0 /* nullptr */) {
	    breakpoint *tem1 = tem;
	    tem = tem->next;
	    delete tem1;
	  }
	  if (best_bp_fits
	      // Decide whether to use the hyphenated breakpoint.
	      && ((hyphen_line_max < 0)
		  // Only choose the hyphenated breakpoint if it would
		  // not exceed the maximum number of consecutive
		  // hyphenated lines.
		  || (hyphen_line_count + 1 <= hyphen_line_max))
	      && !((adjust_mode == ADJUST_BOTH)
		   // Don't choose the hyphenated breakpoint if the line
		   // can be justified by adding no more than
		   // hyphenation_space to any word space.
		   ? ((bp->nspaces > 0)
		      && (((target_text_length - bp->width)
			    + ((bp->nspaces - 1) * hresolution))
			    / bp->nspaces)
			   <= hyphenation_space)
		   // Don't choose the hyphenated breakpoint if the line
		   // is no more than hyphenation_margin short of the
		   // line length.
		   : ((target_text_length - bp->width)
		      <= hyphenation_margin))) {
	    delete bp;
	    return best_bp;
	  }
	  if (best_bp)
	    delete best_bp;
	  return bp;
	}
	else {
	  if ((adjust_mode == ADJUST_BOTH
	       ? hyphenation_space == H0
	       : hyphenation_margin == H0)
	      && (hyphen_line_max < 0
		  || hyphen_line_count + 1 <= hyphen_line_max)) {
	    // No need to consider a non-hyphenated breakpoint.
	    if (best_bp)
	      delete best_bp;
	    breakpoint *tem = bp->next;
	    bp->next = 0 /* nullptr */;
	    while (tem != 0 /* nullptr */) {
	      breakpoint *tem1 = tem;
	      tem = tem->next;
	      delete tem1;
	    }
	    return bp;
	  }
	  // It fits but it's hyphenated.
	  if (!best_bp_fits) {
	    if (best_bp)
	      delete best_bp;
	    best_bp = bp;
	    bp = bp->next;
	    best_bp_fits = true;
	  }
	  else {
	    breakpoint *tem = bp;
	    bp = bp->next;
	    delete tem;
	  }
	}
      }
      else {
	if (best_bp)
	  delete best_bp;
	best_bp = bp;
	bp = bp->next;
      }
    }
    nd = nd->next;
  }
  if (best_bp)
    return best_bp;
  return 0 /* nullptr */;
}

// Iterate over the nodes of the output line, looking for break points.
// The node list is in reverse order, so the first node we see is the
// last (or rightmost) on the line.  Whether a break requires
// hyphenation depends on the properties of the node and the context;
// if we're at a word boundary, we can break without a hyphen regardless
// of the node's own hyphenation properties.
void environment::possibly_hyphenate_line(bool must_break_here)
{
  assert(line != 0 /* nullptr */);
  hyphenation_type prev_type = line->get_hyphenation_type();
  node **startp;
  if (must_break_here)
    startp = &line;
  else
    for (startp = &line->next; *startp != 0 /* nullptr */;
	 startp = &(*startp)->next) {
      hyphenation_type this_type = (*startp)->get_hyphenation_type();
      if (prev_type == HYPHENATION_UNNECESSARY
	  && this_type == HYPHENATION_PERMITTED)
	break;
      prev_type = this_type;
    }
  if (*startp == 0 /* nullptr */)
    return;
  node *tem = *startp;
  do {
    tem = tem->next;
  } while (tem != 0 /* nullptr */
	   && tem->get_hyphenation_type() == HYPHENATION_PERMITTED);
  // This is for characters like hyphen and em dash.
  bool inhibit = ((tem != 0 /* nullptr */)
		  && (tem->get_hyphenation_type()
		      == HYPHENATION_INHIBITED));
  node *end = tem;
  hyphen_list *sl = 0 /* nullptr */;
  tem = *startp;
  node *forward = 0 /* nullptr */;
  int i = 0;
  while (tem != end) {
    sl = tem->get_hyphen_list(sl, &i);
    node *tem1 = tem;
    tem = tem->next;
    tem1->next = forward;
    forward = tem1;
  }
  if (!inhibit) {
    unsigned char prev_code = 0U;
    for (hyphen_list *h = sl; h; h = h->next) {
      h->is_breakable = (prev_code != 0U
			 && h->next != 0 /* nullptr */
			 && h->next->hyphenation_code != 0U);
      prev_code = h->hyphenation_code;
    }
  }
  if ((hyphenation_mode != 0)
      && !inhibit
      // this may not be right if we have extra space on this line
      && !((hyphenation_mode & HYPHEN_NOT_LAST_LINE)
	   && (curdiv->distance_to_next_trap()
	       <= vertical_spacing + total_post_vertical_spacing()))
      && i >= (4
	       - (hyphenation_mode & HYPHEN_FIRST_CHAR ? 1 : 0)
	       - (hyphenation_mode & HYPHEN_LAST_CHAR ? 1 : 0)
	       + (hyphenation_mode & HYPHEN_NOT_FIRST_CHARS ? 1 : 0)
	       + (hyphenation_mode & HYPHEN_NOT_LAST_CHARS ? 1 : 0)))
    hyphenate(sl, hyphenation_mode);
  while (forward != 0 /* nullptr */) {
    node *tem1 = forward;
    forward = forward->next;
    tem1->next = 0 /* nullptr */;
    tem = tem1->add_self(tem, &sl);
  }
  *startp = tem;
}

static node *node_list_reverse(node *nd)
{
  node *res = 0 /* nullptr */;
  while (nd) {
    node *tem = nd;
    nd = nd->next;
    tem->next = res;
    res = tem;
  }
  return res;
}

static bool distribute_space(node *nd, int nspaces,
			     hunits desired_space,
			     bool force_reverse_node_list = false)
{
  if (desired_space.is_zero() || (nspaces == 0) || (desired_space < H0))
    return false;
  // Positive desired space is the typical case.  Negative desired space
  // is possible if we have overrun an unbreakable line.  But we should
  // not get here if there are no adjustable space nodes to adjust.
  assert(nspaces > 0);
  // Space cannot always be distributed evenly among all of the space
  // nodes in the node list: there are limits to device resolution.  We
  // add space until we run out, which might happen before the end of
  // the line.  To achieve uniform typographical grayness and avoid
  // rivers, we switch the end from which space is initially distributed
  // with each line requiring it, unless compelled to reverse it.  We
  // distribute space initially from the left, unlike AT&T troff.
  static bool do_reverse_node_list = false;
  if (force_reverse_node_list || do_reverse_node_list)
    nd = node_list_reverse(nd);
  if (!force_reverse_node_list && spread_limit >= 0
      && desired_space.to_units() > 0) {
    hunits em = curenv->get_size();
    double Ems = static_cast<double>(desired_space.to_units()) / nspaces
		 / (em.is_zero() ? hresolution : em.to_units());
    if (Ems > spread_limit)
      output_warning(WARN_BREAK, "spreading %1m per space", Ems);
  }
  for (node *tem = nd; tem != 0 /* nullptr */; tem = tem->next)
    tem->spread_space(&nspaces, &desired_space);
  if (force_reverse_node_list || do_reverse_node_list)
    (void) node_list_reverse(nd);
  if (!force_reverse_node_list)
    do_reverse_node_list = !do_reverse_node_list;
  return true;
}

// from input.cpp
extern double warn_scale;
extern char warn_scaling_unit;

void environment::possibly_break_line(bool must_break_here,
				      bool must_adjust)
{
  bool was_centered = (centered_line_count > 0);
  if (!is_filling || (current_tab != TAB_NONE) || has_current_field
      || is_dummy_env)
    return;
  while ((line != 0 /* nullptr */)
	 && (must_adjust
	     // When a macro follows a paragraph in fill mode, the
	     // current line should not be empty.
	     || ((width_total - line->width()) > target_text_length))) {
    possibly_hyphenate_line(must_break_here);
    breakpoint *bp = choose_breakpoint();
    if (0 /* nullptr */ == bp)
      // we'll find one eventually
      return;
    node *pre, *post;
    node **ndp = &line;
    while (*ndp != bp->nd)
      ndp = &(*ndp)->next;
    bp->nd->split(bp->index, &pre, &post);
    *ndp = post;
    // The space deficit tells us how much the line is overset if
    // negative, or underset if positive, relative to the configured
    // line length.
    hunits space_deficit = (target_text_length - bp->width);
    // An overset line always gets a warning.
    if (space_deficit < H0) {
      double dsd = static_cast<double>(space_deficit.to_units());
      output_warning(WARN_BREAK, "cannot %1 line; overset by %2%3",
		     (ADJUST_BOTH == adjust_mode) ? "adjust" : "break",
		     in_nroff_mode
		     ? static_cast<int>(ceil(fabs(dsd / hresolution)))
		     : fabs(dsd / warn_scale),
		     in_nroff_mode ? 'n' : warn_scaling_unit);
    }
    // An underset line warns only if it requires adjustment but no
    // adjustable spaces exist on the line.
    else if ((ADJUST_BOTH == adjust_mode)
	     && (space_deficit > H0)
	     && (0 == bp->nspaces)) {
      double dsd = static_cast<double>(space_deficit.to_units());
      output_warning(WARN_BREAK, "cannot adjust line; underset by %1%2",
		     in_nroff_mode
		     ? static_cast<int>(ceil(fabs(dsd / hresolution)))
		     : fabs(dsd / warn_scale),
		     in_nroff_mode ? 'n' : warn_scaling_unit);
    }
    // The extra space is the amount of space to distribute among the
    // adjustable space nodes in an output line; this process occurs
    // only if adjustment is enabled.  We may however want to synthesize
    // an extra indentation from it if centering or right-aligning.
    hunits extra_space = H0;
    switch (adjust_mode) {
    case ADJUST_BOTH:
      if (bp->nspaces != 0)
	extra_space = space_deficit;
      break;
    case ADJUST_CENTER:
      saved_indent += space_deficit / 2;
      was_centered = true;
      break;
    case ADJUST_RIGHT:
      saved_indent += space_deficit;
      break;
    case ADJUST_LEFT:
    case ADJUST_CENTER - 1:
    case ADJUST_RIGHT - 1:
      break;
    default:
      assert(0 == "unhandled case of `adjust_mode`");
    }
    hunits output_width = bp->width; // 0 if no breakpoint is found.
    if (distribute_space(pre, bp->nspaces, extra_space))
      output_width += extra_space;
    // If we had an unbreakable, overset line, we can do no more.
    if (output_width <= 0)
      return;
    input_line_start -= output_width;
    if (bp->hyphenated)
      hyphen_line_count++;
    else
      hyphen_line_count = 0;
    delete bp;
    // Normally, the do_break() member function discards trailing spaces
    // (cf. horizontal motions) from input lines.  But when `\p` is
    // used, that mechanism is bypassed, so we do the equivalent here.
    space_total = 0;
    width_total = 0;
    node *first_non_discardable = 0 /* nullptr */;
    node *tem;
    for (tem = line; tem != 0 /* nullptr */; tem = tem->next)
      if (!tem->discardable())
	first_non_discardable = tem;
    node *to_be_discarded;
    if (first_non_discardable != 0 /* nullptr */) {
      to_be_discarded = first_non_discardable->next;
      first_non_discardable->next = 0 /* nullptr */;
      for (tem = line; tem != 0 /* nullptr */; tem = tem->next) {
	width_total += tem->width();
	space_total += tem->nspaces();
      }
      is_discarding = false;
    }
    else {
      is_discarding = true;
      to_be_discarded = line;
      line = 0 /* nullptr */;
    }
    // Do output_line() here so that line will be 0 iff the
    // the environment will be empty.
    output_line(pre, output_width, was_centered);
    while (to_be_discarded != 0 /* nullptr */) {
      tem = to_be_discarded;
      to_be_discarded = to_be_discarded->next;
      input_line_start -= tem->width();
      delete tem;
    }
    if (line != 0 /* nullptr */) {
      if (have_temporary_indent) {
	saved_indent = temporary_indent;
	have_temporary_indent = false;
      }
      else
	saved_indent = indent;
      target_text_length = line_length - saved_indent;
    }
  }
}

/*
Do the break at the end of input after the end macro (if any).

Unix troff behaves as follows:  if the last line is

foo bar\c

it will output foo on the current page, and bar on the next page;
if the last line is

foo\c

or

foo bar

everything will be output on the current page.  This behaviour must be
considered a bug.

The problem is that some macro packages rely on this.  For example,
the ATK macros have an end macro that emits \c if it needs to print a
table of contents but doesn't do a 'bp in the end macro; instead the
'bp is done in the bottom of page trap.  This works with Unix troff,
provided that the current environment is not empty at the end of the
input file.

The following will make macro packages that do that sort of thing work
even if the current environment is empty at the end of the input file.
If the last input line used \c and this line occurred in the end macro,
then we'll force everything out on the current page, but we'll make
sure that the environment isn't empty so that we won't exit at the
bottom of this page.
*/

void environment::final_break()
{
  if (was_previous_line_interrupted == 2) {
    do_break();
    add_node(new transparent_dummy_node);
  }
  else
    do_break();
}

node *environment::make_tag(const char *nm, int i)
{
  if (is_writing_html) {
    /*
     * need to emit tag for post-grohtml
     * but we check to see whether we can emit specials
     */
    if ((curdiv == topdiv) && (topdiv->before_first_page_status > 0))
      topdiv->begin_page();
    macro m;
    m.append_str("devtag:");
    for (const char *p = nm; *p; p++)
      if (!is_invalid_input_char(static_cast<unsigned char>(*p)))
	m.append(*p);
    m.append(' ');
    m.append_int(i);
    return new device_extension_node(m);
  }
  return new transparent_dummy_node;
}

void environment::dump_troff_state()
{
#define SPACES "                                            "
  fprintf(stderr, SPACES "register 'in' = %d\n",
	  curenv->indent.to_units());
  if (curenv->have_temporary_indent)
    fprintf(stderr, SPACES "register 'ti' = %d\n",
	    curenv->temporary_indent.to_units());
  fprintf(stderr, SPACES "centered lines 'ce' = %d\n",
	  curenv->centered_line_count);
  fprintf(stderr, SPACES "register 'll' = %d\n",
	  curenv->line_length.to_units());
  fprintf(stderr, SPACES "%sfilling\n",
	  curenv->is_filling ? "" : "not ");
  fprintf(stderr, SPACES "page offset 'po' = %d\n",
	  topdiv->get_page_offset().to_units());
  fprintf(stderr, SPACES "seen_break = %d\n", curenv->seen_break);
  fprintf(stderr, SPACES "seen_space = %d\n", curenv->seen_space);
  fprintf(stderr, SPACES "is_discarding = %d\n", curenv->is_discarding);
  fflush(stderr);
#undef SPACES
}

extern void dump_node_list_in_reverse(node *);

void environment::dump_pending_nodes()
{
  dump_node_list_in_reverse(line);
}

statem *environment::construct_state(bool has_only_eol)
{
  if (is_writing_html) {
    statem *s = new statem();
    if (!has_only_eol) {
      s->add_tag(MTSM_IN, indent);
      s->add_tag(MTSM_LL, line_length);
      s->add_tag(MTSM_PO, topdiv->get_page_offset().to_units());
      s->add_tag(MTSM_RJ, right_aligned_line_count);
      if (have_temporary_indent)
	s->add_tag(MTSM_TI, temporary_indent);
      s->add_tag_ta();
      if (seen_break)
	s->add_tag(MTSM_BR);
      if (seen_space)
	s->add_tag(MTSM_SP, seen_space);
      seen_break = false;
      seen_space = false;
    }
    if (seen_eol) {
      s->add_tag(MTSM_EOL);
      s->add_tag(MTSM_CE, centered_line_count);
    }
    seen_eol = false;
    return s;
  }
  else
    return 0 /* nullptr */;
}

void environment::construct_format_state(node *nd, bool was_centered,
					 int filling)
{
  if (is_writing_html) {
    // find first glyph node which has a state.
    while (nd != 0 /* nullptr */ && nd->state == 0 /* nullptr */)
      nd = nd->next;
    if (nd == 0 /* nullptr */ || (nd->state == 0 /* nullptr */))
      return;
    if (seen_space)
      nd->state->add_tag(MTSM_SP, seen_space);
    if (seen_eol && topdiv == curdiv)
      nd->state->add_tag(MTSM_EOL);
    seen_space = false;
    seen_eol = false;
    if (was_centered)
      nd->state->add_tag(MTSM_CE, centered_line_count + 1);
    else
      nd->state->add_tag_if_unknown(MTSM_CE, 0);
    nd->state->add_tag_if_unknown(MTSM_FI, filling);
    nd = nd->next;
    while (nd != 0 /* nullptr */) {
      if (nd->state != 0 /* nullptr */) {
	nd->state->sub_tag_ce();
	nd->state->add_tag_if_unknown(MTSM_FI, filling);
      }
      nd = nd->next;
    }
  }
}

void environment::construct_new_line_state(node *nd)
{
  if (is_writing_html) {
    // find first glyph node which has a state.
    while (nd != 0 /* nullptr */ && nd->state == 0 /* nullptr */)
      nd = nd->next;
    if (nd == 0 /* nullptr */ || nd->state == 0 /* nullptr */)
      return;
    if (seen_space)
      nd->state->add_tag(MTSM_SP, seen_space);
    if (seen_eol && topdiv == curdiv)
      nd->state->add_tag(MTSM_EOL);
    seen_space = false;
    seen_eol = false;
  }
}

extern int global_diverted_space;

// "Forced adjustment" refers to spreading of adjustable spaces (and
// perhaps only that, even if in the future we implement "squeezing"),
// when it is not normally called for, as at the end of a paragraph.

void environment::do_break(bool want_forced_adjustment)
{
  bool was_centered = false;
  if ((curdiv == topdiv) && (topdiv->before_first_page_status > 0)) {
    topdiv->begin_page();
    return;
  }
  if (current_tab != TAB_NONE)
    wrap_up_tab();
  if (line) {
    // this is so that hyphenation works
    if (line->nspaces() == 0) {
      line = new space_node(H0, get_fill_color(), line);
      space_total++;
    }
    possibly_break_line(false /* must break here */,
			want_forced_adjustment);
  }
  while (line != 0 /* nullptr */ && line->discardable()) {
    width_total -= line->width();
    space_total -= line->nspaces();
    node *tem = line;
    line = line->next;
    delete tem;
  }
  is_discarding = false;
  input_line_start = H0;
  if (line != 0 /* nullptr */) {
    if (is_filling) {
      switch (adjust_mode) {
      case ADJUST_CENTER:
	saved_indent += (target_text_length - width_total) / 2;
	was_centered = true;
	break;
      case ADJUST_RIGHT:
	saved_indent += target_text_length - width_total;
	break;
      }
    }
    if (want_nodes_dumped && (curdiv == topdiv))
      curenv->dump_pending_nodes();
    node *tem = line;
    line = 0 /* nullptr */;
    output_line(tem, width_total, was_centered);
    hyphen_line_count = 0;
  }
  was_previous_line_interrupted = 0;
#ifdef WIDOW_CONTROL
  mark_last_line();
  output_pending_lines();
#endif /* WIDOW_CONTROL */
  if (!global_diverted_space) {
    curdiv->modified_tag.incl(MTSM_BR);
    seen_break = true;
  }
}

bool environment::is_empty()
{
  return (current_tab == TAB_NONE) && line == 0 /* nullptr */
	  && pending_lines == 0 /* nullptr */;
}

void do_break_request(bool want_adjustment)
{
  while (!tok.is_newline() && !tok.is_eof())
    tok.next();
  if (was_invoked_with_regular_control_character)
    curenv->do_break(want_adjustment);
  tok.next();
}

static void break_without_forced_adjustment_request()
{
  do_break_request(false);
}

static void break_with_forced_adjustment_request()
{
  do_break_request(true);
}

void title()
{
  if (!has_arg(true /* peek */)) {
    warning(WARN_MISSING, "title line request expects a delimited"
	    " argument");
    skip_line();
    return;
  }
  if ((curdiv == topdiv) && (topdiv->before_first_page_status > 0)) {
    handle_initial_title();
    return;
  }
  node *part[3];
  hunits part_width[3];
  part[0] = part[1] = part[2] = 0 /* nullptr */;
  environment env(curenv);
  environment *oldenv = curenv;
  curenv = &env;
  read_title_parts(part, part_width);
  curenv = oldenv;
  curenv->size = env.size;
  curenv->prev_size = env.prev_size;
  curenv->requested_size = env.requested_size;
  curenv->prev_requested_size = env.prev_requested_size;
  curenv->char_height = env.char_height;
  curenv->char_slant = env.char_slant;
  curenv->fontno = env.fontno;
  curenv->prev_fontno = env.prev_fontno;
  curenv->stroke_color = env.stroke_color;
  curenv->prev_stroke_color = env.prev_stroke_color;
  curenv->fill_color = env.fill_color;
  curenv->prev_fill_color = env.prev_fill_color;
  node *nd = 0 /* nullptr */;
  node *p = part[2];
  while (p != 0 /* nullptr */) {
    node *tem = p;
    p = p->next;
    tem->next = nd;
    nd = tem;
  }
  hunits length_title(curenv->title_length);
  hunits f = length_title - part_width[1];
  hunits f2 = f/2;
  nd = new hmotion_node(f2 - part_width[2], curenv->get_fill_color(),
			nd);
  p = part[1];
  while (p != 0) {
    node *tem = p;
    p = p->next;
    tem->next = nd;
    nd = tem;
  }
  nd = new hmotion_node(f - f2 - part_width[0],
			curenv->get_fill_color(), nd);
  p = part[0];
  while (p != 0) {
    node *tem = p;
    p = p->next;
    tem->next = nd;
    nd = tem;
  }
  curenv->output_title(nd, !curenv->is_filling,
		       curenv->vertical_spacing,
		       curenv->total_post_vertical_spacing(),
		       length_title);
  curenv->hyphen_line_count = 0;
  tok.next();
}

void adjust()
{
  curenv->adjust_mode |= 1;
  if (has_arg()) {
    int c = tok.ch(); // safely compares to char literals; TODO: grochar
    switch (c) {
    case 'l':
      curenv->adjust_mode = ADJUST_LEFT;
      break;
    case 'r':
      curenv->adjust_mode = ADJUST_RIGHT;
      break;
    case 'c':
      curenv->adjust_mode = ADJUST_CENTER;
      break;
    case 'b':
    case 'n':
      curenv->adjust_mode = ADJUST_BOTH;
      break;
    default:
      int n;
      if (read_integer(&n)) {
	if (n < 0)
	  warning(WARN_RANGE, "negative adjustment mode");
	else if (n > ADJUST_MAX)
	  warning(WARN_RANGE, "adjustment mode must be in range 0..%1"
		  " (or 'l', 'r', 'c', 'b', or 'n'), got %2",
		  ADJUST_MAX, n);
	else
	  curenv->adjust_mode = n;
      }
    }
  }
  skip_line();
}

void no_adjust()
{
  curenv->adjust_mode &= ~1;
  skip_line();
}

void do_input_trap(bool respect_continuation)
{
  curenv->input_trap_count = -1;
  curenv->input_trap = 0 /* nullptr */;
  curenv->continued_input_trap = respect_continuation;
  int n;
  if (has_arg() && read_integer(&n)) {
    if (n <= 0)
      warning(WARN_RANGE,
	      "input trap line count must be greater than zero");
    else {
      symbol s = read_identifier(true /* required */);
      if (!s.is_null()) {
	curenv->input_trap_count = n;
	curenv->input_trap = s;
      }
    }
  }
  skip_line();
}

void input_trap()
{
  do_input_trap(false);
}

void input_trap_continued()
{
  do_input_trap(true);
}

/* tabs */

// must not be R or C or L or a legitimate part of a numeric expression
// TODO: grochar
const unsigned char TAB_REPEAT_CHAR = (unsigned char)('T');

struct tab {
  tab *next;
  hunits pos;
  tab_type type;
  tab(hunits, tab_type);
  enum { BLOCK = 1024 };
};

tab::tab(hunits x, tab_type t) : next(0), pos(x), type(t)
{
}

tab_stops::tab_stops(hunits distance, tab_type type)
: initial_list(0)
{
  repeated_list = new tab(distance, type);
}

tab_stops::~tab_stops()
{
  clear();
}

tab_type tab_stops::distance_to_next_tab(hunits curpos, hunits *distance)
{
  hunits nextpos;

  return distance_to_next_tab(curpos, distance, &nextpos);
}

tab_type tab_stops::distance_to_next_tab(hunits curpos,
					 hunits *distance,
					 hunits *nextpos)
{
  hunits lastpos = 0;
  tab *tem;
  for (tem = initial_list; tem && tem->pos <= curpos; tem = tem->next)
    lastpos = tem->pos;
  if (tem) {
    *distance = tem->pos - curpos;
    *nextpos  = tem->pos;
    return tem->type;
  }
  if (repeated_list == 0 /* nullptr */)
    return TAB_NONE;
  hunits base = lastpos;
  for (;;) {
    for (tem = repeated_list; tem && tem->pos + base <= curpos;
	 tem = tem->next)
      lastpos = tem->pos;
    if (tem) {
      *distance = tem->pos + base - curpos;
      *nextpos  = tem->pos + base;
      return tem->type;
    }
    if (lastpos < 0)
      lastpos = 0;
    base += lastpos;
  }
  return TAB_NONE;
}

const char *tab_stops::to_string()
{
  static char *buf = 0 /* nullptr */;
  static int buf_size = 0;
  // figure out a maximum on the amount of space we can need
  int count = 0;
  tab *p;
  for (p = initial_list; p; p = p->next)
    ++count;
  for (p = repeated_list; p; p = p->next)
    ++count;
  // (10 for digits + 1 for u + 1 for 'C' or 'R') + 2 for ' &' + 1 for '\0'
  int need = count*12 + 3;
  if (buf == 0 || need > buf_size) {
    if (buf)
      delete[] buf;
    buf_size = need;
    buf = new char[buf_size]; // C++03: new char[buf_size]();
    (void) memset(buf, 0, buf_size * sizeof(char));
  }
  char *ptr = buf;
  for (p = initial_list; p; p = p->next) {
    strcpy(ptr, i_to_a(p->pos.to_units()));
    ptr = strchr(ptr, '\0');
    *ptr++ = 'u';
    *ptr = '\0';
    switch (p->type) {
    case TAB_LEFT:
      break;
    case TAB_RIGHT:
      *ptr++ = 'R';
      break;
    case TAB_CENTER:
      *ptr++ = 'C';
      break;
    case TAB_NONE:
    default:
      assert(0 == "unhandled case of `p->type` (tab_type)");
    }
  }
  if (repeated_list)
    *ptr++ = TAB_REPEAT_CHAR;
  for (p = repeated_list; p; p = p->next) {
    strcpy(ptr, i_to_a(p->pos.to_units()));
    ptr = strchr(ptr, '\0');
    *ptr++ = 'u';
    *ptr = '\0';
    switch (p->type) {
    case TAB_LEFT:
      break;
    case TAB_RIGHT:
      *ptr++ = 'R';
      break;
    case TAB_CENTER:
      *ptr++ = 'C';
      break;
    case TAB_NONE:
    default:
      assert(0 == "unhandled case of `p->type` (tab_type)");
    }
  }
  *ptr++ = '\0';
  return buf;
}

tab_stops::tab_stops() : initial_list(0), repeated_list(0)
{
}

tab_stops::tab_stops(const tab_stops &ts)
: initial_list(0), repeated_list(0)
{
  tab **p = &initial_list;
  tab *t = ts.initial_list;
  while (t) {
    *p = new tab(t->pos, t->type);
    t = t->next;
    p = &(*p)->next;
  }
  p = &repeated_list;
  t = ts.repeated_list;
  while (t) {
    *p = new tab(t->pos, t->type);
    t = t->next;
    p = &(*p)->next;
  }
}

void tab_stops::clear()
{
  while (initial_list) {
    tab *tem = initial_list;
    initial_list = initial_list->next;
    delete tem;
  }
  while (repeated_list) {
    tab *tem = repeated_list;
    repeated_list = repeated_list->next;
    delete tem;
  }
}

void tab_stops::add_tab(hunits pos, tab_type type, bool is_repeated)
{
  tab **p;
  for (p = is_repeated ? &repeated_list : &initial_list; *p;
       p = &(*p)->next)
    ;
  *p = new tab(pos, type);
}


void tab_stops::operator=(const tab_stops &ts)
{
  clear();
  tab **p = &initial_list;
  tab *t = ts.initial_list;
  while (t) {
    *p = new tab(t->pos, t->type);
    t = t->next;
    p = &(*p)->next;
  }
  p = &repeated_list;
  t = ts.repeated_list;
  while (t) {
    *p = new tab(t->pos, t->type);
    t = t->next;
    p = &(*p)->next;
  }
}

static void configure_tab_stops_request()
{
  hunits pos;
  hunits prev_pos = 0;
  bool is_first_stop = true;
  bool is_repeating_stop = false;
  tab_stops tabs;
  while (has_arg()) {
    if (tok.ch() == TAB_REPEAT_CHAR) {
      tok.next();
      is_repeating_stop = true;
      prev_pos = 0;
    }
    if (!read_hunits(&pos, 'm', prev_pos))
      break;
    tab_type type = TAB_LEFT;
    if (tok.ch() == int('C')) { // TODO: grochar
      tok.next();
      type = TAB_CENTER;
    }
    else if (tok.ch() == int('R')) { // TODO: grochar
      tok.next();
      type = TAB_RIGHT;
    }
    else if (tok.ch() == int('L')) { // TODO: grochar
      tok.next();
    }
    if (pos <= prev_pos && ((!is_first_stop) || is_repeating_stop))
      warning(WARN_RANGE,
	      "positions of tab stops must be strictly increasing");
    else {
      tabs.add_tab(pos, type, is_repeating_stop);
      prev_pos = pos;
      is_first_stop = false;
    }
  }
  curenv->tabs = tabs;
  curdiv->modified_tag.incl(MTSM_TA);
  skip_line();
}

const char *environment::get_tabs()
{
  return tabs.to_string();
}

tab_type environment::distance_to_next_tab(hunits *distance)
{
  return using_line_tabs
    ? curenv->tabs.distance_to_next_tab(get_text_length(), distance)
    : curenv->tabs.distance_to_next_tab(get_input_line_position(), distance);
}

tab_type environment::distance_to_next_tab(hunits *distance, hunits *leftpos)
{
  return using_line_tabs
    ? curenv->tabs.distance_to_next_tab(get_text_length(), distance, leftpos)
    : curenv->tabs.distance_to_next_tab(get_input_line_position(), distance,
					leftpos);
}

// XXX: Field characters are global; shouldn't they be environmental?
static void field_characters_request()
{
  field_delimiter_char = read_character();
  if (field_delimiter_char)
    padding_indicator_char = read_character();
  else
    padding_indicator_char = 0 /* nullptr */;
  skip_line();
}

void line_tabs_request()
{
  int n;
  if (has_arg() && read_integer(&n))
    curenv->using_line_tabs = (n > 0);
  else
    curenv->using_line_tabs = true;
  skip_line();
}

int environment::is_using_line_tabs()
{
  return using_line_tabs;
}

void environment::wrap_up_tab()
{
  if (current_tab == TAB_NONE)
    return;
  if (line == 0)
    start_line();
  hunits tab_amount;
  switch (current_tab) {
  case TAB_RIGHT:
    tab_amount = tab_distance - tab_width;
    line = make_tab_node(tab_amount, line);
    break;
  case TAB_CENTER:
    tab_amount = tab_distance - tab_width/2;
    line = make_tab_node(tab_amount, line);
    break;
  case TAB_NONE:
  case TAB_LEFT:
  default:
    assert(0 == "unhandled case of `current_tab` (tab_type)");
  }
  width_total += tab_amount;
  width_total += tab_width;
  if (has_current_field) {
    if (tab_precedes_field) {
      pre_field_width += tab_amount;
      tab_precedes_field = false;
    }
    field_distance -= tab_amount;
    field_spaces += tab_field_spaces;
  }
  if (tab_contents != 0 /* nullptr */) {
    node *tem;
    for (tem = tab_contents; tem->next != 0 /* nullptr */;
	 tem = tem->next)
      ;
    tem->next = line;
    line = tab_contents;
  }
  tab_field_spaces = 0;
  tab_contents = 0 /* nullptr */;
  tab_width = H0;
  tab_distance = H0;
  current_tab = TAB_NONE;
}

node *environment::make_tab_node(hunits d, node *next)
{
  if ((leader_node != 0 /* nullptr */) && (d < 0)) {
    error("motion generated by leader cannot be negative");
    delete leader_node;
    leader_node = 0 /* nullptr */;
  }
  if (0 /* nullptr */ == leader_node)
    return new hmotion_node(d, 1, 0, get_fill_color(), next);
  node *nd = new hline_node(d, leader_node, next);
  leader_node = 0 /* nullptr */;
  return nd;
}

void environment::advance_to_tab_stop(bool use_leader)
{
  hunits d;
  hunits absolute;
  if (current_tab != TAB_NONE)
    wrap_up_tab();
  charinfo *ci = use_leader ? leader_char : tab_char;
  delete leader_node;
  leader_node = 0 /* nullptr */;
  if (ci != 0 /* nullptr */)
    leader_node = make_char_node(ci);
  tab_type t = distance_to_next_tab(&d, &absolute);
  switch (t) {
  case TAB_NONE:
    return;
  case TAB_LEFT:
    add_node(make_tag("tab L", absolute.to_units()));
    add_node(make_tab_node(d));
    return;
  case TAB_RIGHT:
    add_node(make_tag("tab R", absolute.to_units()));
    break;
  case TAB_CENTER:
    add_node(make_tag("tab C", absolute.to_units()));
    break;
  default:
    assert(0 == "unhandled case of `t` (tab_type)");
  }
  tab_width = 0;
  tab_distance = d;
  tab_contents = 0 /* nullptr */;
  current_tab = t;
  tab_field_spaces = 0;
}

void environment::start_field()
{
  assert(!has_current_field);
  hunits d;
  if (distance_to_next_tab(&d) != TAB_NONE) {
    pre_field_width = get_text_length();
    field_distance = d;
    has_current_field = true;
    field_spaces = 0;
    tab_field_spaces = 0;
    for (node *p = line; p != 0 /* nullptr */; p = p->next)
      if (p->nspaces()) {
	p->freeze_space();
	space_total--;
      }
    tab_precedes_field = current_tab != TAB_NONE;
  }
  else
    error("zero field width");
}

void environment::wrap_up_field()
{
  if ((current_tab == TAB_NONE) && field_spaces == 0)
    add_padding();
  hunits padding = field_distance - (get_text_length() - pre_field_width);
  if ((current_tab != TAB_NONE) && tab_field_spaces != 0) {
    hunits tab_padding = scale(padding,
			       tab_field_spaces,
			       field_spaces + tab_field_spaces);
    padding -= tab_padding;
    (void) distribute_space(tab_contents, tab_field_spaces, tab_padding,
			    true /* force reversal of node list */);
    tab_field_spaces = 0;
    tab_width += tab_padding;
  }
  if (field_spaces != 0) {
    (void) distribute_space(line, field_spaces, padding,
			    true /* force reversal of node list */);
    width_total += padding;
    if (current_tab != TAB_NONE) {
      // the start of the tab has been moved to the right by padding, so
      tab_distance -= padding;
      if (tab_distance <= H0) {
	// use the next tab stop instead
	current_tab = tabs.distance_to_next_tab(get_input_line_position()
						- tab_width,
						&tab_distance);
	if (current_tab == TAB_NONE || current_tab == TAB_LEFT) {
	  width_total += tab_width;
	  if (current_tab == TAB_LEFT) {
	    line = make_tab_node(tab_distance, line);
	    width_total += tab_distance;
	    current_tab = TAB_NONE;
	  }
	  if (tab_contents != 0 /* nullptr */) {
	    node *tem;
	    for (tem = tab_contents; tem->next != 0 /* nullptr */;
		 tem = tem->next)
	      ;
	    tem->next = line;
	    line = tab_contents;
	    tab_contents = 0 /* nullptr */;
	  }
	  tab_width = H0;
	  tab_distance = H0;
	}
      }
    }
  }
  has_current_field = false;
}

void environment::add_padding()
{
  if (current_tab != TAB_NONE) {
    tab_contents = new space_node(H0, get_fill_color(), tab_contents);
    tab_field_spaces++;
  }
  else {
    if (line == 0 /* nullptr */)
      start_line();
    line = new space_node(H0, get_fill_color(), line);
    field_spaces++;
  }
}

typedef int (environment::*INT_FUNCP)();
typedef unsigned int (environment::*UNSIGNED_FUNCP)();
typedef vunits (environment::*VUNITS_FUNCP)();
typedef hunits (environment::*HUNITS_FUNCP)();
typedef const char *(environment::*STRING_FUNCP)();

class int_env_reg : public reg {
  INT_FUNCP func;
 public:
  int_env_reg(INT_FUNCP);
  const char *get_string();
  bool get_value(units *val);
};

class unsigned_env_reg : public reg {
  UNSIGNED_FUNCP func;
 public:
  unsigned_env_reg(UNSIGNED_FUNCP);
  const char *get_string();
  bool get_value(unsigned int *val);
};

class vunits_env_reg : public reg {
  VUNITS_FUNCP func;
 public:
  vunits_env_reg(VUNITS_FUNCP f);
  const char *get_string();
  bool get_value(units *val);
};

class hunits_env_reg : public reg {
  HUNITS_FUNCP func;
 public:
  hunits_env_reg(HUNITS_FUNCP f);
  const char *get_string();
  bool get_value(units *val);
};

class string_env_reg : public reg {
  STRING_FUNCP func;
public:
  string_env_reg(STRING_FUNCP);
  const char *get_string();
};

int_env_reg::int_env_reg(INT_FUNCP f) : func(f)
{
}

bool int_env_reg::get_value(units *val)
{
  *val = (curenv->*func)();
  return true;
}

const char *int_env_reg::get_string()
{
  return i_to_a((curenv->*func)());
}

unsigned_env_reg::unsigned_env_reg(UNSIGNED_FUNCP f) : func(f)
{
}

bool unsigned_env_reg::get_value(unsigned int *val)
{
  *val = (curenv->*func)();
  return true;
}

const char *unsigned_env_reg::get_string()
{
  return ui_to_a((curenv->*func)());
}

vunits_env_reg::vunits_env_reg(VUNITS_FUNCP f) : func(f)
{
}

bool vunits_env_reg::get_value(units *val)
{
  *val = (curenv->*func)().to_units();
  return true;
}

const char *vunits_env_reg::get_string()
{
  return i_to_a((curenv->*func)().to_units());
}

hunits_env_reg::hunits_env_reg(HUNITS_FUNCP f) : func(f)
{
}

bool hunits_env_reg::get_value(units *val)
{
  *val = (curenv->*func)().to_units();
  return true;
}

const char *hunits_env_reg::get_string()
{
  return i_to_a((curenv->*func)().to_units());
}

string_env_reg::string_env_reg(STRING_FUNCP f) : func(f)
{
}

const char *string_env_reg::get_string()
{
  return (curenv->*func)();
}

class horizontal_place_reg : public general_reg {
public:
  horizontal_place_reg();
  bool get_value(units *);
  void set_value(units);
};

horizontal_place_reg::horizontal_place_reg()
{
}

bool horizontal_place_reg::get_value(units *res)
{
  *res = curenv->get_input_line_position().to_units();
  return true;
}

void horizontal_place_reg::set_value(units n)
{
  curenv->set_input_line_position(hunits(n));
}

int environment::get_zoom()
{
  return env_get_zoom(this);
}

int environment::get_numbering_nodes()
{
  return (curenv->numbering_nodes ? 1 : 0);
}

const char *environment::get_font_family_string()
{
  return family->nm.contents();
}

const char *environment::get_stroke_color_string()
{
  return stroke_color->nm.contents();
}

const char *environment::get_fill_color_string()
{
  return fill_color->nm.contents();
}

const char *environment::get_prev_stroke_color_string()
{
  return prev_stroke_color->nm.contents();
}

const char *environment::get_prev_fill_color_string()
{
  return prev_fill_color->nm.contents();
}

const char *environment::get_font_name_string()
{
  symbol f = get_font_name(fontno, this);
  return f.contents();
}

const char *environment::get_style_name_string()
{
  symbol f = get_style_name(fontno);
  return f.contents();
}

const char *environment::get_name_string()
{
  return name.contents();
}

// Convert a quantity in scaled points to ascii decimal fraction.

const char *sptoa(int sp)
{
  assert(sp > 0);
  assert(sizescale > 0);
  if (sizescale == 1)
    return i_to_a(sp);
  if (sp % sizescale == 0)
    return i_to_a(sp/sizescale);
  // See if 1/sizescale is exactly representable as a decimal fraction,
  // ie its only prime factors are 2 and 5.
  int n = sizescale;
  int power2 = 0;
  while ((n & 1) == 0) {
    n >>= 1;
    power2++;
  }
  int power5 = 0;
  while ((n % 5) == 0) {
    n /= 5;
    power5++;
  }
  if (n == 1) {
    int decimal_point = power5 > power2 ? power5 : power2;
    if (decimal_point <= 10) {
      int factor = 1;
      int t;
      for (t = decimal_point - power2; --t >= 0;)
	factor *= 2;
      for (t = decimal_point - power5; --t >= 0;)
	factor *= 5;
      if (factor == 1 || sp <= INT_MAX/factor)
	return if_to_a(sp*factor, decimal_point);
    }
  }
  double s = double(sp)/double(sizescale);
  double factor = 10.0;
  double val = s;
  int decimal_point = 0;
  do {
    double v = ceil(s*factor);
    if (v > INT_MAX)
      break;
    val = v;
    factor *= 10.0;
  } while (++decimal_point < 10);
  return if_to_a(int(val), decimal_point);
}

const char *environment::get_point_size_string()
{
  return sptoa(curenv->get_point_size());
}

const char *environment::get_requested_point_size_string()
{
  return sptoa(curenv->get_requested_point_size());
}

void environment::dump()
{
  // At the time this request is invoked, the following values are zero
  // or meaningless.
  //
  //   char_height, char_slant,
  //   was_line_interrupted
  //   current_tab, tab_width, tab_distance
  //   has_current_field, field_distance, pre_field_width, field_spaces,
  //     tab_field_spaces, tab_precedes_field
  //   composite
  //
  if (!in_nroff_mode) {
    errprint("  previous type size: %1p (%2s)\n",
	     prev_size.to_points(), prev_size.to_scaled_points());
    errprint("  type size: %1p (%2s)\n",
	     size.to_points(), size.to_scaled_points());
    errprint("  previous requested type size: %1s\n",
	     prev_requested_size);
    errprint("  requested type size: %1s\n", requested_size);
    font_size::dump_size_list();
    errprint("  previous default family: '%1'\n",
	     prev_family->nm.contents());
    errprint("  default family: '%1'\n", family->nm.contents());
  }
  errprint("  previous resolved font selection: %1 ('%2')\n",
	   prev_fontno, get_font_name(prev_fontno, this).contents());
  errprint("  resolved font selection: %1 ('%2')\n", fontno,
	   get_font_name(fontno, this).contents());
  errprint("  space size: %1/12 of font space width\n", space_size);
  errprint("  sentence space size: %1/12 of font space width\n",
	   sentence_space_size);
  errprint("  previous line length: %1u\n",
	   prev_line_length.to_units());
  errprint("  line length: %1u\n", line_length.to_units());
  errprint("  previous title line length: %1u\n",
	   prev_title_length.to_units());
  errprint("  title line length: %1u\n", title_length.to_units());
  errprint("  previous line interrupted/continued: %1\n",
	   was_previous_line_interrupted ? "yes" : "no");
  errprint("  filling: %1\n", is_filling ? "on" : "off");
  errprint("  alignment/adjustment: %1\n",
	   adjust_mode == ADJUST_LEFT
	     ? "left"
	     : adjust_mode == ADJUST_BOTH
		 ? "both"
		 : adjust_mode == ADJUST_CENTER
		     ? "center"
		     : "right");
  if (centered_line_count > 0)
    errprint("  lines remaining to center: %1\n", centered_line_count);
  if (right_aligned_line_count > 0)
    errprint("  lines remaining to right-align: %1\n",
	     right_aligned_line_count);
  errprint("  previous vertical spacing: %1u\n",
	   prev_vertical_spacing.to_units());
  errprint("  vertical spacing: %1u\n", vertical_spacing.to_units());
  errprint("  previous post-vertical spacing: %1u\n",
	   prev_post_vertical_spacing.to_units());
  errprint("  post-vertical spacing: %1u\n",
	   post_vertical_spacing.to_units());
  errprint("  previous line spacing: %1\n", prev_line_spacing);
  errprint("  line spacing: %1\n", line_spacing);
  errprint("  previous indentation: %1u\n", prev_indent.to_units());
  errprint("  indentation: %1u\n", indent.to_units());
  errprint("  temporary indentation: %1u\n",
	   temporary_indent.to_units());
  errprint("  temporary indentation pending: %1\n",
	   have_temporary_indent ? "yes" : "no");
  errprint("  total indentation: %1u\n", saved_indent.to_units());
  if (underlined_line_count > 0) {
    errprint("  lines remaining to underline: %1\n",
	     underlined_line_count);
    errprint("  font number before underlining: %1\n",
	     pre_underline_fontno);
    errprint("  underlining spaces: %1\n",
	     underline_spaces ? "yes" : "no");
  }
  if (input_trap.contents() != 0 /* nullptr */) {
    errprint("  input trap macro: '%1'\n", input_trap.contents());
    errprint("  lines remaining until input trap springs: %1\n",
	     input_trap_count);
    errprint("  input trap respects output line continuation: %1\n",
	     continued_input_trap ? "yes" : "no");
  }
  errprint("  previous text length: %1u\n",
	   prev_text_length.to_units());
  if (line != 0 /* nullptr */) {
    errprint("  text length: %1u\n", width_total.to_units());
    errprint("  number of adjustable spaces: %1\n", space_total);
  }
  errprint("  target text length: %1u\n",
	   target_text_length.to_units());
  errprint("  input line start: %1u\n", input_line_start.to_units());
  errprint("  computing tab stops from: %1\n",
	   using_line_tabs ? "output line start (\"line tabs\")"
	     : "input line start");
  errprint("  forcing adjustment: %1\n", is_spreading ? "yes" : "no");
  if (margin_character_node != 0 /* nullptr */) {
    errprint("  margin character flags: %1\n",
	     margin_character_flags == MC_ON
	       ? "on"
	       : margin_character_flags == MC_NEXT
		   ? "next"
		   : margin_character_flags == (MC_ON | MC_NEXT)
		       ? "on, next"
		       : "none");
    errprint("  margin character distance: %1u\n",
	     margin_character_distance.to_units());
  }
  if (numbering_nodes != 0 /* nullptr */) {
    errprint("  line number digit width: %1u\n",
	     line_number_digit_width.to_units());
    errprint("  separation between number and text: %1 digit spaces\n",
	     number_text_separation);
    errprint("  line number indentation: %1 digit spaces\n",
	     line_number_indent);
    errprint("  numbering every %1 line%2\n",
	     line_number_multiple > 1
	       ? i_to_a(line_number_multiple) : "",
	     line_number_multiple > 1 ? "s" : "");
    errprint("  lines remaining for which to suppress numbering: %1\n",
	     no_number_count);
  }
  string hf = hyphenation_mode ? "on" : "off";
  if (hyphenation_mode & HYPHEN_NOT_LAST_LINE)
    hf += ", not on line before vertical position trap";
  if (hyphenation_mode & HYPHEN_LAST_CHAR)
    hf += ", allowed before last character";
  if (hyphenation_mode & HYPHEN_NOT_LAST_CHARS)
    hf += ", not allowed within last two characters";
  if (hyphenation_mode & HYPHEN_FIRST_CHAR)
    hf += ", allowed after first character";
  if (hyphenation_mode & HYPHEN_NOT_FIRST_CHARS)
    hf += ", not allowed within first two characters";
  hf += '\0';
  errprint("  hyphenation mode: %1 (%2)\n", hyphenation_mode,
	   hf.contents());
  errprint("  hyphenation mode default: %1\n",
	   hyphenation_mode_default);
  errprint("  count of consecutive hyphenated lines: %1\n",
	   hyphen_line_count);
  errprint("  consecutive hyphenated line count limit: %1%2\n",
	   hyphen_line_max, hyphen_line_max < 0 ? " (unlimited)" : "");
  errprint("  hyphenation space: %1u\n", hyphenation_space.to_units());
  errprint("  hyphenation margin: %1u\n",
	   hyphenation_margin.to_units());
#ifdef WIDOW_CONTROL
  errprint("  widow control: %1\n", want_widow_control ? "yes" : "no");
#endif /* WIDOW_CONTROL */
  errprint("  previous stroke color: %1\n",
	   get_prev_stroke_color_string());
  errprint("  stroke color: %1\n", get_stroke_color_string());
  errprint("  previous fill color: %1\n",
	   get_prev_fill_color_string());
  errprint("  fill color: %1\n", get_fill_color_string());
  fflush(stderr);
}

void print_environment_request()
{
  errprint("Current Environment:\n");
  curenv->dump();
  dictionary_iterator iter(env_dictionary);
  symbol s;
  environment *e;
  while (iter.get(&s, (void **)&e)) {
    assert(!s.is_null());
    errprint("Environment %1:\n", s.contents());
    if (e != curenv)
      e->dump();
    else
      errprint("  current\n");
  }
  fflush(stderr);
  skip_line();
}

static void print_pending_output_line_request()
{
  curenv->dump_pending_nodes();
  skip_line();
}

// Hyphenation - TeX's hyphenation algorithm with a less fancy
// implementation.

struct trie_node;

class trie {
  trie_node *tp;
  virtual void do_match(int, void *) = 0;
  virtual void do_delete(void *) = 0;
  void delete_trie_node(trie_node *);
public:
  trie() : tp(0 /* nullptr */) {}
  virtual ~trie();		// virtual to shut up g++
  void insert(const char *, int, void *);
  // find calls do_match for each match it finds
  void find(const char *, int);
  void clear();
};

class hyphen_trie : private trie {
  int *h;
  void do_match(int i, void *v);
  void do_delete(void *v);
  void insert_pattern(const char *, int, int *);
  void insert_hyphenation(dictionary *, const char *, int);
  int hpf_getc(FILE *f);
public:
  hyphen_trie() {}
  ~hyphen_trie() {}
  void hyphenate(const char *, int, int *);
  void read_patterns_file(const char *, int, dictionary *);
};

struct hyphenation_language {
  symbol name;
  dictionary exceptions;
  hyphen_trie patterns;
  hyphenation_language(symbol nm) : name(nm), exceptions(501) {}
  ~hyphenation_language() { }
};

dictionary language_dictionary(5);
hyphenation_language *current_language = 0 /* nullptr */;

static void select_hyphenation_language()
{
  if (!has_arg()) {
    current_language = 0 /* nullptr */;
    skip_line();
    return;
  }
  symbol nm = read_identifier();
  if (!nm.is_null()) {
    current_language = static_cast<hyphenation_language *>
      (language_dictionary.lookup(nm));
    if (0 /* nullptr */ == current_language) {
      current_language = new hyphenation_language(nm);
      (void) language_dictionary.lookup(nm,
	static_cast<hyphenation_language *>(current_language));
    }
  }
  skip_line();
}

void environment_copy()
{
  if (!has_arg()) {
    warning(WARN_MISSING, "environment copy request expects an"
	    " argument");
    skip_line();
    return;
  }
  environment *e = 0 /* nullptr */;
  tok.skip_spaces();
  symbol nm = read_long_identifier();
  assert(nm != 0 /* nullptr */);
  e = static_cast<environment *>(env_dictionary.lookup(nm));
  if (e != 0 /* nullptr */)
    curenv->copy(e);
  else
    error("cannot copy from nonexistent environment '%1'",
	  nm.contents());
  skip_line();
}

void environment_switch()
{
  if (curenv->is_dummy()) {
    error("cannot switch out of dummy environment");
  }
  else {
    symbol nm = read_long_identifier();
    if (nm.is_null()) {
      if (env_stack == 0 /* nullptr */)
	error("environment stack underflow");
      else {
	bool seen_space = curenv->seen_space;
	bool seen_eol   = curenv->seen_eol;
	bool suppress_next_eol = curenv->suppress_next_eol;
	curenv = env_stack->env;
	curenv->seen_space = seen_space;
	curenv->seen_eol   = seen_eol;
	curenv->suppress_next_eol = suppress_next_eol;
	env_list_node *tem = env_stack;
	env_stack = env_stack->next;
	delete tem;
      }
    }
    else {
      environment *e
	= static_cast<environment *>(env_dictionary.lookup(nm));
      if (!e) {
	e = new environment(nm);
	(void) env_dictionary.lookup(nm, e);
      }
      env_stack = new env_list_node(curenv, env_stack);
      curenv = e;
    }
  }
  skip_line();
}

const int WORD_MAX = 256;	// we use unsigned char for offsets in
				// hyphenation exceptions

static void add_hyphenation_exception_words_request()
{
  if (!has_arg()) {
    warning(WARN_MISSING, "hyphenation exception word request expects"
	    " one or more arguments");
    skip_line();
    return;
  }
  if (0 /* nullptr */ == current_language) {
    error("cannot add hyphenation exception words when no hyphenation"
	  " language is selected");
    skip_line();
    return;
  }
  char buf[WORD_MAX + 1];
  unsigned char pos[WORD_MAX + 2];
  for (;;) {
    if (!has_arg())
      break;
    int i = 0;
    int npos = 0;
    while ((i < WORD_MAX)
	   && !tok.is_space()
	   && !tok.is_newline()
	   && !tok.is_eof()) {
      charinfo *ci = tok.get_charinfo(true /* required */);
      if (0 /* nullptr */ == ci) {
	assert(0 == "attempted to use token without charinfo in"
	       " hyphenation exception word");
	skip_line();
	return;
      }
      tok.next();
      if (ci->get_ascii_code() == '-') {
	if (i > 0 && (npos == 0 || pos[npos - 1] != i))
	  pos[npos++] = i;
      }
      else {
	unsigned char c = ci->get_hyphenation_code();
	if (0U == c)
	  break;
	buf[i++] = c;
      }
    }
    if (i > 0) {
      pos[npos] = 0;
      buf[i] = '\0';
      // C++03: new unsigned char[npos + 1]();
      unsigned char *tem = new unsigned char[npos + 1];
      (void) memset(tem, 0, ((npos + 1) * sizeof(unsigned char)));
      memcpy(tem, pos, npos + 1);
      tem = static_cast<unsigned char *>
	    (current_language->exceptions.lookup(symbol(buf), tem));
      if (tem)
	delete[] tem;
    }
  }
  skip_line();
}

static void print_hyphenation_exceptions()
{
  if (0 /* nullptr */ == current_language)
    return;
  dictionary_iterator iter(current_language->exceptions);
  symbol entry;
  unsigned char *hypoint;
  // Pathologically, we could have a hyphenation point after every
  // character in a word except the last.  The word may have a trailing
  // space; see `hyphen_trie::read_patterns_file()`.
  const size_t bufsz = WORD_MAX * 2;
  char wordbuf[bufsz]; // need to `errprint()` it, so not `unsigned`
  // We must use the nuclear `reinterpret_cast` operator because GNU
  // troff's dictionary types use a pre-STL approach to containers.
  while (iter.get(&entry, reinterpret_cast<void **>(&hypoint))) {
    assert(!entry.is_null());
    assert(hypoint != 0 /* nullptr */);
    string word = entry.contents();
    (void) memset(wordbuf, '\0', bufsz);
    size_t i = 0, j = 0, len = word.length();
    bool is_mode_dependent = false;
    while (i < len) {
      if ((hypoint != 0 /* nullptr */) && (*hypoint == i)) {
	wordbuf[j++] = '-';
	hypoint++;
      }
      if (word[i] == ' ') {
	assert(i == (len - 1));
	is_mode_dependent = true;
      }
      wordbuf[j++] = word[i++];
    }
    errprint("%1", wordbuf);
    if (is_mode_dependent)
      errprint("\t*");
    errprint("\n");
  }
  fflush(stderr);
  skip_line();
}

struct trie_node {
  char c;
  trie_node *down;
  trie_node *right;
  void *val;
  trie_node(char, trie_node *);
};

trie_node::trie_node(char ch, trie_node *p)
: c(ch), down(0), right(p), val(0)
{
}

trie::~trie()
{
  clear();
}

void trie::clear()
{
  delete_trie_node(tp);
  tp = 0;
}


void trie::delete_trie_node(trie_node *p)
{
  if (p) {
    delete_trie_node(p->down);
    delete_trie_node(p->right);
    if (p->val)
      do_delete(p->val);
    delete p;
  }
}

void trie::insert(const char *pat, int patlen, void *val)
{
  trie_node **p = &tp;
  assert(patlen > 0 && pat != 0);
  for (;;) {
    while (*p != 0 && (*p)->c < pat[0])
      p = &((*p)->right);
    if (*p == 0 || (*p)->c != pat[0])
      *p = new trie_node(pat[0], *p);
    if (--patlen == 0) {
      (*p)->val = val;
      break;
    }
    ++pat;
    p = &((*p)->down);
  }
}

void trie::find(const char *pat, int patlen)
{
  trie_node *p = tp;
  for (int i = 0; p != 0 && i < patlen; i++) {
    while (p != 0 && p->c < pat[i])
      p = p->right;
    if (p != 0 && p->c == pat[i]) {
      if (p->val != 0)
	do_match(i+1, p->val);
      p = p->down;
    }
    else
      break;
  }
}

struct operation {
  operation *next;
  short distance;
  short num;
  operation(int, int, operation *);
};

operation::operation(int i, int j, operation *op)
: next(op), distance(j), num(i)
{
}

void hyphen_trie::insert_pattern(const char *pat, int patlen, int *num)
{
  operation *op = 0;
  for (int i = 0; i < patlen+1; i++)
    if (num[i] != 0)
      op = new operation(num[i], patlen - i, op);
  insert(pat, patlen, op);
}

void hyphen_trie::insert_hyphenation(dictionary *ex, const char *pat,
				     int patlen)
{
  char buf[WORD_MAX + 2];
  unsigned char pos[WORD_MAX + 2];
  int i = 0, j = 0;
  int npos = 0;
  while (j < patlen) {
    unsigned char c = pat[j++];
    if (c == '-') {
      if (i > 0 && (npos == 0 || pos[npos - 1] != i))
	pos[npos++] = i;
    }
    else if (c == ' ')
      buf[i++] = ' ';
    else
      buf[i++] = hpf_code_table[c];
  }
  if (i > 0) {
    pos[npos] = 0;
    buf[i] = '\0';
    // C++03: new unsigned char[npos + 1]();
    unsigned char *tem = new unsigned char[npos + 1];
    (void) memset(tem, 0, ((npos + 1) * sizeof(unsigned char)));
    memcpy(tem, pos, npos + 1);
    tem = static_cast<unsigned char *>(ex->lookup(symbol(buf), tem));
    if (0 /* nullptr */ == tem)
      delete[] tem;
  }
}

void hyphen_trie::hyphenate(const char *word, int len, int *hyphens)
{
  int j;
  for (j = 0; j < len + 1; j++)
    hyphens[j] = 0;
  for (j = 0; j < len - 1; j++) {
    h = hyphens + j;
    find(word + j, len - j);
  }
}

inline int max(int m, int n)
{
  return m > n ? m : n;
}

void hyphen_trie::do_match(int i, void *v)
{
  operation *op = static_cast<operation *>(v);
  while (op != 0) {
    h[i - op->distance] = max(h[i - op->distance], op->num);
    op = op->next;
  }
}

void hyphen_trie::do_delete(void *v)
{
  operation *op = static_cast<operation *>(v);
  while (op) {
    operation *tem = op;
    op = tem->next;
    delete tem;
  }
}

/* We use very simple rules to parse TeX's hyphenation patterns.

   . '%' starts a comment even if preceded by '\'.

   . No support for digraphs and like '\$'.

   . '^^xx' ('x' is 0-9 or a-f), and '^^x' (character code of 'x' in the
     range 0-127) are recognized; other use of '^' causes an error.

   . No macro expansion.

   . We check for the expression '\patterns{...}' (possibly with
     whitespace before and after the braces).  Everything between the
     braces is taken as hyphenation patterns.  Consequently, '{' and '}'
     are not allowed in patterns.

   . Similarly, '\hyphenation{...}' gives a list of hyphenation
     exceptions.

   . '\endinput' is recognized also.

   . For backward compatibility, if '\patterns' is missing, the
     whole file is treated as a list of hyphenation patterns (only
     recognizing '%' as the start of a comment.

*/

int hyphen_trie::hpf_getc(FILE *f)
{
  int c = getc(f);
  int c1;
  int cc = 0;
  if (c != '^')
    return c;
  c = getc(f);
  if (c != '^')
    goto fail;
  c = getc(f);
  c1 = getc(f);
  if (((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'))
      && ((c1 >= '0' && c1 <= '9') || (c1 >= 'a' && c1 <= 'f'))) {
    if (c >= '0' && c <= '9')
      c -= '0';
    else
      c = c - 'a' + 10;
    if (c1 >= '0' && c1 <= '9')
      c1 -= '0';
    else
      c1 = c1 - 'a' + 10;
    cc = c * 16 + c1;
  }
  else {
    ungetc(c1, f);
    if (c >= 0 && c <= 63)
      cc = c + 64;
    else if (c >= 64 && c <= 127)
      cc = c - 64;
    else
      goto fail;
  }
  return cc;
fail:
  error("invalid ^, ^^x, or ^^xx character in hyphenation patterns file");
  return c;
}

void hyphen_trie::read_patterns_file(const char *name, int append,
				     dictionary *ex)
{
  if (!append)
    clear();
  char buf[WORD_MAX + 1];
  for (int i = 0; i < WORD_MAX + 1; i++)
    buf[i] = 0;
  int num[WORD_MAX + 1];
  errno = 0;
  char *path = 0;
  FILE *fp = mac_path->open_file(name, &path);
  if (0 /* nullptr */ == fp) {
    error("cannot open hyphenation pattern file '%1': %2", name,
	  strerror(errno));
    return;
  }
  int c = hpf_getc(fp);
  bool have_patterns = false;		// seen \patterns
  bool is_final_pattern = false;	// have a trailing closing brace
  bool have_hyphenation = false;	// seen \hyphenation
  bool is_final_hyphenation = false;	// have a trailing closing brace
  bool have_keyword = false;		// seen \patterns or \hyphenation
  bool is_traditional = false;		// don't handle \patterns
  for (;;) {
    for (;;) {
      if (c == '%') {		// skip comments
	do {
	  c = getc(fp);
	} while (c != EOF && c != '\n');
      }
      if (c == EOF || !csspace(c))
	break;
      c = hpf_getc(fp);
    }
    if (c == EOF) {
      if (have_keyword || is_traditional)	// we are done
	break;
      else {			// rescan file in 'is_traditional' mode
	rewind(fp);
	is_traditional = true;
	c = hpf_getc(fp);
	continue;
      }
    }
    int i = 0;
    num[0] = 0;
    if (!(c == '{' || c == '}')) {	// skip braces at line start
      do {				// scan patterns
	if (csdigit(c))
	  num[i] = c - '0';
	else {
	  buf[i++] = c;
	  num[i] = 0;
	}
	c = hpf_getc(fp);
      } while (i < WORD_MAX && c != EOF && !csspace(c)
	       && c != '%' && c != '{' && c != '}');
    }
    if (!is_traditional) {
      if (i >= 9 && !strncmp(buf + i - 9, "\\patterns", 9)) {
	while (csspace(c))
	  c = hpf_getc(fp);
	if (c == '{') {
	  if (have_patterns || have_hyphenation)
	    error("\\patterns is not allowed inside of %1 group",
		  have_patterns ? "\\patterns" : "\\hyphenation");
	  else {
	    have_patterns = true;
	    have_keyword = true;
	  }
	  c = hpf_getc(fp);
	  continue;
	}
      }
      else if (i >= 12 && !strncmp(buf + i - 12, "\\hyphenation", 12)) {
	while (csspace(c))
	  c = hpf_getc(fp);
	if (c == '{') {
	  if (have_patterns || have_hyphenation)
	    error("\\hyphenation is not allowed inside of %1 group",
		  have_patterns ? "\\patterns" : "\\hyphenation");
	  else {
	    have_hyphenation = true;
	    have_keyword = true;
	  }
	  c = hpf_getc(fp);
	  continue;
	}
      }
      else if (strstr(buf, "\\endinput")) {
	if (have_patterns || have_hyphenation)
	  error("found \\endinput inside of %1 group",
		have_patterns ? "\\patterns" : "\\hyphenation");
	break;
      }
      else if (c == '}') {
	if (have_patterns) {
	  have_patterns = false;
	  if (i > 0)
	    is_final_pattern = true;
	}
	else if (have_hyphenation) {
	  have_hyphenation = false;
	  if (i > 0)
	    is_final_hyphenation = true;
	}
	c = hpf_getc(fp);
      }
      else if (c == '{') {
	if (have_patterns || have_hyphenation)
	  error("'{' is not allowed within %1 group",
		have_patterns ? "\\patterns" : "\\hyphenation");
	c = hpf_getc(fp);		// skipped if not starting
					// \patterns or \hyphenation
      }
    }
    else {
      if (c == '{' || c == '}')
	c = hpf_getc(fp);
    }
    if (i > 0) {
      if (have_patterns || is_final_pattern || is_traditional) {
	for (int j = 0; j < i; j++)
	  buf[j] = hpf_code_table[static_cast<unsigned char>(buf[j])];
	insert_pattern(buf, i, num);
	is_final_pattern = false;
      }
      else if (have_hyphenation || is_final_hyphenation) {
	// hyphenation exceptions in a pattern file are subject to `.hy'
	// restrictions; we mark such entries with a trailing space
	buf[i++] = ' ';
	insert_hyphenation(ex, buf, i);
	is_final_hyphenation = false;
      }
    }
  }
  fclose(fp);
  free(path);
  return;
}

class hyphenation_language_reg : public reg {
public:
  const char *get_string();
};

const char *hyphenation_language_reg::get_string()
{
  return (current_language != 0 /* nullptr */)
	  ? current_language->name.contents() : "";
}

class hyphenation_default_mode_reg : public reg {
public:
  const char *get_string();
};

const char *hyphenation_default_mode_reg::get_string()
{
  return i_to_a(curenv->get_hyphenation_mode_default());
}

#define init_int_env_reg(name, func) \
  register_dictionary.define(name, new int_env_reg(&environment::func))

#define init_unsigned_env_reg(name, func) \
  register_dictionary.define(name, new unsigned_env_reg(&environment::func))

#define init_vunits_env_reg(name, func) \
  register_dictionary.define(name, new vunits_env_reg(&environment::func))

#define init_hunits_env_reg(name, func) \
  register_dictionary.define(name, new hunits_env_reg(&environment::func))

#define init_string_env_reg(name, func) \
  register_dictionary.define(name, new string_env_reg(&environment::func))

// Most hyphenation functionality is environment-specific; see
// init_hyphenation_pattern_requests() below for globally managed state.
void init_env_requests()
{
  init_request("ad", adjust);
  init_request("br", break_without_forced_adjustment_request);
  init_request("brp", break_with_forced_adjustment_request);
  init_request("ce", center);
  init_request("cu", continuous_underline);
  init_request("ev", environment_switch);
  init_request("evc", environment_copy);
  init_request("fam", family_change);
  init_request("fc", field_characters_request);
  init_request("fi", fill);
  init_request("fcolor", select_fill_color_request);
  init_request("ft", select_font_request);
  init_request("gcolor", select_stroke_color_request);
  init_request("hc", hyphenation_character_request);
  init_request("hla", select_hyphenation_language);
  init_request("hlm", hyphen_line_max_request);
  init_request("hy", hyphenate_request);
  init_request("hydefault", set_hyphenation_mode_default);
  init_request("hym", hyphenation_margin_request);
  init_request("hys", hyphenation_space_request);
  init_request("in", indent);
  init_request("it", input_trap);
  init_request("itc", input_trap_continued);
  init_request("lc", leader_character_request);
  init_request("linetabs", line_tabs_request);
  init_request("ll", line_length);
  init_request("ls", line_spacing);
  init_request("lt", title_length);
  init_request("mc", margin_character);
  init_request("na", no_adjust);
  init_request("nf", no_fill);
  init_request("nh", no_hyphenate);
  init_request("nm", number_lines);
  init_request("nn", no_number);
  init_request("pev", print_environment_request);
  init_request("pline", print_pending_output_line_request);
  init_request("ps", point_size);
  init_request("pvs", post_vertical_spacing);
  init_request("rj", right_justify);
  init_request("sizes", override_available_type_sizes_request);
  init_request("ss", space_size);
  init_request("ta", configure_tab_stops_request);
  init_request("ti", temporary_indent);
  init_request("tc", tab_character_request);
  init_request("tl", title);
  init_request("ul", underline);
  init_request("vs", vertical_spacing);
#ifdef WIDOW_CONTROL
  init_request("wdc", widow_control_request);
#endif /* WIDOW_CONTROL */
  init_hunits_env_reg(".b", get_emboldening_offset);
  init_vunits_env_reg(".cdp", get_prev_char_depth);
  init_int_env_reg(".ce", get_centered_line_count);
  init_vunits_env_reg(".cht", get_prev_char_height);
  init_hunits_env_reg(".csk", get_prev_char_skew);
  init_string_env_reg(".ev", get_name_string);
  init_int_env_reg(".f", get_font);
  init_string_env_reg(".fam", get_font_family_string);
  init_string_env_reg(".fn", get_font_name_string);
  init_int_env_reg(".height", get_char_height);
  register_dictionary.define(".hla", new hyphenation_language_reg);
  init_int_env_reg(".hlc", get_hyphen_line_count);
  init_int_env_reg(".hlm", get_hyphen_line_max);
  init_unsigned_env_reg(".hy", get_hyphenation_mode);
  init_unsigned_env_reg(".hydefault", get_hyphenation_mode_default);
  init_hunits_env_reg(".hym", get_hyphenation_margin);
  init_hunits_env_reg(".hys", get_hyphenation_space);
  init_hunits_env_reg(".i", get_indent);
  init_hunits_env_reg(".in", get_saved_indent);
  init_int_env_reg(".int", get_was_previous_line_interrupted);
  init_int_env_reg(".it", get_input_trap_line_count);
  init_int_env_reg(".itc", get_input_trap_respects_continuation);
  init_string_env_reg(".itm", get_input_trap_macro);
  init_int_env_reg(".linetabs", is_using_line_tabs);
  init_hunits_env_reg(".lt", get_title_length);
  init_unsigned_env_reg(".j", get_adjust_mode);
  init_hunits_env_reg(".k", get_text_length);
  init_int_env_reg(".L", get_line_spacing);
  init_hunits_env_reg(".l", get_line_length);
  init_hunits_env_reg(".ll", get_saved_line_length);
  init_string_env_reg(".M", get_fill_color_string);
  init_string_env_reg(".m", get_stroke_color_string);
  init_hunits_env_reg(".n", get_prev_text_length);
  init_int_env_reg(".nm", get_numbering_nodes);
  init_int_env_reg(".nn", get_no_number_count);
  init_int_env_reg(".ps", get_point_size);
  init_int_env_reg(".psr", get_requested_point_size);
  init_vunits_env_reg(".pvs", get_post_vertical_spacing);
  init_int_env_reg(".rj", get_right_aligned_line_count);
  init_string_env_reg(".s", get_point_size_string);
  init_int_env_reg(".slant", get_char_slant);
  init_int_env_reg(".ss", get_space_size);
  init_int_env_reg(".sss", get_sentence_space_size);
  init_string_env_reg(".sr", get_requested_point_size_string);
  init_string_env_reg(".sty", get_style_name_string);
  init_string_env_reg(".tabs", get_tabs);
  init_int_env_reg(".u", get_fill);
  init_vunits_env_reg(".v", get_vertical_spacing);
  init_hunits_env_reg(".w", get_prev_char_width);
  init_int_env_reg(".zoom", get_zoom);
  register_dictionary.define("ct", new variable_reg(&ct_reg_contents));
  register_dictionary.define("hp", new horizontal_place_reg);
  register_dictionary.define("ln", new variable_reg(&next_line_number));
  register_dictionary.define("rsb", new variable_reg(&rsb_reg_contents));
  register_dictionary.define("rst", new variable_reg(&rst_reg_contents));
  register_dictionary.define("sb", new variable_reg(&sb_reg_contents));
  register_dictionary.define("skw", new variable_reg(&skw_reg_contents));
  register_dictionary.define("ssc", new variable_reg(&ssc_reg_contents));
  register_dictionary.define("st", new variable_reg(&st_reg_contents));
  // TODO: Kill the following off in groff 1.24.0 release + 2 years.
  deprecated_font_identifiers.push_back("AX");
  deprecated_font_identifiers.push_back("KR");
  deprecated_font_identifiers.push_back("KI");
  deprecated_font_identifiers.push_back("KB");
  deprecated_font_identifiers.push_back("KX");
  deprecated_font_identifiers.push_back("CW");
  deprecated_font_identifiers.push_back("C");
  deprecated_font_identifiers.push_back("CO");
  deprecated_font_identifiers.push_back("CX");
  deprecated_font_identifiers.push_back("H");
  deprecated_font_identifiers.push_back("HO");
  deprecated_font_identifiers.push_back("HX");
  deprecated_font_identifiers.push_back("Hr");
  deprecated_font_identifiers.push_back("Hi");
  deprecated_font_identifiers.push_back("Hb");
  deprecated_font_identifiers.push_back("Hx");
  deprecated_font_identifiers.push_back("PA");
  deprecated_font_identifiers.push_back("PX");
  deprecated_font_identifiers.push_back("NX");
  deprecated_font_identifiers.push_back("ZI");
}

// Appendix H of _The TeXbook_ is useful background for the following.

static void hyphenate(hyphen_list *h, unsigned int flags)
{
  if (0 /* nullptr */ == current_language)
    return;
  while (h != 0 /* nullptr */) {
    while ((h != 0 /* nullptr */) && (0U == h->hyphenation_code))
      h = h->next;
    int len = 0;
    // Locate hyphenable points within a (subset of) an input word.
    //
    // We first look up the word in the environment's hyphenation
    // exceptions dictionary; its keys are C strings, so the buffer
    // `hbuf` that holds our word needs to be null terminated and we
    // allocate a byte for that.  If the lookup fails, we apply the
    // hyphenation patterns, which require that the word be bracketed at
    // each end with a dot ('.'), so we allocate two further bytes.
    //
    // `hbuf` can be thought of as a mapping of the letters of the input
    // word to the hyphenation codes that correspond to each letter.
    // The hyphenation codes are normalized; "AbBoT" becomes "abbot".
    //
    // We can hyphenate words longer than WORD_MAX, but WORD_MAX is the
    // maximum size of the "window" inside a word within which we apply
    // the hyphenation patterns to determine a break point.
    char hbuf[WORD_MAX + 2 + 1];
    (void) memset(hbuf, '\0', sizeof hbuf);
    char *bufp = hbuf + 1;
    hyphen_list *tem;
    for (tem = h; tem && len < WORD_MAX; tem = tem->next) {
      if (tem->hyphenation_code != 0U)
	bufp[len++] = tem->hyphenation_code;
      else
	break;
    }
    hyphen_list *nexth = tem;
    if (len >= 2) {
      // Check hyphenation exceptions defined with `hw` request.
      assert((bufp + len) < (hbuf + sizeof hbuf));
      bufp[len] = '\0';
      unsigned char *pos = static_cast<unsigned char *>(
			   current_language->exceptions.lookup(bufp));
      if (pos != 0 /* nullptr */) {
	int j = 0;
	int i = 1;
	for (tem = h; tem != 0 /* nullptr */; tem = tem->next, i++)
	  if (pos[j] == i) {
	    tem->is_hyphen = true;
	    j++;
	  }
      }
      else {
	// Check `\hyphenation' entries from pattern files; such entries
	// are marked with a trailing space.
	assert((hbuf + len + 1) < (hbuf + sizeof hbuf));
	bufp[len] = ' ';
	bufp[len + 1] = '\0';
	pos = static_cast<unsigned char *>(
	      current_language->exceptions.lookup(bufp));
	if (pos != 0 /* nullptr */) {
	  int j = 0;
	  int i = 1;
	  tem = h;
	  if (pos[j] == i) {
	    if (flags & HYPHEN_FIRST_CHAR)
	      tem->is_hyphen = true;
	    j++;
	  }
	  tem = tem->next;
	  i++;
	  if (pos[j] == i) {
	    if (!(flags & HYPHEN_NOT_FIRST_CHARS))
	      tem->is_hyphen = true;
	    j++;
	  }
	  tem = tem->next;
	  i++;
	  if (!(flags & HYPHEN_LAST_CHAR))
	    --len;
	  if (flags & HYPHEN_NOT_LAST_CHARS)
	    --len;
	  for (; i < len && tem; tem = tem->next, i++)
	    if (pos[j] == i) {
	      tem->is_hyphen = true;
	      j++;
	    }
	}
	else {
	  hbuf[0] = hbuf[len + 1] = '.';
	  int num[WORD_MAX + 2 + 1];
	  (void) memset(num, 0, sizeof num);
	  current_language->patterns.hyphenate(hbuf, len + 2, num);
	  // The position of a hyphenation point gets marked with an odd
	  // number.  Example:
	  //
	  //   hbuf:  . h e l p f u l .
	  //   num:  0 0 0 2 4 3 0 0 0 0
	  if (!(flags & HYPHEN_FIRST_CHAR))
	    num[2] = 0;
	  if (flags & HYPHEN_NOT_FIRST_CHARS)
	    num[3] = 0;
	  if (flags & HYPHEN_LAST_CHAR)
	    ++len;
	  if (flags & HYPHEN_NOT_LAST_CHARS)
	    --len;
	  int i;
	  for (i = 2, tem = h; i < len && tem; tem = tem->next, i++)
	    if (num[i] & 1)
	      tem->is_hyphen = true;
	}
      }
    }
    h = nexth;
  }
}

static void read_hyphenation_patterns_from_file(bool append)
{
  char *filename = read_rest_of_line_as_argument();
  if (filename != 0 /* nullptr */) {
    if (0 /* nullptr */ == current_language)
      error("no current hyphenation language");
    else
      current_language->patterns.read_patterns_file(filename, append,
	&current_language->exceptions);
  }
  tok.next();
}

static void load_hyphenation_patterns_from_file()
{
  if (!has_arg(true /* peek */)) {
    warning(WARN_MISSING, "hyphenation pattern load request expects"
	    " argument");
    skip_line();
    return;
  }
  read_hyphenation_patterns_from_file(false /* append */);
  // No `skip_line()` here; the above function calls
  // `read_rest_of_line_as_argument()` and `tok.next()`.
}

static void append_hyphenation_patterns_from_file()
{
  if (!has_arg(true /* peek */)) {
    warning(WARN_MISSING, "hyphenation pattern appendment request"
	    " expects argument");
    skip_line();
    return;
  }
  read_hyphenation_patterns_from_file(true /* append */);
  // No `skip_line()` here; the above function calls
  // `read_rest_of_line_as_argument()` and `tok.next()`.
}

// Most hyphenation functionality is environment-specific; see
// init_env_requests() above.
void init_hyphenation_pattern_requests()
{
  init_request("hpf", load_hyphenation_patterns_from_file);
  init_request("hpfa", append_hyphenation_patterns_from_file);
  init_request("hw", add_hyphenation_exception_words_request);
  init_request("phw", print_hyphenation_exceptions);
}

// Local Variables:
// fill-column: 72
// mode: C++
// End:
// vim: set cindent noexpandtab shiftwidth=2 textwidth=72:

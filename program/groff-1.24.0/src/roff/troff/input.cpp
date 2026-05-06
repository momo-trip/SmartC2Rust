/* Copyright 1989-2024 Free Software Foundation, Inc.
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
#include <errno.h> // ENOENT, errno
#include <locale.h> // setlocale()
#include <stdcountof.h>
#include <stdio.h> // EOF, FILE, clearerr(), fclose(), fflush(),
		   // fileno(), fopen(), fprintf(), fseek(), getc(),
		   // pclose(), popen(), printf(), SEEK_SET, snprintf(),
		   // sprintf(), setbuf(), stderr, stdin, stdout,
		   // ungetc()
#include <stdlib.h> // atoi(), exit(), EXIT_FAILURE, EXIT_SUCCESS,
		    // free(), getenv(), putenv(), strtol(), system()
#include <string.h> // strcpy(), strdup(), strerror()

#include <getopt.h> // getopt_long()

#include <stack>

#include "json-encode.h" // json_encode_char()

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
#include "defs.h"
#include "unicode.h"
#include "curtime.h"

// needed for getpid() and isatty()
#include "posix.h"
#include "nonposix.h"

#define MACRO_PREFIX "tmac."
#define MACRO_POSTFIX ".tmac"
#define INITIAL_STARTUP_FILE "troffrc"
#define FINAL_STARTUP_FILE   "troffrc-end"
#define DEFAULT_INPUT_STACK_LIMIT 1000

#ifndef DEFAULT_WARNING_MASK
// warnings that are enabled by default
#define DEFAULT_WARNING_MASK \
     (WARN_CHAR|WARN_BREAK|WARN_SPACE|WARN_FONT|WARN_FILE)
#endif

extern "C" const char *program_name;
extern "C" const char *Version_string;

// initial size for input buffers that need to grow arbitrarily
static const int default_buffer_size = 16;

#ifdef COLUMN
void init_column_requests();
#endif /* COLUMN */

// forward declarations
static node *read_drawing_command();
static void read_drawing_command_color_arguments(token &);
static void push_token(const token &);
static void unsafe_transparent_throughput_file_request();
#ifdef COLUMN
void vjustify();
#endif /* COLUMN */
static void transparent_throughput_file_request();

token tok;
bool was_invoked_with_regular_control_character = false;
bool using_character_classes = false;
static bool permit_color_output = true;
bool want_color_output = true;
static bool want_backtraces = false;
char *pipe_command = 0 /* nullptr */;
charinfo *charset_table[256];
unsigned char hpf_code_table[256];

static unsigned int warning_mask = DEFAULT_WARNING_MASK;
static bool want_errors_inhibited = false;
static bool want_input_ignored = false;

static void enable_warning(const char *);
static void disable_warning(const char *);

static symbol end_of_input_macro_name;
static symbol blank_line_macro_name;
static symbol leading_spaces_macro_name;
static bool want_att_compat = false;
bool want_abstract_output = false;
bool want_nodes_dumped = false;
bool want_output_suppressed = false;
bool is_writing_html = false;
static int suppression_level = 0;	// depth of nested \O escapes

bool in_nroff_mode = false;
bool is_device_ps_or_pdf = false;

// Keep track of whether \f, \F, \D'F...', \H, \m, \M, \O[345], \R, \s,
// or \S has been processed in token::next().
static bool have_formattable_input = false;
// `have_formattable_input` is reset immediately upon reading a new
// input line, but we need more state information because the input line
// might have been continued/interrupted with `\c`.
// Consider:
//   \f[TB]\m[red]hello\c
//   \f[]\m[]
static bool have_formattable_input_on_interrupted_line = false;

bool device_has_tcommand = false;	// 't' output command supported
static bool want_unsafe_requests = false;	// be safer by default

static bool have_multiple_params = false;	// \[e aa], \*[foo bar]

double spread_limit = -3.0 - 1.0;	// negative means deactivated

double warn_scale;
char warn_scaling_unit;
bool want_html_debugging = true;	// enable more diagnostics

search_path *mac_path = &safer_macro_path;

// Initialize inclusion search path with only the current directory.
search_path include_search_path(0 /* nullptr */, 0 /* nullptr */, 0, 1);

static int read_char_in_copy_mode(node ** /* nd; 0 to discard */,
				  bool /* is_defining */ = false,
				  bool /* handle_escaped_E */ = false);
static void copy_mode_error(const char *,
			    const errarg & = empty_errarg,
			    const errarg & = empty_errarg,
			    const errarg & = empty_errarg);

enum read_mode { ALLOW_EMPTY, WITH_ARGS, NO_ARGS };
static symbol read_escape_parameter(read_mode = NO_ARGS);
static symbol read_long_escape_parameters(read_mode = NO_ARGS);
static void interpolate_string(symbol);
static void interpolate_string_with_args(symbol);
static void interpolate_macro(symbol, bool = false);
static void interpolate_number_format(symbol);
static void interpolate_environment_variable(symbol);

static symbol composite_glyph_name(symbol);
static void interpolate_positional_parameter(symbol);
static request_or_macro *lookup_request(symbol);
static bool read_delimited_measurement(units * /* n */,
    unsigned char /* si */);
static bool read_delimited_measurement(units * /* n */,
    unsigned char /* si */, units /* prev_value */);
static symbol read_input_until_terminator(bool /* required */,
    unsigned char /* end_char */, bool /* want_identifier */ = false);
static bool read_line_rule_expression(units * /* res */,
    unsigned char /* si */, charinfo ** /* cp */);
static bool read_size(int *);
static symbol read_delimited_identifier();
static void init_registers();
static void trapping_blank_line();

class input_iterator;
input_iterator *make_temp_iterator(const char *);
const char *input_char_description(int);

void process_input_stack();
void chop_macro();	// declare to avoid friend name injection

static const unsigned char default_escape_char = (unsigned char)('\\');
static unsigned char escape_char = default_escape_char;
static const unsigned char default_control_char = (unsigned char)('.');
static const unsigned char default_no_break_control_char
  = (unsigned char)('\'');

static void assign_escape_character_request()
{
  unsigned char ec = 0U;
  bool is_invalid = false;
  if (has_arg()) {
    if (tok.ch() == 0U)
      is_invalid = true;
    else
      ec = tok.ch();
  }
  else
    ec = default_escape_char;
  bool do_nothing = false;
  static const char already_cc[] = "the control character is already";
  static const char already_nbcc[] = "the no-break control character is"
				     " already";
  const char *already_message = 0 /* nullptr */;
  if (curenv->get_control_character() == ec) {
      already_message = already_cc;
      do_nothing = true;
  }
  else if (curenv->get_no_break_control_character() == ec) {
      already_message = already_nbcc;
      do_nothing = true;
  }
  if (do_nothing)
    error("ignoring escape character change request; %1%2 %3",
	  is_invalid ? "cannot select invalid escape character, and"
	  : "", already_message, input_char_description(ec));
  else if (is_invalid) {
    error("cannot select %1 as escape character; using '%2'",
	  tok.description(), char(default_escape_char));
    escape_char = default_escape_char;
  }
  else
    escape_char = ec;
  skip_line();
}

void escape_off_request()
{
  escape_char = 0U;
  skip_line();
}

static unsigned char saved_escape_char = '\\';

void save_escape_char_request()
{
  saved_escape_char = escape_char;
  skip_line();
}

void restore_escape_char_request()
{
  escape_char = saved_escape_char;
  skip_line();
}

void assign_control_character_request()
{
  unsigned char cc = 0U;
  bool is_invalid = false;
  if (has_arg()) {
    if (tok.ch() == 0U)
      is_invalid = true;
    else
      cc = tok.ch();
  }
  else
    cc = default_control_char;
  bool do_nothing = false;
  char already_ec[] = "the escape character is already";
  char already_nbcc[] = "the no-break control character is already";
  char *already_message = 0 /* nullptr */;
  if (cc == escape_char) {
      already_message = already_ec;
      do_nothing = true;
  }
  else if (curenv->get_no_break_control_character() == cc) {
      already_message = already_nbcc;
      do_nothing = true;
  }
  bool assignment_worked = false;
  if (do_nothing)
    error("ignoring control character change request; %1%2 %3",
	  is_invalid ? "cannot select invalid control character, and"
	  : "", already_message, input_char_description(cc));
  else if (is_invalid) {
    error("cannot select %1 as control character; using '%2'",
	  tok.description(), char(default_control_char));
    assignment_worked
      = curenv->set_control_character(default_control_char);
  }
  else
    assignment_worked = curenv->set_control_character(cc);
  assert(assignment_worked);
  skip_line();
}

void assign_no_break_control_character_request()
{
  unsigned char nbcc = 0U;
  bool is_invalid = false;
  if (has_arg()) {
    if (tok.ch() == 0U)
      is_invalid = true;
    else
      nbcc = tok.ch();
  }
  else
    nbcc = default_no_break_control_char;
  bool do_nothing = false;
  char already_ec[] = "the escape character is already";
  char already_cc[] = "the (breaking) control character is already";
  char *already_message = 0 /* nullptr */;
  if (nbcc == escape_char) {
      already_message = already_ec;
      do_nothing = true;
  }
  else if (curenv->get_control_character() == nbcc) {
      already_message = already_cc;
      do_nothing = true;
  }
  bool assignment_worked = false;
  if (do_nothing)
    error("ignoring no-break control character change request; %1%2 %3",
	  is_invalid ? "cannot select invalid no-break control"
	               " character, and"
	  : "", already_message, input_char_description(nbcc));
  else if (is_invalid) {
    error("cannot select %1 as no-break control character;"
	  " using \"%2\"", tok.description(),
	  default_no_break_control_char);
    assignment_worked
      = curenv->set_no_break_control_character(default_no_break_control_char);
  }
  else
    assignment_worked = curenv->set_no_break_control_character(nbcc);
  assert(assignment_worked);
  skip_line();
}

struct arg_list;

class input_iterator {
public:
  input_iterator();
  input_iterator(bool /* is_div */);
  virtual ~input_iterator() {}
  int get(node **);
  friend class input_stack;
  bool is_diversion;
  statem *diversion_state;
protected:
  const unsigned char *ptr;
  const unsigned char *endptr;
  input_iterator *next;
private:
  virtual int fill(node **);
  virtual int peek();
  virtual bool has_args() { return false; }
  virtual int nargs() { return 0; }
  virtual input_iterator *get_arg(int) { return 0 /* nullptr */; }
  virtual arg_list *get_arg_list() { return 0 /* nullptr */; }
  virtual symbol get_macro_name() { return NULL_SYMBOL; }
  virtual bool space_follows_arg(int) { return false; }
  virtual bool get_break_flag() { return false; }
  virtual bool get_location(bool /* allow_macro */,
			    const char ** /* filep */,
			    int * /* linep */) { return false; }
  virtual void backtrace() {}
  virtual bool set_location(const char *, int) { return false; }
  virtual bool next_file(FILE *, const char *) { return false; }
  virtual void shift(int) {}
  virtual int is_boundary() {return 0; } // three-valued Boolean :-|
  virtual bool is_file() { return false; }
  virtual bool is_macro() { return false; }
  virtual void set_att_compat(bool) {}
  virtual bool get_att_compat() { return false; }
};

input_iterator::input_iterator()
: is_diversion(false), ptr(0 /* nullptr */), endptr(0 /* nullptr */)
{
}

input_iterator::input_iterator(bool is_div)
: is_diversion(is_div), ptr(0 /* nullptr */), endptr(0 /* nullptr */)
{
}

int input_iterator::fill(node **)
{
  return EOF;
}

int input_iterator::peek()
{
  return EOF;
}

inline int input_iterator::get(node **p)
{
  return ptr < endptr ? *ptr++ : fill(p);
}

class input_boundary : public input_iterator {
public:
  int is_boundary() { return 1; }
};

class input_return_boundary : public input_iterator {
public:
  int is_boundary() { return 2; }
};

class file_iterator : public input_iterator {
  FILE *fp;
  int lineno;
  char *filename;
  bool was_popened;
  bool seen_newline;
  bool seen_escape;
  enum { BUF_SIZE = 512 };
  unsigned char buf[BUF_SIZE];
  void close();
public:
  file_iterator(FILE *, const char *, bool = false);
  ~file_iterator();
  int fill(node **);
  int peek();
  bool get_location(bool /* allow_macro */, const char ** /* filep */,
		    int * /* linep */);
  void backtrace();
  bool set_location(const char *, int);
  bool next_file(FILE *, const char *);
  bool is_file() { return true; }
};

file_iterator::file_iterator(FILE *f, const char *fn, bool popened)
: fp(f), lineno(1), was_popened(popened),
  seen_newline(false), seen_escape(false)
{
  filename = strdup(const_cast<char *>(fn));
  if ((font::use_charnames_in_special) && (fn != 0 /* nullptr */)) {
    if (!the_output)
      init_output();
    the_output->put_filename(fn, popened);
  }
}

file_iterator::~file_iterator()
{
  close();
}

void file_iterator::close()
{
  if (fp == stdin)
    clearerr(stdin);
  else if (was_popened)
    pclose(fp);
  else
    fclose(fp);
}

bool file_iterator::next_file(FILE *f, const char *s)
{
  close();
  fp = f;
  set_location(s, 1);
  seen_newline = false;
  seen_escape = false;
  was_popened = false;
  ptr = 0 /* nullptr */;
  endptr = 0 /* nullptr */;
  return true;
}

int file_iterator::fill(node **)
{
  if (seen_newline)
    lineno++;
  seen_newline = false;
  unsigned char *p = buf;
  ptr = p;
  unsigned char *e = p + BUF_SIZE;
  while (p < e) {
    int c = getc(fp);
    if (EOF == c)
      break;
    if (is_invalid_input_char(c))
      warning(WARN_INPUT, "invalid input character code %1", c);
    else {
      *p++ = c;
      if ('\n' == c) {
	seen_escape = false;
	seen_newline = true;
	break;
      }
      seen_escape = ('\\' == c); // XXX: should be (escape_char == c)?
    }
  }
  if (p > buf) {
    endptr = p;
    return *ptr++;
  }
  else {
    endptr = p;
    return EOF;
  }
}

int file_iterator::peek()
{
  int c = getc(fp);
  while (is_invalid_input_char(c)) {
    warning(WARN_INPUT, "invalid input character code %1", c);
    c = getc(fp);
  }
  if (c != EOF)
    ungetc(c, fp);
  return c;
}

bool file_iterator::get_location(bool /*allow_macro*/,
				 const char **filenamep, int *linenop)
{
  *linenop = lineno;
  assert(filename != 0 /* nullptr */);
  if (0 /* nullptr */ == filename)
    return false;
  if (strcmp(filename, "-") == 0)
    *filenamep = "<standard input>";
  else
    *filenamep = filename;
  return true;
}

void file_iterator::backtrace()
{
  const char *f;
  int n;
  // Get side effect of filename rewrite if stdin.
  (void) get_location(false /* allow macro */, &f, &n);
  if (program_name != 0 /* nullptr */)
    errprint("%1: ", program_name);
  errprint("backtrace: %3 '%1':%2\n", f, n,
	   was_popened ? "pipe" : "file");
}

bool file_iterator::set_location(const char *f, int ln)
{
  if (f != 0 /* nullptr */)
    filename = const_cast<char *>(f);
  lineno = ln;
  return true;
}

input_iterator nil_iterator;

class input_stack {
public:
  static int get(node **);
  static int peek();
  static void push(input_iterator *);
  static input_iterator *get_arg(int);
  static arg_list *get_arg_list();
  static symbol get_macro_name();
  static bool space_follows_arg(int);
  static int get_break_flag();
  static int nargs();
  static bool get_location(bool /* allow_macro */,
			   const char ** /* filep */,
			   int * /* linep */);
  static bool set_location(const char *, int);
  static void backtrace();
  static void next_file(FILE *, const char *);
  static void end_file();
  static void shift(int n);
  static void add_boundary();
  static void add_return_boundary();
  static int is_return_boundary();
  static void remove_boundary();
  static int get_level();
  static int get_div_level();
  static void increase_level();
  static void decrease_level();
  static void clear();
  static void pop_macro();
  static void set_att_compat(bool);
  static bool get_att_compat();
  static statem *get_diversion_state();
  static void check_end_diversion(input_iterator *t);
  static int limit;
  static int div_level;
  static statem *diversion_state;
private:
  static input_iterator *top;
  static int level;
  static int finish_get(node **);
  static int finish_peek();
};

input_iterator *input_stack::top = &nil_iterator;
int input_stack::level = 0;
int input_stack::limit = DEFAULT_INPUT_STACK_LIMIT;
int input_stack::div_level = 0;
statem *input_stack::diversion_state = 0 /* nullptr */;
bool suppress_push = false;


inline int input_stack::get_level()
{
  return level;
}

inline void input_stack::increase_level()
{
  level++;
}

inline void input_stack::decrease_level()
{
  level--;
}

inline int input_stack::get_div_level()
{
  return div_level;
}

inline int input_stack::get(node **np)
{
  int res = (top->ptr < top->endptr) ? *top->ptr++ : finish_get(np);
  if (res == '\n') {
    have_formattable_input_on_interrupted_line = have_formattable_input;
    have_formattable_input = false;
  }
  return res;
}

int input_stack::finish_get(node **np)
{
  for (;;) {
    int c = top->fill(np);
    if (c != EOF || top->is_boundary())
      return c;
    if (top == &nil_iterator)
      break;
    input_iterator *tem = top;
    check_end_diversion(tem);
#if defined(DEBUGGING)
    if (want_html_debugging)
      if (tem->is_diversion)
	fprintf(stderr,
		"in diversion level = %d\n", input_stack::get_div_level());
#endif
    top = top->next;
    level--;
    delete tem;
    if (top->ptr < top->endptr)
      return *top->ptr++;
  }
  assert(level == 0);
  return EOF;
}

inline int input_stack::peek()
{
  return (top->ptr < top->endptr) ? *top->ptr : finish_peek();
}

void input_stack::check_end_diversion(input_iterator *t)
{
  if (t->is_diversion) {
    div_level--;
    if (diversion_state != 0 /* nullptr */)
      delete diversion_state;
    diversion_state = t->diversion_state;
  }
}

int input_stack::finish_peek()
{
  for (;;) {
    int c = top->peek();
    if (c != EOF || top->is_boundary())
      return c;
    if (top == &nil_iterator)
      break;
    input_iterator *tem = top;
    check_end_diversion(tem);
    top = top->next;
    level--;
    delete tem;
    if (top->ptr < top->endptr)
      return *top->ptr;
  }
  assert(level == 0);
  return EOF;
}

void input_stack::add_boundary()
{
  push(new input_boundary);
}

void input_stack::add_return_boundary()
{
  push(new input_return_boundary);
}

int input_stack::is_return_boundary()
{
  return top->is_boundary() == 2;
}

void input_stack::remove_boundary()
{
  assert(top->is_boundary());
  input_iterator *temp = top->next;
  check_end_diversion(top);

  delete top;
  top = temp;
  level--;
}

void input_stack::push(input_iterator *in)
{
  if (in == 0)
    return;
  if (++level > limit && limit > 0)
    fatal("input stack limit of %1 levels exceeded"
	  " (probable infinite loop)", limit);
  in->next = top;
  top = in;
  if (top->is_diversion) {
    div_level++;
    in->diversion_state = diversion_state;
    diversion_state = curenv->construct_state(false);
#if defined(DEBUGGING)
    if (want_html_debugging) {
      curenv->dump_troff_state();
      fflush(stderr);
    }
#endif
  }
#if defined(DEBUGGING)
  if (want_html_debugging)
    if (top->is_diversion) {
      fprintf(stderr,
	      "in diversion level = %d\n", input_stack::get_div_level());
      fflush(stderr);
    }
#endif
}

statem *get_diversion_state()
{
  return input_stack::get_diversion_state();
}

statem *input_stack::get_diversion_state()
{
  if (0 /* nullptr */ == diversion_state)
    return 0 /* nullptr */;
  else
    return new statem(diversion_state);
}

input_iterator *input_stack::get_arg(int i)
{
  input_iterator *p;
  for (p = top; p != 0 /* nullptr */; p = p->next)
    if (p->has_args())
      return p->get_arg(i);
  return 0 /* nullptr */;
}

arg_list *input_stack::get_arg_list()
{
  input_iterator *p;
  for (p = top; p != 0 /* nullptr */; p = p->next)
    if (p->has_args())
      return p->get_arg_list();
  return 0 /* nullptr */;
}

symbol input_stack::get_macro_name()
{
  input_iterator *p;
  for (p = top; p != 0 /* nullptr */; p = p->next)
    if (p->has_args())
      return p->get_macro_name();
  return NULL_SYMBOL;
}

bool input_stack::space_follows_arg(int i)
{
  input_iterator *p;
  for (p = top; p != 0 /* nullptr */; p = p->next)
    if (p->has_args())
      return p->space_follows_arg(i);
  return false;
}

int input_stack::get_break_flag()
{
  return top->get_break_flag();
}

void input_stack::shift(int n)
{
  for (input_iterator *p = top; p != 0 /* nullptr */; p = p->next)
    if (p->has_args()) {
      p->shift(n);
      return;
    }
}

int input_stack::nargs()
{
  for (input_iterator *p =top; p != 0 /* nullptr */; p = p->next)
    if (p->has_args())
      return p->nargs();
  return 0;
}

bool input_stack::get_location(bool allow_macro, const char **filenamep,
			      int *linenop)
{
  for (input_iterator *p = top; p != 0 /* nullptr */; p = p->next)
    if (p->get_location(allow_macro, filenamep, linenop))
      return true;
  return false;
}

void input_stack::backtrace()
{
  for (input_iterator *p = top; p != 0 /* nullptr */; p = p->next)
    p->backtrace();
}

bool input_stack::set_location(const char *filename, int lineno)
{
  for (input_iterator *p = top; p != 0 /* nullptr */; p = p->next)
    if (p->set_location(filename, lineno))
      return true;
  return false;
}

void input_stack::next_file(FILE *fp, const char *s)
{
  input_iterator **pp;
  for (pp = &top; *pp != &nil_iterator; pp = &(*pp)->next)
    if ((*pp)->next_file(fp, s))
      return;
  if (++level > limit && limit > 0)
    fatal("input stack limit of %1 levels exceeded", limit);
  *pp = new file_iterator(fp, s);
  (*pp)->next = &nil_iterator;
}

void input_stack::end_file()
{
  for (input_iterator **pp = &top;
       *pp != &nil_iterator;
       pp = &(*pp)->next)
    if ((*pp)->is_file()) {
      input_iterator *tem = *pp;
      check_end_diversion(tem);
      *pp = (*pp)->next;
      delete tem;
      level--;
      return;
    }
}

void input_stack::clear()
{
  int nboundaries = 0;
  while (top != &nil_iterator) {
    if (top->is_boundary())
      nboundaries++;
    input_iterator *tem = top;
    check_end_diversion(tem);
    top = top->next;
    level--;
    delete tem;
  }
  // Keep while_request happy.
  for (; nboundaries > 0; --nboundaries)
    add_return_boundary();
}

void input_stack::pop_macro()
{
  int nboundaries = 0;
  bool is_macro = false;
  do {
    if (top->next == &nil_iterator)
      break;
    if (top->is_boundary())
      nboundaries++;
    is_macro = top->is_macro();
    input_iterator *tem = top;
    check_end_diversion(tem);
    top = top->next;
    level--;
    delete tem;
  } while (!is_macro);
  // Keep while_request happy.
  for (; nboundaries > 0; --nboundaries)
    add_return_boundary();
}

inline void input_stack::set_att_compat(bool b)
{
  top->set_att_compat(b);
}

inline bool input_stack::get_att_compat()
{
  return top->get_att_compat();
}

void backtrace_request()
{
  input_stack::backtrace();
  fflush(stderr);
  skip_line();
}

void next_file()
{
  char *filename = 0 /* nullptr */;
  if (has_arg(true /* peek */)) {
    filename = read_rest_of_line_as_argument();
    tok.next();
  }
  if (0 /* nullptr */ == filename)
    input_stack::end_file();
  else {
    errno = 0;
    FILE *fp = include_search_path.open_file_cautiously(filename);
    if (0 /* nullptr */ == fp)
      error("cannot open '%1': %2", filename, strerror(errno));
    else
      input_stack::next_file(fp, filename);
  }
  // TODO: Add `filename` to file name set.
  tok.next();
}

void shift()
{
  int n;
  if (!has_arg() || !read_integer(&n))
    n = 1;
  input_stack::shift(n);
  skip_line();
}

// TODO: return unsigned char (future: grochar)?  We handle EOF here.
static char read_char_in_escape_sequence_parameter(bool allow_space
						   = false)
{
  int c = read_char_in_copy_mode(0 /* nullptr */,
				 false /* is_defining */,
				 true /* handle_escaped_E  */);
  switch (c) {
  case EOF:
    copy_mode_error("end of input in escape sequence");
    return '\0';
  default:
    if (!is_invalid_input_char(c))
      break;
    // fall through
  case '\n':
    if (c == '\n')
      input_stack::push(make_temp_iterator("\n"));
    // fall through
  case ' ':
    if (c == ' ' && allow_space)
      break;
    // fall through
  case '\t':
  case '\001':
  case '\b':
    copy_mode_error("%1 is not allowed in an escape sequence argument",
		    input_char_description(c));
    return '\0';
  }
  return c;
}

static symbol read_two_char_escape_parameter()
{
  char buf[3];
  buf[0] = read_char_in_escape_sequence_parameter();
  if (buf[0] != '\0') {
    buf[1] = read_char_in_escape_sequence_parameter();
    if (buf[1] == '\0')
      buf[0] = '\0';
    else
      buf[2] = '\0';
  }
  return symbol(buf);
}

static symbol read_long_escape_parameters(read_mode mode)
{
  int start_level = input_stack::get_level();
  int buf_size = default_buffer_size;
  char *buf = 0 /* nullptr */;
  try {
    // C++03: new char[buf_size]();
    buf = new char[buf_size];
  }
  catch (const std::bad_alloc &e) {
    fatal("cannot allocate %1 bytes to read input line", buf_size);
  }
  (void) memset(buf, 0, (buf_size * sizeof(char)));
  int i = 0;
  char c;
  bool have_char = false;
  for (;;) {
    c = read_char_in_escape_sequence_parameter(have_char
					       && (WITH_ARGS == mode));
    if ('\0' == c) {
      delete[] buf;
      return NULL_SYMBOL;
    }
    have_char = true;
    if ((WITH_ARGS == mode) && (' ' == c))
      break;
    if (i + 2 > buf_size) {
      char *old_buf = buf;
      int new_buf_size = buf_size * 2;
      // C++03: new char[new_buf_size]();
      try {
	buf = new char[new_buf_size];
      }
      catch (const std::bad_alloc &e) {
	fatal("cannot allocate %1 bytes to read input line", buf_size);
      }
      (void) memset(buf, 0, (new_buf_size * sizeof(char)));
      (void) memcpy(buf, old_buf, buf_size);
      buf_size = new_buf_size;
      delete[] old_buf;
    }
    if ((']' == c) && (input_stack::get_level() == start_level))
      break;
    buf[i++] = c;
  }
  buf[i] = '\0';
  if (0 == i) {
    if (mode != ALLOW_EMPTY)
      // XXX: `.device \[]` passes through as-is but `\X \[]` doesn't,
      // landing here.  Implement almost-but-not-quite-copy-mode?
      copy_mode_error("empty escape sequence argument");
    return EMPTY_SYMBOL;
  }
  if (' ' == c)
    have_multiple_params = true;
  symbol s(buf);
  delete[] buf;
  return s;
}

static symbol read_escape_parameter(read_mode mode)
{
  char c = read_char_in_escape_sequence_parameter();
  if ('\0' == c)
    return NULL_SYMBOL;
  if ('(' == c)
    return read_two_char_escape_parameter();
  if (('[' == c) && !want_att_compat)
    return read_long_escape_parameters(mode);
  char buf[2];
  buf[0] = c;
  buf[1] = '\0';
  return symbol(buf);
}

static symbol read_increment_and_escape_parameter(int *incp)
{
  char c = read_char_in_escape_sequence_parameter();
  switch (c) {
  case 0:
    *incp = 0;
    return NULL_SYMBOL;
  case '(':
    *incp = 0;
    return read_two_char_escape_parameter();
  case '+':
    *incp = 1;
    return read_escape_parameter();
  case '-':
    *incp = -1;
    return read_escape_parameter();
  case '[':
    if (!want_att_compat) {
      *incp = 0;
      return read_long_escape_parameters();
    }
    break;
  }
  *incp = 0;
  char buf[2];
  buf[0] = c;
  buf[1] = '\0';
  return symbol(buf);
}

// In copy mode, we don't tokenize normally; characters on the input
// stream are typically read into the contents of an existing node (like
// a string or macro definition), or discarded.  A handful of escape
// sequences (\n, etc.) interpolate as they do outside of copy mode.
static int read_char_in_copy_mode(node **nd,
				  bool is_defining,
				  bool handle_escaped_E)
{
  for (;;) {
    int c = input_stack::get(nd);
    if (c == PUSH_GROFF_MODE) {
      input_stack::set_att_compat(want_att_compat);
      want_att_compat = false;
      continue;
    }
    if (c == PUSH_COMP_MODE) {
      input_stack::set_att_compat(want_att_compat);
      want_att_compat = true;
      continue;
    }
    if (c == POP_GROFFCOMP_MODE) {
      want_att_compat = input_stack::get_att_compat();
      continue;
    }
    if (c == BEGIN_QUOTE) {
      input_stack::increase_level();
      continue;
    }
    if (c == END_QUOTE) {
      input_stack::decrease_level();
      continue;
    }
    if (c == DOUBLE_QUOTE)
      continue;
    if ((c == ESCAPE_E) && handle_escaped_E)
      c = escape_char;
    if (c == ESCAPE_NEWLINE) {
      if (is_defining)
	return c;
      do {
	c = input_stack::get(nd);
      } while (c == ESCAPE_NEWLINE);
    }
    if ((c != escape_char) || (0U == escape_char))
      return c;
  again:
    c = input_stack::peek();
    switch (c) {
    case 0:
      return escape_char;
    case '"':
      (void) input_stack::get(0 /* nullptr */);
      while ((c = input_stack::get(0 /* nullptr */)) != '\n'
	     && c != EOF)
	;
      return c;
    case '#':			// Like \" but newline is ignored.
      (void) input_stack::get(0 /* nullptr */);
      while ((c = input_stack::get(0 /* nullptr */)) != '\n')
	if (c == EOF)
	  return EOF;
      break;
    case '$':
      {
	(void) input_stack::get(0 /* nullptr */);
	symbol s = read_escape_parameter();
	if (!(s.is_null() || s.is_empty()))
	  interpolate_positional_parameter(s);
	break;
      }
    case '*':
      {
	(void) input_stack::get(0 /* nullptr */);
	symbol s = read_escape_parameter(WITH_ARGS);
	if (!(s.is_null() || s.is_empty())) {
	  if (have_multiple_params) {
	    have_multiple_params = false;
	    interpolate_string_with_args(s);
	  }
	  else
	    interpolate_string(s);
	}
	break;
      }
    case 'a':
      (void) input_stack::get(0 /* nullptr */);
      return '\001';
    case 'e':
      (void) input_stack::get(0 /* nullptr */);
      return ESCAPE_e;
    case 'E':
      (void) input_stack::get(0 /* nullptr */);
      if (handle_escaped_E)
	goto again;
      return ESCAPE_E;
    case 'n':
      {
	(void) input_stack::get(0 /* nullptr */);
	int inc;
	symbol s = read_increment_and_escape_parameter(&inc);
	if (!(s.is_null() || s.is_empty()))
	  interpolate_register(s, inc);
	break;
      }
    case 'g':
      {
	(void) input_stack::get(0 /* nullptr */);
	symbol s = read_escape_parameter();
	if (!(s.is_null() || s.is_empty()))
	  interpolate_number_format(s);
	break;
      }
    case 't':
      (void) input_stack::get(0 /* nullptr */);
      return '\t';
    case 'V':
      {
	(void) input_stack::get(0 /* nullptr */);
	symbol s = read_escape_parameter();
	if (!(s.is_null() || s.is_empty()))
	  interpolate_environment_variable(s);
	break;
      }
    case '\n':
      (void) input_stack::get(0 /* nullptr */);
      if (is_defining)
	return ESCAPE_NEWLINE;
      break;
    case ' ':
      (void) input_stack::get(0 /* nullptr */);
      return ESCAPE_SPACE;
    case '~':
      (void) input_stack::get(0 /* nullptr */);
      return ESCAPE_TILDE;
    case ':':
      (void) input_stack::get(0 /* nullptr */);
      return ESCAPE_COLON;
    case '|':
      (void) input_stack::get(0 /* nullptr */);
      return ESCAPE_BAR;
    case '^':
      (void) input_stack::get(0 /* nullptr */);
      return ESCAPE_CIRCUMFLEX;
    case '{':
      (void) input_stack::get(0 /* nullptr */);
      return ESCAPE_LEFT_BRACE;
    case '}':
      (void) input_stack::get(0 /* nullptr */);
      return ESCAPE_RIGHT_BRACE;
    case '`':
      (void) input_stack::get(0 /* nullptr */);
      return ESCAPE_LEFT_QUOTE;
    case '\'':
      (void) input_stack::get(0 /* nullptr */);
      return ESCAPE_RIGHT_QUOTE;
    case '-':
      (void) input_stack::get(0 /* nullptr */);
      return ESCAPE_HYPHEN;
    case '_':
      (void) input_stack::get(0 /* nullptr */);
      return ESCAPE_UNDERSCORE;
    case 'c':
      (void) input_stack::get(0 /* nullptr */);
      return ESCAPE_c;
    case '!':
      (void) input_stack::get(0 /* nullptr */);
      return ESCAPE_BANG;
    case '?':
      (void) input_stack::get(0 /* nullptr */);
      return ESCAPE_QUESTION;
    case '&':
      (void) input_stack::get(0 /* nullptr */);
      return ESCAPE_AMPERSAND;
    case ')':
      (void) input_stack::get(0 /* nullptr */);
      return ESCAPE_RIGHT_PARENTHESIS;
    case '.':
      (void) input_stack::get(0 /* nullptr */);
      return c;
    case '%':
      (void) input_stack::get(0 /* nullptr */);
      return ESCAPE_PERCENT;
    default:
      if (c == escape_char) {
	(void) input_stack::get(0 /* nullptr */);
	return c;
      }
      else
	return escape_char;
    }
  }
}

// \a or \t
class non_interpreted_char_node : public node {
  unsigned char c;
public:
  non_interpreted_char_node(unsigned char);
  void asciify(macro *);
  node *copy();
  bool interpret(macro *);
  bool is_same_as(node *);
  const char *type();
  bool causes_tprint();
  bool is_tag();
};

bool non_interpreted_char_node::is_same_as(node *nd)
{
  return c == static_cast<non_interpreted_char_node *>(nd)->c;
}

const char *non_interpreted_char_node::type()
{
  return "non-interpreted character node";
}

bool non_interpreted_char_node::causes_tprint()
{
  return false;
}

bool non_interpreted_char_node::is_tag()
{
  return false;
}

non_interpreted_char_node::non_interpreted_char_node(unsigned char cc) : c(cc)
{
  assert(cc != 0U);
}

void non_interpreted_char_node::asciify(macro *)
{
  delete this;
}

node *non_interpreted_char_node::copy()
{
  return new non_interpreted_char_node(c);
}

bool non_interpreted_char_node::interpret(macro *mac)
{
  mac->append(c);
  return true;
}

// forward declarations
static void do_width();
static node *do_non_interpreted();
static node *do_device_extension();
static node *do_suppress(symbol nm);
static void do_register();

dictionary color_dictionary(501);

static color *lookup_color(symbol nm)
{
  assert(!nm.is_null());
  if (nm == default_symbol)
    return &default_color;
  color *c = static_cast<color *>(color_dictionary.lookup(nm));
  if (0 == c /* nullptr */)
    warning(WARN_COLOR, "color '%1' not defined", nm.contents());
  return c;
}

void do_stroke_color(symbol nm) // \m
{
  if (nm.is_null())
    return;
  if (nm.is_empty())
    curenv->set_stroke_color(curenv->get_prev_stroke_color());
  else {
    color *tem = lookup_color(nm);
    if (tem != 0 /* nullptr */)
      curenv->set_stroke_color(tem);
    else
      (void) color_dictionary.lookup(nm, new color(nm));
  }
}

void do_fill_color(symbol nm) // \M
{
  if (nm.is_null())
    return;
  if (nm.is_empty())
    curenv->set_fill_color(curenv->get_prev_fill_color());
  else {
    color *tem = lookup_color(nm);
    if (tem != 0 /* nullptr */)
      curenv->set_fill_color(tem);
    else
      (void) color_dictionary.lookup(nm, new color(nm));
  }
}

static unsigned int read_color_channel_value(const char *scheme,
					     const char *col)
{
  units val;
  if (!read_measurement(&val, (unsigned char)('f'))) { // TODO: grochar
    warning(WARN_COLOR, "%1 in %2 definition set to 0", col, scheme);
    tok.next();
    return 0;
  }
  if (val < 0) {
    warning(WARN_RANGE, "%1 cannot be negative: set to 0", col);
    return 0;
  }
  if (val > color::MAX_COLOR_VAL+1) {
    warning(WARN_RANGE, "%1 cannot be greater than 1", col);
    // we change 0x10000 to 0xffff
    return color::MAX_COLOR_VAL;
  }
  return (unsigned int)(val);
}

static color *read_rgb(unsigned char end = 0U)
{
  symbol component = read_input_until_terminator(false /* required */,
						 end);
  if (component.is_null()) {
    warning(WARN_COLOR, "missing rgb color values");
    return 0 /* nullptr */;
  }
  const char *s = component.contents();
  color *col = new color;
  if ('#' == *s) {
    if (!col->read_rgb(s)) {
      warning(WARN_COLOR, "expecting rgb color definition,"
	      " not '%1'", s);
      delete col;
      return 0 /* nullptr */;
    }
  }
  else {
    if (!end)
      input_stack::push(make_temp_iterator(" "));
    input_stack::push(make_temp_iterator(s));
    tok.next();
    unsigned int r = read_color_channel_value("rgb color",
					      "red component");
    unsigned int g = read_color_channel_value("rgb color",
					      "green component");
    unsigned int b = read_color_channel_value("rgb color",
					      "blue component");
    col->set_rgb(r, g, b);
  }
  return col;
}

static color *read_cmy(unsigned char end = 0U)
{
  symbol component = read_input_until_terminator(false /* required */,
						 end);
  if (component.is_null()) {
    warning(WARN_COLOR, "missing cmy color values");
    return 0 /* nullptr */;
  }
  const char *s = component.contents();
  color *col = new color;
  if ('#' == *s) {
    if (!col->read_cmy(s)) {
      warning(WARN_COLOR, "expecting cmy color definition,"
	      " not '%1'", s);
      delete col;
      return 0 /* nullptr */;
    }
  }
  else {
    if (!end)
      input_stack::push(make_temp_iterator(" "));
    input_stack::push(make_temp_iterator(s));
    tok.next();
    unsigned int c = read_color_channel_value("cmy color",
					      "cyan component");
    unsigned int m = read_color_channel_value("cmy color",
					      "magenta component");
    unsigned int y = read_color_channel_value("cmy color",
					      "yellow component");
    col->set_cmy(c, m, y);
  }
  return col;
}

static color *read_cmyk(unsigned char end = 0U)
{
  symbol component = read_input_until_terminator(false /* required */,
						 end);
  if (component.is_null()) {
    warning(WARN_COLOR, "missing cmyk color values");
    return 0 /* nullptr */;
  }
  const char *s = component.contents();
  color *col = new color;
  if ('#' == *s) {
    if (!col->read_cmyk(s)) {
      warning(WARN_COLOR, "expecting cmyk color definition,"
	      " not '%1'", s);
      delete col;
      return 0 /* nullptr */;
    }
  }
  else {
    if (!end)
      input_stack::push(make_temp_iterator(" "));
    input_stack::push(make_temp_iterator(s));
    tok.next();
    unsigned int c = read_color_channel_value("cmyk color",
					      "cyan component");
    unsigned int m = read_color_channel_value("cmyk color",
					      "magenta component");
    unsigned int y = read_color_channel_value("cmyk color",
					      "yellow component");
    unsigned int k = read_color_channel_value("cmyk color",
					      "black component");
    col->set_cmyk(c, m, y, k);
  }
  return col;
}

static color *read_gray(unsigned char end = 0U)
{
  symbol component = read_input_until_terminator(false /* required */,
						 end);
  if (component.is_null()) {
    warning(WARN_COLOR, "missing gray value");
    return 0 /* nullptr */;
  }
  const char *s = component.contents();
  color *col = new color;
  if ('#' == *s) {
    if (!col->read_gray(s)) {
      warning(WARN_COLOR, "expecting gray definition,"
	      " not '%1'", s);
      delete col;
      return 0 /* nullptr */;
    }
  }
  else {
    if (!end)
      input_stack::push(make_temp_iterator("\n"));
    input_stack::push(make_temp_iterator(s));
    tok.next();
    unsigned int g = read_color_channel_value("gray", "gray value");
    col->set_gray(g);
  }
  return col;
}

static void activate_color()
{
  int n;
  bool is_color_desired = false;
  if (has_arg() && read_integer(&n))
    is_color_desired = (n > 0);
  else
    is_color_desired = true;
  if (is_color_desired && !permit_color_output) {
    error("color output disabled via command line");
    is_color_desired = false;
  }
  want_color_output = is_color_desired;
  skip_line();
}

static void define_color()
{
  if (!has_arg()) {
    warning(WARN_MISSING, "color definition request expects arguments");
    skip_line();
    return;
  }
  symbol color_name = read_long_identifier();
  // Testing has_arg() should have ensured this.
  assert(color_name != 0 /* nullptr */);
  if (color_name == default_symbol) {
    warning(WARN_COLOR, "default color cannot be redefined");
    skip_line();
    return;
  }
  symbol color_space = read_long_identifier();
  if (color_space.is_null()) {
    warning(WARN_MISSING, "missing color space in color definition"
	    " request");
    skip_line();
    return;
  }
  color *col;
  if (strcmp(color_space.contents(), "rgb") == 0)
    col = read_rgb();
  else if (strcmp(color_space.contents(), "cmyk") == 0)
    col = read_cmyk();
  else if (strcmp(color_space.contents(), "gray") == 0)
    col = read_gray();
  else if (strcmp(color_space.contents(), "grey") == 0)
    col = read_gray();
  else if (strcmp(color_space.contents(), "cmy") == 0)
    col = read_cmy();
  else {
    warning(WARN_COLOR, "unknown color space '%1';"
	    " use 'rgb', 'cmyk', 'gray' or 'cmy'",
	    color_space.contents());
    skip_line();
    return;
  }
  if (col != 0 /* nullptr */) {
    col->nm = color_name;
    (void) color_dictionary.lookup(color_name, col);
  }
  skip_line();
}

static void print_color_request()
{
  symbol key;
  color *value;
  if (has_arg()) {
    do {
      key = read_identifier();
      value = static_cast<color *>(color_dictionary.lookup(key));
      if (value != 0 /* nullptr */)
	errprint("%1\t%2\n", key.contents(), value->print_color());
    } while (has_arg());
  }
  else {
    dictionary_iterator iter(color_dictionary);
    // We must use the nuclear `reinterpret_cast` operator because GNU
    // troff's dictionary types use a pre-STL approach to containers.
    while (iter.get(&key, reinterpret_cast<void **>(&value))) {
      assert(!key.is_null());
      assert(value != 0 /* nullptr */);
      errprint("%1\t%2\n", key.contents(), value->print_color());
    }
  }
  fflush(stderr);
  skip_line();
}

node *do_overstrike() // \o
{
  overstrike_node *osnode = new overstrike_node;
  int start_level = input_stack::get_level();
  token start_token;
  start_token.next();
  if (!want_att_compat && !start_token.is_usable_as_delimiter())
    warning(WARN_DELIM, "using %1 as an escape sequence delimiter"
			" is deprecated", tok.description());
  else if (want_att_compat
           && !start_token.is_usable_as_delimiter(false,
		  DELIMITER_ATT_STRING_EXPRESSION)) {
    warning(WARN_DELIM, "overstriking escape sequence"
	    " does not accept %1 as a delimiter",
	    start_token.description());
    delete osnode;
    return 0 /* nullptr */;
  }
  // TODO: groff 1.24.0 release + 2 years?
#if 0
  if (!start_token.is_usable_as_delimiter(true /* report error */)) {
    delete osnode;
    return 0 /* nullptr */;
  }
#endif
  for (;;) {
    tok.next();
    if (tok.is_newline() || tok.is_eof()) {
      // token::description() writes to static, class-wide storage, so
      // we must allocate a copy of it before issuing the next
      // diagnostic.
      char *delimdesc = strdup(start_token.description());
      warning(WARN_DELIM, "missing closing delimiter in overstrike"
	      " escape sequence; expected %1, got %2", delimdesc,
	      tok.description());
      free(delimdesc);
      break;
    }
    if (tok == start_token
	&& (want_att_compat || input_stack::get_level() == start_level))
      break;
    if (tok.is_horizontal_motion())
      osnode->overstrike(tok.nd->copy());
    else if (tok.is_unstretchable_space()) {
      node *n = new hmotion_node(curenv->get_space_width(),
				 curenv->get_fill_color());
      osnode->overstrike(n);
    }
    else {
      // TODO: In theory, we could accept spaces and horizontal motions.
      charinfo *ci = tok.get_charinfo(true /* required */);
      if (0 /* nullptr */ == ci) {
	error("%1 is not supported in an overstrike escape sequence"
	      " argument", tok.description());
	delete osnode;
	return 0 /* nullptr */;
      }
      else {
	node *n = curenv->make_char_node(ci);
	if (n != 0 /* nullptr */)
	  osnode->overstrike(n);
      }
    }
  }
  return osnode;
}

static node *do_bracket() // \b
{
  bracket_node *bracketnode = new bracket_node;
  token start_token;
  start_token.next();
  if (!want_att_compat && !start_token.is_usable_as_delimiter())
    warning(WARN_DELIM, "using %1 as an escape sequence delimiter"
			" is deprecated", tok.description());
  else if (want_att_compat
           && !start_token.is_usable_as_delimiter(false,
		  DELIMITER_ATT_STRING_EXPRESSION)) {
    warning(WARN_DELIM, "bracket-building escape sequence"
	    " does not accept %1 as a delimiter",
	    start_token.description());
    delete bracketnode;
    return 0 /* nullptr */;
  }
  // TODO: groff 1.24.0 release + 2 years?
#if 0
  if (!start_token.is_usable_as_delimiter(true /* report error */)) {
    delete bracketnode;
    return 0 /* nullptr */;
  }
#endif
  int start_level = input_stack::get_level();
  for (;;) {
    tok.next();
    if (tok.is_newline() || tok.is_eof()) {
      // token::description() writes to static, class-wide storage, so
      // we must allocate a copy of it before issuing the next
      // diagnostic.
      char *delimdesc = strdup(start_token.description());
      warning(WARN_DELIM, "missing closing delimiter in"
	      " bracket-building escape sequence; expected %1, got"
	      " %2", delimdesc, tok.description());
      free(delimdesc);
      break;
    }
    if (tok == start_token
	&& (want_att_compat || input_stack::get_level() == start_level))
      break;
    // TODO: In theory, we could accept spaces and horizontal motions.
    charinfo *ci = tok.get_charinfo(true /* required */);
    if (0 /* nullptr */ == ci) {
      error("%1 is not supported in a bracket-building escape sequence"
	    " argument", tok.description());
      delete bracketnode;
      return 0 /* nullptr */;
    }
    else {
      node *n = curenv->make_char_node(ci);
      if (n != 0 /* nullptr */)
	bracketnode->bracket(n);
    }
  }
  return bracketnode;
}

static const char *do_name_test() // \A
{
  token start_token;
  start_token.next();
  if (!want_att_compat && !start_token.is_usable_as_delimiter())
    warning(WARN_DELIM, "using %1 as an escape sequence delimiter"
			" is deprecated", tok.description());
  else if (want_att_compat
           && !start_token.is_usable_as_delimiter(false,
		  DELIMITER_ATT_STRING_EXPRESSION)) {
    warning(WARN_DELIM, "name test escape sequence"
	    " does not accept %1 as a delimiter",
	    start_token.description());
    return 0 /* nullptr */;
  }
  // TODO: groff 1.24.0 release + 2 years?
#if 0
  if (!start_token.is_usable_as_delimiter(true /* report error */))
    return 0 /* nullptr */;
#endif
  int start_level = input_stack::get_level();
  bool got_bad_char = false;
  bool got_some_char = false;
  for (;;) {
    tok.next();
    if (tok.is_newline() || tok.is_eof()) {
      // token::description() writes to static, class-wide storage, so
      // we must allocate a copy of it before issuing the next
      // diagnostic.
      char *delimdesc = strdup(start_token.description());
      warning(WARN_DELIM, "missing closing delimiter in identifier"
	      " validation escape sequence; expected %1, got %2",
	      delimdesc, tok.description());
      free(delimdesc);
      break;
    }
    if (tok == start_token
	&& (want_att_compat || input_stack::get_level() == start_level))
      break;
    if (tok.ch() == 0U)
      got_bad_char = true;
    got_some_char = true;
  }
  return (got_some_char && !got_bad_char) ? "1" : "0";
}

static const char *do_expr_test() // \B
{
  token start_token;
  start_token.next();
  if (!want_att_compat
      && !start_token.is_usable_as_delimiter(true /* report error */))
    return 0 /* nullptr */;
  else if (want_att_compat
           && !start_token.is_usable_as_delimiter(false,
		  DELIMITER_ATT_NUMERIC_EXPRESSION)) {
    warning(WARN_DELIM, "numeric expression test escape sequence"
	    " does not accept %1 as a delimiter",
	    start_token.description());
    return 0 /* nullptr */;
  }
  int start_level = input_stack::get_level();
  tok.next();
  // disable all warning and error messages temporarily
  unsigned int saved_warning_mask = warning_mask;
  bool saved_want_errors_inhibited = want_errors_inhibited;
  warning_mask = 0;
  want_errors_inhibited = true;
  int dummy;
  // TODO: grochar
  bool result = read_measurement(&dummy, (unsigned char)('u'),
				 true /* is_mandatory */);
  warning_mask = saved_warning_mask;
  want_errors_inhibited = saved_want_errors_inhibited;
  // read_measurement() has left `token` pointing at the input character
  // after the end of the expression.
  if (tok == start_token && input_stack::get_level() == start_level)
    return (result ? "1" : "0");
  // There may be garbage after the expression but before the closing
  // delimiter.  Eat it.
  for (;;) {
    if (tok.is_newline() || tok.is_eof()) {
      char *delimdesc = strdup(start_token.description());
      warning(WARN_DELIM, "missing closing delimiter in numeric"
	      " expression validation escape sequence; expected %1,"
	      " got %2", delimdesc, tok.description());
      free(delimdesc);
      break;
    }
    tok.next();
    if (tok == start_token && input_stack::get_level() == start_level)
      break;
  }
  return "0";
}

#if 0
static node *do_zero_width_output()
{
  token start_token;
  start_token.next();
  int start_level = input_stack::get_level();
  environment env(curenv);
  environment *oldenv = curenv;
  curenv = &env;
  for (;;) {
    tok.next();
    if (tok.is_newline() || tok.is_eof()) {
      error("missing closing delimiter");
      break;
    }
    if (tok == start_token
	&& (want_att_compat || input_stack::get_level() == start_level))
      break;
    tok.process();
  }
  curenv = oldenv;
  node *rev = env.extract_output_line();
  node *n = 0 /* nullptr */;
  while (rev != 0 /* nullptr */) {
    node *tem = rev;
    rev = rev->next;
    tem->next = n;
    n = tem;
  }
  return new zero_width_node(n);
}

#else

// It's undesirable for \Z to change environments, because then
// \n(.w won't work as expected.

static node *do_zero_width_output() // \Z
{
  node *rev = new dummy_node;
  node *n = 0 /* nullptr */;
  token start_token;
  start_token.next();
  if (!want_att_compat && !start_token.is_usable_as_delimiter())
    warning(WARN_DELIM, "using %1 as an escape sequence delimiter"
			" is deprecated", tok.description());
  else if (want_att_compat
           && !start_token.is_usable_as_delimiter(false,
		  DELIMITER_ATT_STRING_EXPRESSION)) {
    warning(WARN_DELIM, "zero-width sequence escape sequence"
	    " does not accept %1 as a delimiter",
	    start_token.description());
    return 0 /* nullptr */;
  }
  // TODO: groff 1.24.0 release + 2 years?
#if 0
  if (!start_token.is_usable_as_delimiter(true /* report error */)) {
    delete rev;
    return 0 /* nullptr */;
  }
#endif
  int start_level = input_stack::get_level();
  for (;;) {
    tok.next();
    if (tok.is_newline() || tok.is_eof()) {
      // token::description() writes to static, class-wide storage, so
      // we must allocate a copy of it before issuing the next
      // diagnostic.
      char *delimdesc = strdup(start_token.description());
      warning(WARN_DELIM, "missing closing delimiter in zero-width"
	      " output escape sequence; expected %1, got %2", delimdesc,
	      tok.description());
      free(delimdesc);
      break;
    }
    if (tok == start_token
	&& (want_att_compat || input_stack::get_level() == start_level))
      break;
    // XXX: does the initial dummy node leak if this fails?
    if (!tok.add_to_zero_width_node_list(&rev))
      error("%1 is not allowed in a zero-width output escape"
	    " sequence argument", tok.description());
  }
  while (rev != 0 /* nullptr */) {
    node *tem = rev;
    rev = rev->next;
    tem->next = n;
    n = tem;
  }
  return new zero_width_node(n);
}

#endif

token_node *node::get_token_node()
{
  return 0 /* nullptr */;
}

class token_node : public node {
public:
  token tk;
  token_node(const token &t);
  void asciify(macro *);
  node *copy();
  token_node *get_token_node();
  bool is_same_as(node *);
  const char *type();
  bool causes_tprint();
  bool is_tag();
};

token_node::token_node(const token &t) : tk(t)
{
}

void token_node::asciify(macro *)
{
  assert(0 == "attempting to 'asciify' a `token_node`");
  delete this;
}

node *token_node::copy()
{
  return new token_node(tk);
}

token_node *token_node::get_token_node()
{
  return this;
}

bool token_node::is_same_as(node *nd)
{
  return (tk == static_cast<token_node *>(nd)->tk);
}

const char *token_node::type()
{
  return "token node";
}

bool token_node::causes_tprint()
{
  return false;
}

bool token_node::is_tag()
{
  return false;
}

token::token() : nd(0 /* nullptr */), type(TOKEN_EMPTY)
{
}

token::~token()
{
  delete nd;
}

token::token(const token &t)
: nm(t.nm), c(t.c), val(t.val), dim(t.dim), type(t.type)
{
  if (t.nd != 0 /* nullptr */)
    nd = t.nd->copy();
  else
    nd = 0 /* nullptr */;
}

void token::operator=(const token &t)
{
  delete nd;
  nm = t.nm;
  if (t.nd != 0 /* nullptr */)
    nd = t.nd->copy();
  else
    nd = 0 /* nullptr */;
  c = t.c;
  val = t.val;
  dim = t.dim;
  type = t.type;
}

void token::skip_spaces()
{
  while (is_space())
    next();
}

void token::diagnose_non_character()
{
  // TODO: What about
  //   is_space()
  //   is_stretchable_space()
  //   is_unstrechable_space()
  //   is_horizontal_motion()
  //   is_horizontal_whitespace()
  //   is_leader()
  //   is_backspace()
  //   is_dummy()
  //   is_transparent()
  //   is_transparent_dummy()
  //   is_left_brace()
  //   is_page_ejector()
  //   is_hyphen_indicator()
  //   is_zero_width_break()
  // ?
  if (!is_newline() && !is_eof() && !is_right_brace() && !is_tab())
    error("expected ordinary, special, or indexed character, got %1;"
	  " ignoring", description());
}

// Indicate whether an argument lies ahead on the current line in the
// input stream, skipping over spaces.  This function is therefore not
// appropriate for use when handling requests or escape sequences that
// don't use space to separate their arguments, as with `.tr aAbB` or
// `\o'^e'`.
//
// Specify `want_peek` if request reads the next argument in copy mode,
// or otherwise must interpret it specially, as when reading a
// conditional expression (`if`, `ie`, `while`), or expecting a
// delimited argument (`tl`).
bool has_arg(bool want_peek)
{
  if (tok.is_newline() || tok.is_eof())
    return false;
  if (want_peek) {
    int c;
    for (;;) {
      c = input_stack::peek();
      if (' ' == c)
	(void) read_char_in_copy_mode(0 /* nullptr */);
      else
	break;
    }
    return !(('\n' == c) || (EOF == c));
  }
  else {
    tok.skip_spaces();
    return !(tok.is_newline() || tok.is_eof());
  }
}

void token::make_space()
{
  type = TOKEN_SPACE;
}

void token::make_newline()
{
  type = TOKEN_NEWLINE;
}

void token::next()
{
  if (nd != 0 /* nullptr */) {
    delete nd;
    nd = 0 /* nullptr */;
  }
  units x;
  for (;;) {
    node *n = 0 /* nullptr */;
    int cc = input_stack::get(&n);
    if ((cc != escape_char) || 0U == escape_char) {
    handle_ordinary_char:
      switch (cc) {
      case INPUT_NO_BREAK_SPACE:
	  type = TOKEN_STRETCHABLE_SPACE;
	  return;
      case INPUT_SOFT_HYPHEN:
	  type = TOKEN_HYPHEN_INDICATOR;
	  return;
      case PUSH_GROFF_MODE:
	input_stack::set_att_compat(want_att_compat);
	want_att_compat = false;
	continue;
      case PUSH_COMP_MODE:
	input_stack::set_att_compat(want_att_compat);
	want_att_compat = true;
	continue;
      case POP_GROFFCOMP_MODE:
	want_att_compat = input_stack::get_att_compat();
	continue;
      case BEGIN_QUOTE:
	input_stack::increase_level();
	continue;
      case END_QUOTE:
	input_stack::decrease_level();
	continue;
      case DOUBLE_QUOTE:
	continue;
      case EOF:
	type = TOKEN_EOF;
	return;
      case TRANSPARENT_FILE_REQUEST:
      case TITLE_REQUEST:
      case COPY_FILE_REQUEST:
#ifdef COLUMN
      case VJUSTIFY_REQUEST:
#endif /* COLUMN */
	type = TOKEN_REQUEST;
	c = cc;
	return;
      case BEGIN_TRAP:
	type = TOKEN_BEGIN_TRAP;
	return;
      case END_TRAP:
	type = TOKEN_END_TRAP;
	return;
      case LAST_PAGE_EJECTOR:
	seen_last_page_ejector = true;
	// fall through
      case PAGE_EJECTOR:
	type = TOKEN_PAGE_EJECTOR;
	return;
      case ESCAPE_PERCENT:
      ESCAPE_PERCENT:
	type = TOKEN_HYPHEN_INDICATOR;
	return;
      case ESCAPE_SPACE:
      ESCAPE_SPACE:
	type = TOKEN_UNSTRETCHABLE_SPACE;
	return;
      case ESCAPE_TILDE:
      ESCAPE_TILDE:
	type = TOKEN_STRETCHABLE_SPACE;
	return;
      case ESCAPE_COLON:
      ESCAPE_COLON:
	type = TOKEN_ZERO_WIDTH_BREAK;
	return;
      case ESCAPE_e:
      ESCAPE_e:
	type = TOKEN_ESCAPE;
	return;
      case ESCAPE_E:
	goto handle_escape_char;
      case ESCAPE_BAR:
      ESCAPE_BAR:
	type = TOKEN_HORIZONTAL_MOTION;
	nd = new hmotion_node(curenv->get_narrow_space_width(),
			      curenv->get_fill_color());
	return;
      case ESCAPE_CIRCUMFLEX:
      ESCAPE_CIRCUMFLEX:
	type = TOKEN_HORIZONTAL_MOTION;
	nd = new hmotion_node(curenv->get_half_narrow_space_width(),
			      curenv->get_fill_color());
	return;
      case ESCAPE_NEWLINE:
	have_formattable_input = false;
	break;
      case ESCAPE_LEFT_BRACE:
      ESCAPE_LEFT_BRACE:
	type = TOKEN_LEFT_BRACE;
	return;
      case ESCAPE_RIGHT_BRACE:
      ESCAPE_RIGHT_BRACE:
	type = TOKEN_RIGHT_BRACE;
	return;
      case ESCAPE_LEFT_QUOTE:
      ESCAPE_LEFT_QUOTE:
	type = TOKEN_SPECIAL_CHAR;
	nm = symbol("ga");
	return;
      case ESCAPE_RIGHT_QUOTE:
      ESCAPE_RIGHT_QUOTE:
	type = TOKEN_SPECIAL_CHAR;
	nm = symbol("aa");
	return;
      case ESCAPE_HYPHEN:
      ESCAPE_HYPHEN:
	type = TOKEN_SPECIAL_CHAR;
	nm = symbol("-");
	return;
      case ESCAPE_UNDERSCORE:
      ESCAPE_UNDERSCORE:
	type = TOKEN_SPECIAL_CHAR;
	nm = symbol("ul");
	return;
      case ESCAPE_c:
      ESCAPE_c:
	type = TOKEN_INTERRUPT;
	return;
      case ESCAPE_BANG:
      ESCAPE_BANG:
	type = TOKEN_TRANSPARENT;
	return;
      case ESCAPE_QUESTION:
      ESCAPE_QUESTION:
	nd = do_non_interpreted();
	if (nd != 0 /* nullptr */) {
	  type = TOKEN_NODE;
	  return;
	}
	break;
      case ESCAPE_AMPERSAND:
      ESCAPE_AMPERSAND:
	type = TOKEN_DUMMY;
	return;
      case ESCAPE_RIGHT_PARENTHESIS:
      ESCAPE_RIGHT_PARENTHESIS:
	type = TOKEN_TRANSPARENT_DUMMY;
	return;
      case '\b':
	type = TOKEN_BACKSPACE;
	return;
      case ' ':
	type = TOKEN_SPACE;
	return;
      case '\t':
	type = TOKEN_TAB;
	return;
      case '\n':
	type = TOKEN_NEWLINE;
	return;
      case '\001':
	type = TOKEN_LEADER;
	return;
      case 0:
	{
	  assert(n != 0 /* nullptr */);
	  token_node *tn = n->get_token_node();
	  if (tn != 0 /* nullptr */) {
	    *this = tn->tk;
	    delete tn;
	  }
	  else {
	    nd = n;
	    type = TOKEN_NODE;
	  }
	}
	return;
      default:
	type = TOKEN_CHAR;
	c = cc;
	return;
      }
    }
    else {
    handle_escape_char:
      cc = input_stack::get(&n);
      switch (cc) {
      case '(':
	nm = read_two_char_escape_parameter();
	type = TOKEN_SPECIAL_CHAR;
	return;
      case EOF:
	type = TOKEN_EOF;
	error("end of input after escape character");
	return;
      case '`':
	goto ESCAPE_LEFT_QUOTE;
      case '\'':
	goto ESCAPE_RIGHT_QUOTE;
      case '-':
	goto ESCAPE_HYPHEN;
      case '_':
	goto ESCAPE_UNDERSCORE;
      case '%':
	goto ESCAPE_PERCENT;
      case ' ':
	goto ESCAPE_SPACE;
      case '0':
	nd = new hmotion_node(curenv->get_digit_width(),
			      curenv->get_fill_color());
	type = TOKEN_HORIZONTAL_MOTION;
	return;
      case '|':
	goto ESCAPE_BAR;
      case '^':
	goto ESCAPE_CIRCUMFLEX;
      case '/':
	if (want_att_compat)
	  warning(WARN_SYNTAX, "an escaped '%1' is not portable to"
		  " AT&T troff", char(cc));
	type = TOKEN_ITALIC_CORRECTION;
	return;
      case ',':
	if (want_att_compat)
	  warning(WARN_SYNTAX, "an escaped '%1' is not portable to"
		  " AT&T troff", char(cc));
	type = TOKEN_NODE;
	nd = new left_italic_corrected_node;
	return;
      case '&':
	goto ESCAPE_AMPERSAND;
      case ')':
	if (want_att_compat)
	  warning(WARN_SYNTAX, "an escaped '%1' is not portable to"
		  " AT&T troff", char(cc));
	goto ESCAPE_RIGHT_PARENTHESIS;
      case '!':
	goto ESCAPE_BANG;
      case '?':
	if (want_att_compat)
	  warning(WARN_SYNTAX, "an escaped '%1' is not portable to"
		  " AT&T troff", char(cc));
	goto ESCAPE_QUESTION;
      case '~':
	if (want_att_compat)
	  warning(WARN_SYNTAX, "an escaped '%1' is not portable to"
		  " AT&T troff", char(cc));
	goto ESCAPE_TILDE;
      case ':':
	if (want_att_compat)
	  warning(WARN_SYNTAX, "an escaped '%1' is not portable to"
		  " AT&T troff", char(cc));
	goto ESCAPE_COLON;
      case '"':
	while ((cc = input_stack::get(0 /* nullptr */)) != '\n'
	       && cc != EOF)
	  ;
	if (cc == '\n')
	  type = TOKEN_NEWLINE;
	else
	  type = TOKEN_EOF;
	return;
      case '#':			// Like \" but newline is ignored.
	if (want_att_compat)
	  warning(WARN_SYNTAX, "an escaped '%1' is not portable to"
		  " AT&T troff", char(cc));
	while ((cc = input_stack::get(0 /* nullptr */)) != '\n')
	  if (cc == EOF) {
	    type = TOKEN_EOF;
	    return;
	  }
	break;
      case '$':
	{
	  symbol s = read_escape_parameter();
	  if (!(s.is_null() || s.is_empty()))
	    interpolate_positional_parameter(s);
	  break;
	}
      case '*':
	{
	  symbol s = read_escape_parameter(WITH_ARGS);
	  if (!(s.is_null() || s.is_empty())) {
	    if (have_multiple_params) {
	      have_multiple_params = false;
	      interpolate_string_with_args(s);
	    }
	    else
	      interpolate_string(s);
	  }
	  break;
	}
      case 'a':
	nd = new non_interpreted_char_node('\001');
	type = TOKEN_NODE;
	return;
      case 'A':
	if (want_att_compat)
	  warning(WARN_SYNTAX, "an escaped '%1' is not portable to"
		  " AT&T troff", char(cc));
	{
	  const char *res = do_name_test();
	  if (0 /* nullptr */ == res)
	    break;
	  c = *res;
	  type = TOKEN_CHAR;
	}
	return;
      case 'b':
	nd = do_bracket();
	if (0 /* nullptr */ == nd)
	  break;
	type = TOKEN_NODE;
	return;
      case 'B':
	if (want_att_compat)
	  warning(WARN_SYNTAX, "an escaped '%1' is not portable to"
		  " AT&T troff", char(cc));
	{
	  const char *res = do_expr_test();
	  if (0 /* nullptr */ == res)
	    break;
	  c = *res;
	  type = TOKEN_CHAR;
	}
	return;
      case 'c':
	goto ESCAPE_c;
      case 'C':
	nm = read_delimited_identifier();
	if (nm.is_null())
	  break;
	type = TOKEN_DELIMITED_SPECIAL_CHAR;
	return;
      case 'd':
	type = TOKEN_NODE;
	nd = new vmotion_node(curenv->get_size() / 2,
			      curenv->get_fill_color());
	return;
      case 'D':
	nd = read_drawing_command();
	if (0 /* nullptr */ == nd)
	  break;
	type = TOKEN_NODE;
	return;
      case 'e':
	goto ESCAPE_e;
      case 'E':
	if (want_att_compat)
	  warning(WARN_SYNTAX, "an escaped '%1' is not portable to"
		  " AT&T troff", char(cc));
	goto handle_escape_char;
      case 'f':
	if (curenv->get_was_line_interrupted()) {
	  warning(WARN_SYNTAX, "ignoring escaped '%1' on input line"
		  " after output line continuation escape sequence",
		  char(cc));
	  break;
	}
	select_font(read_escape_parameter(ALLOW_EMPTY));
	if (!want_att_compat)
	  have_formattable_input = true;
	break;
      case 'F':
	if (want_att_compat)
	  warning(WARN_SYNTAX, "an escaped '%1' is not portable to"
		  " AT&T troff", char(cc));
	curenv->set_family(read_escape_parameter(ALLOW_EMPTY));
	have_formattable_input = true;
	break;
      case 'g':
	{
	  symbol s = read_escape_parameter();
	  if (!(s.is_null() || s.is_empty()))
	    interpolate_number_format(s);
	  break;
	}
      case 'h':
	if (!read_delimited_measurement(&x, 'm'))
	  break;
	type = TOKEN_DELIMITED_HORIZONTAL_MOTION;
	nd = new hmotion_node(x, curenv->get_fill_color());
	return;
      case 'H':
	// don't take height increments relative to previous height if
	// in compatibility mode
	if (!want_att_compat && curenv->get_char_height()) {
	  if (read_delimited_measurement(&x, 'z',
					 curenv->get_char_height()))
	    curenv->set_char_height(x);
	}
	else {
	  if (read_delimited_measurement(&x, 'z',
		curenv->get_requested_point_size()))
	    curenv->set_char_height(x);
	}
	if (!want_att_compat)
	  have_formattable_input = true;
	break;
      case 'k':
	nm = read_escape_parameter();
	if (nm.is_null() || nm.is_empty())
	  break;
	type = TOKEN_MARK_INPUT;
	return;
      case 'l':
      case 'L':
	{
	  charinfo *s = 0 /* nullptr */;
	  if (!read_line_rule_expression(&x, (cc == 'l' ? 'm': 'v'),
					 &s))
	    break;
	  if (0 /* nullptr */ == s)
	    s = lookup_charinfo(cc == 'l' ? "ru" : "br");
	  type = TOKEN_NODE;
	  node *char_node = curenv->make_char_node(s);
	  if (cc == 'l')
	    nd = new hline_node(x, char_node);
	  else
	    nd = new vline_node(x, char_node);
	  return;
	}
      case 'm':
	if (want_att_compat)
	  warning(WARN_SYNTAX, "an escaped '%1' is not portable to"
		  " AT&T troff", char(cc));
	do_stroke_color(read_escape_parameter(ALLOW_EMPTY));
	if (!want_att_compat)
	  have_formattable_input = true;
	break;
      case 'M':
	if (want_att_compat)
	  warning(WARN_SYNTAX, "an escaped '%1' is not portable to"
		  " AT&T troff", char(cc));
	do_fill_color(read_escape_parameter(ALLOW_EMPTY));
	if (!want_att_compat)
	  have_formattable_input = true;
	break;
      case 'n':
	{
	  int inc;
	  symbol s = read_increment_and_escape_parameter(&inc);
	  if (!(s.is_null() || s.is_empty()))
	    interpolate_register(s, inc);
	  break;
	}
      case 'N':
	// The argument is a glyph index, which is dimensionless.
	if (!read_delimited_measurement(&val, 0 /* dimensionless */))
	  break;
	type = TOKEN_INDEXED_CHAR;
	return;
      case 'o':
	nd = do_overstrike();
	if (0 /* nullptr */ == nd)
	  break;
	type = TOKEN_NODE;
	return;
      case 'O':
	if (want_att_compat)
	  warning(WARN_SYNTAX, "an escaped '%1' is not portable to"
		  " AT&T troff", char(cc));
	nd = do_suppress(read_escape_parameter());
	if (0 /* nullptr */ == nd)
	  break;
	type = TOKEN_NODE;
	return;
      case 'p':
	type = TOKEN_SPREAD;
	return;
      case 'r':
	type = TOKEN_NODE;
	nd = new vmotion_node(-curenv->get_size(), curenv->get_fill_color());
	return;
      case 'R':
	if (want_att_compat)
	  warning(WARN_SYNTAX, "an escaped '%1' is not portable to"
		  " AT&T troff", char(cc));
	do_register();
	if (!want_att_compat)
	  have_formattable_input = true;
	break;
      case 's':
	if (curenv->get_was_line_interrupted()) {
	  warning(WARN_SYNTAX, "ignoring escaped '%1' on input line"
		  " after output line continuation escape sequence",
		  char(cc));
	  break;
	}
	if (read_size(&x))
	  curenv->set_size(x);
	if (!want_att_compat)
	  have_formattable_input = true;
	break;
      case 'S':
	// The argument is in degrees, which are dimensionless.
	if (read_delimited_measurement(&x, 0 /* dimensionless */))
	  curenv->set_char_slant(x);
	if (!want_att_compat)
	  have_formattable_input = true;
	break;
      case 't':
	type = TOKEN_NODE;
	nd = new non_interpreted_char_node('\t');
	return;
      case 'u':
	type = TOKEN_NODE;
	nd = new vmotion_node(-curenv->get_size() / 2,
			      curenv->get_fill_color());
	return;
      case 'v':
	if (!read_delimited_measurement(&x, 'v'))
	  break;
	type = TOKEN_NODE;
	nd = new vmotion_node(x, curenv->get_fill_color());
	return;
      case 'V':
	if (want_att_compat)
	  warning(WARN_SYNTAX, "an escaped '%1' is not portable to"
		  " AT&T troff", char(cc));
	{
	  symbol s = read_escape_parameter();
	  if (!(s.is_null() || s.is_empty()))
	    interpolate_environment_variable(s);
	  break;
	}
      case 'w':
	do_width();
	break;
      case 'x':
	if (!read_delimited_measurement(&x, 'v'))
	  break;
	type = TOKEN_NODE;
	nd = new extra_size_node(x);
	return;
      case 'X':
	nd = do_device_extension();
	if (0 /* nullptr */ == nd)
	  break;
	type = TOKEN_NODE;
	return;
      case 'Y':
	if (want_att_compat)
	  warning(WARN_SYNTAX, "an escaped '%1' is not portable to"
		  " AT&T troff", char(cc));
	{
	  symbol s = read_escape_parameter();
	  if (s.is_null() || s.is_empty())
	    break;
	  request_or_macro *p = lookup_request(s);
	  macro *m = p->to_macro();
	  if (0 /* nullptr */ == m) {
	    error("cannot interpolate '%1' to device-independent"
		  " output; it is a request, not a macro",
		  s.contents());
	    break;
	  }
	  nd = new device_extension_node(*m);
	  type = TOKEN_NODE;
	  return;
	}
      case 'z':
	next();
	if ((TOKEN_NODE == type)
	    || (TOKEN_HORIZONTAL_MOTION == type)
	    || (TOKEN_DELIMITED_HORIZONTAL_MOTION == type))
	  nd = new zero_width_node(nd);
	else {
	  // TODO: In theory, we could accept spaces and horizontal
	  // motions.
	  charinfo *ci = get_charinfo(true /* required */);
	  if (0 /* nullptr */ == ci) {
	    error("%1 is not supported in a zero-width character"
		  " escape sequence argument", tok.description());
	    break;
	  }
	  node *gn = curenv->make_char_node(ci);
	  if (0 /* nullptr */ == gn) {
	    assert("make_char_node failed to create a character"
		   " node");
	    break;
	  }
	  nd = new zero_width_node(gn);
	  type = TOKEN_NODE;
	}
	return;
      case 'Z':
	if (want_att_compat)
	  warning(WARN_SYNTAX, "an escaped '%1' is not portable to"
		  " AT&T troff", char(cc));
	nd = do_zero_width_output();
	if (0 /* nullptr */ == nd)
	  break;
	type = TOKEN_NODE;
	return;
      case '{':
	goto ESCAPE_LEFT_BRACE;
      case '}':
	goto ESCAPE_RIGHT_BRACE;
      case '\n':
	break;
      case '[':
	if (want_att_compat)
	  warning(WARN_SYNTAX, "an escaped '%1' is not portable to"
		  " AT&T troff", char(cc));
	if (!want_att_compat) {
	  symbol s = read_long_escape_parameters(WITH_ARGS);
	  if (s.is_null() || s.is_empty())
	    break;
	  if (have_multiple_params) {
	    have_multiple_params = false;
	    nm = composite_glyph_name(s);
	  }
	  else {
	    const char *sc = s.contents();
	    const char *gn = 0 /* nullptr */;
	    if ((strlen(sc) > 2) && (sc[0] == 'u'))
	      gn = valid_unicode_code_sequence(sc, 0 /* nullptr */);
	    if (gn != 0 /* nullptr */) {
	      const char *gn_decomposed = decompose_unicode(gn);
	      if (gn_decomposed != 0 /* nullptr */)
		gn = &gn_decomposed[1];
	      const char *groff_gn = unicode_to_glyph_name(gn);
	      if (groff_gn != 0 /* nullptr */)
		nm = symbol(groff_gn);
	      else {
		// ISO C++ does not permit VLAs on the stack.
		// C++03: new char[strlen(gn) + 1 + 1]();
		char *buf = new char[strlen(gn) + 1 + 1];
		(void) memset(buf, 0,
			      (strlen(gn) + 1 + 1) * sizeof(char));
		strcpy(buf, "u");
		strcat(buf, gn);
		nm = symbol(buf);
		delete[] buf;
	      }
	    }
	    else
	      nm = symbol(sc);
	  }
	  type = TOKEN_SPECIAL_CHAR;
	  return;
	}
	goto handle_ordinary_char;
      default:
	if ((cc != escape_char) && (cc != '.'))
	  warning(WARN_ESCAPE, "ignoring escape character before %1",
		  input_char_description(cc));
	goto handle_ordinary_char;
      }
    }
  }
}

bool token::operator==(const token &t)
{
  if (type != t.type)
    return false;
  switch (type) {
  case TOKEN_CHAR:
    return c == t.c;
  case TOKEN_SPECIAL_CHAR:
  case TOKEN_DELIMITED_SPECIAL_CHAR:
    return nm == t.nm;
  case TOKEN_INDEXED_CHAR:
    return val == t.val;
  default:
    return true;
  }
}

bool token::operator!=(const token &t)
{
  return !(*this == t);
}

// Is the character usable as a delimiter?
//
// This is used directly only by `do_device_extension()`, because it is
// the only escape sequence that reads its argument in copy mode (so it
// doesn't tokenize it) and accepts a user-specified delimiter.
static bool is_char_usable_as_delimiter(int c)
{
  // Reject all characters that can validly begin a numeric expression.
  switch (c) {
  case '0':
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
  case '8':
  case '9':
  case '+':
  case '-':
  // case '/':
  // case '*':
  // case '%':
  // case '<':
  // case '>':
  // case '=':
  // case '&':
  // case ':':
  case '(':
  // case ')':
  case '.':
  case '|':
    return false;
  default:
    return true;
  }
}

void token::describe_node(char *buf, size_t bufsz)
{
  assert(nd != 0 /* nullptr */);
  if (0 /* nullptr */ == nd) {
    (void) snprintf(buf, bufsz, "a null(!) node");
    return;
  }
  // Ah, the joys of computational natural language grammar.
  const char *ndtype = nd->type();
  const char initial_letter = ndtype[0];
  bool is_vowelly = false;
  // I wonder if Kernighan thought that the presence of set types and an
  // "in" operator was one of Pascal's great blunders.  --GBR
  if (('a' == initial_letter)
      || ('e' == initial_letter)
      || ('i' == initial_letter)
      || ('o' == initial_letter)
      || ('u' == initial_letter))
    is_vowelly = true;
  (void) memset(buf, 0, bufsz);
  (void) snprintf(buf, bufsz, "a%s %s", is_vowelly ? "n" : "", ndtype);
}

// Is the token a valid delimiter (like `'`)?
bool token::is_usable_as_delimiter(bool report_error,
				   enum delimiter_context context)
{
  bool is_valid = false;
  switch (type) {
  case TOKEN_CHAR:
    if (!want_att_compat)
      is_valid = is_char_usable_as_delimiter(c);
    else {
      assert(context != DELIMITER_GROFF_EXPRESSION);
      switch (context) {
      case DELIMITER_ATT_STRING_EXPRESSION:
	if (csgraph(c)
	    || (((c > 0) && (c < 012)) || (014 == c) || (0177 == c)))
	  is_valid = true;
	break;
      case DELIMITER_ATT_NUMERIC_EXPRESSION:
	if (csgraph(c)
	    || (((c > 0) && (c < 012)) || (014 == c) || (0177 == c)))
	  is_valid = true;
	// AT&T troff doesn't accept as numeric expression delimiters
	// characters that validly appear in a numeric expression,
	// _except_ for numerals, `|`, and `.`.
	if (('+' == c)
	    || ('-' == c)
	    || ('/' == c)
	    || ('*' == c)
	    || ('%' == c)
	    || ('<' == c)
	    || ('>' == c)
	    || ('=' == c)
	    || ('&' == c)
	    || (':' == c)
	    || ('(' == c)
	    || (')' == c))
	  is_valid = false;
	break;
      case DELIMITER_ATT_OUTPUT_COMPARISON_EXPRESSION:
	if (csupper(c)
	    || (cslower(c)
		&& (c != 'e')
		&& (c != 'n')
		&& (c != 'o')
		&& (c != 't'))
	    || cspunct(c)
	    || (((c > 0) && (c < 012)) || (014 == c) || (0177 == c)))
	  is_valid = true;
	// AT&T troff doesn't accept as conditional expression
	// delimiters characters that can validly appear in a numeric
	// expression, nor `!`.  We already excluded numerals above.
	if (('+' == c)
	    || ('-' == c)
	    || ('/' == c)
	    || ('*' == c)
	    || ('%' == c)
	    || ('<' == c)
	    || ('>' == c)
	    || ('=' == c)
	    || ('&' == c)
	    || (':' == c)
	    || ('(' == c)
	    || (')' == c)
	    || ('|' == c)
	    || ('.' == c)
	    || ('!' == c))
	  is_valid = false;
	break;
      default:
	assert(0 == "unhandled case of `context` (enum dcontext)");
	break;
      }
    }
    if (!is_valid && report_error)
      error("character '%1' is not allowed as a delimiter",
	    static_cast<char>(c));
    return is_valid;
  case TOKEN_NODE:
    if (report_error) {
      // Reserve a buffer large enough to handle the lengthiest case.
      // See `token::description()`.
      const size_t bufsz
	= sizeof "space character horizontal motion node token"
	  + sizeof "bracketrighttp"
	  + 2 /* for trailing '"' and '\0' */;
      // C++03: char[bufsz]();
      static char buf[bufsz];
      (void) memset(buf, 0, bufsz);
      describe_node(buf, bufsz);
      error("%1 is not allowed as a delimiter", buf);
    }
    return false;
  case TOKEN_SPACE:
  case TOKEN_STRETCHABLE_SPACE:
  case TOKEN_UNSTRETCHABLE_SPACE:
  case TOKEN_DELIMITED_HORIZONTAL_MOTION:
  case TOKEN_DELIMITED_SPECIAL_CHAR:
  case TOKEN_NEWLINE:
  case TOKEN_EOF:
    if (report_error)
      error("%1 is not allowed as a delimiter", description());
    return false;
  default:
    return true;
  }
}

const char *token::description()
{
  // Reserve a buffer large enough to handle the lengthiest cases.  The
  // user can still contrive, by accident or otherwise, an arbitrarily
  // long identifier.
  //   "character code XXX"
  //   "special character 'bracketrighttp'"
  //   "indexed character -2147483648"
  //   "space character horizontal motion node token"
  //   "nonexistent special character or class"
  // Future:
  //   "character code XXX (U+XXXX)" or similar
  const size_t bufsz
    = sizeof "space character horizontal motion node token"
      + sizeof "bracketrighttp"
      + 2 /* for trailing '"' and '\0' */;
  static char buf[bufsz];
  (void) memset(buf, 0, bufsz);
  switch (type) {
  case TOKEN_EMPTY:
    return "an indeterminate token (at start of input?)";
  case TOKEN_BACKSPACE:
    return "a backspace character";
  case TOKEN_CHAR:
    if (INPUT_DELETE == c)
      return "a delete character";
    else if ('\'' == c) {
      (void) snprintf(buf, bufsz, "character \"%c\"", c);
      return buf;
    }
    else if (c < 128) {
      (void) snprintf(buf, bufsz, "character '%c'", c);
      return buf;
    }
    else {
      (void) snprintf(buf, bufsz, "character code %d", c);
      return buf;
    }
  case TOKEN_DUMMY:
    return "an escaped '&'";
  case TOKEN_ESCAPE:
    return "an escaped 'e'";
  case TOKEN_HYPHEN_INDICATOR:
    return "an escaped '%'";
  case TOKEN_INTERRUPT:
    return "an escaped 'c'";
  case TOKEN_ITALIC_CORRECTION:
    return "an escaped '/'";
  case TOKEN_LEADER:
    return "a leader character";
  case TOKEN_LEFT_BRACE:
    return "an escaped '{'";
  case TOKEN_MARK_INPUT:
    return "an escaped 'k'";
  case TOKEN_NEWLINE:
    return "a newline";
  case TOKEN_NODE:
    {
      static char nodebuf[bufsz - (sizeof " token")];
      (void) strcpy(nodebuf, "an undescribed node");
      describe_node(nodebuf, bufsz);
      (void) snprintf(buf, bufsz, "%s token", nodebuf);
      return buf;
    }
  case TOKEN_INDEXED_CHAR:
    (void) snprintf(buf, bufsz, "indexed character %d",
		    character_index());
    return buf;
  case TOKEN_RIGHT_BRACE:
    return "an escaped '}'";
  case TOKEN_SPACE:
    return "a space";
  case TOKEN_SPECIAL_CHAR:
  case TOKEN_DELIMITED_SPECIAL_CHAR:
    // We normally use apostrophes for quotation in diagnostic messages,
    // but many special character names contain them.  Fall back to
    // double quotes if this one does.  A user-defined special character
    // name could contain both characters; we expect such users to lie
    // comfortably in the bed they made for themselves.
    {
      const char *sc = nm.contents();
      char qc = '\'';
      if (strchr(sc, '\'') != 0 /* nullptr */)
	qc = '"';
      // TODO: This truncates the names of impractically long special
      // character or character class names.  Do something about that.
      // (The truncation is visually indicated by the absence of a
      // closing quotation mark.)
      static const char special_character[] = "special character";
      static const char character_class[] = "character class";
      static const char nonexistent[] = "nonexistent special character"
					" or class";
      const char *ctype = special_character;
      charinfo *ci = get_charinfo(false /* required */,
				  true /* suppress creation */);
      if (0 /* nullptr */ == ci)
	ctype = nonexistent;
      else if (ci->is_class())
	ctype = character_class;
      (void) snprintf(buf, bufsz, "%s %c%s%c", ctype, qc, sc, qc);
      return buf;
    }
  case TOKEN_SPREAD:
    return "an escaped 'p'";
  case TOKEN_STRETCHABLE_SPACE:
    return "an escaped '~'";
  case TOKEN_UNSTRETCHABLE_SPACE:
    return "an escaped ' '";
  case TOKEN_DELIMITED_HORIZONTAL_MOTION:
    return "a parameterized horizontal motion";
  case TOKEN_HORIZONTAL_MOTION:
    return "a horizontal motion";
  case TOKEN_TAB:
    return "a tab character";
  case TOKEN_TRANSPARENT:
    return "an escaped '!'";
  case TOKEN_TRANSPARENT_DUMMY:
    return "an escaped ')'";
  case TOKEN_ZERO_WIDTH_BREAK:
    return "an escaped ':'";
  case TOKEN_EOF:
    return "end of input";
  // troff generates the following token types internally; the input
  // can't (directly), so seeing them described likely suggests a bug in
  // the formatter.  We consequently don't attempt to make their
  // descriptions intelligible to the novice.
  case TOKEN_BEGIN_TRAP:
    return "a \"begin trap\" token";
  case TOKEN_END_TRAP:
    return "an \"end trap\" token";
  // TOKEN_NODE is handled above
  case TOKEN_PAGE_EJECTOR:
    return "a \"page ejector\" token";
  case TOKEN_REQUEST:
    return "a \"request\" token";
  default:
    assert(0 == "unhandled case of `type` (token)");
    return "an undescribed token";
  }
}

void skip_line()
{
  while (!tok.is_newline())
    if (tok.is_eof())
      return;
    else
      tok.next();
  tok.next();
}

void compatible()
{
  int n;
  if (has_arg() && read_integer(&n))
    want_att_compat = (n > 0);
  else
    want_att_compat = true;
  skip_line();
}

static void diagnose_missing_identifier(bool required)
{
  if (tok.is_newline() || tok.is_eof()) {
    if (required)
      warning(WARN_MISSING, "missing identifier");
  }
  else if (tok.is_right_brace() || tok.is_tab()) {
    // token::description() writes to static, class-wide storage, so we
    // must allocate a copy of it before issuing the next diagnostic.
    char *start = strdup(tok.description());
    do {
      tok.next();
    } while (tok.is_space() || tok.is_right_brace() || tok.is_tab());
    // XXX: unreachable code? --GBR
    if (!tok.is_newline() && !tok.is_eof())
      error("%1 is not allowed before an argument", start);
    else if (required)
      warning(WARN_MISSING, "missing identifier");
    free(start);
  }
  else if (required)
    error("expected identifier, got %1", tok.description());
  else
    error("expected identifier, got %1; treated as missing",
	  tok.description());
}

static void diagnose_invalid_identifier()
{
  if (!tok.is_newline() && !tok.is_eof() && !tok.is_space()
      && !tok.is_tab() && !tok.is_right_brace()
      // We don't want to give a warning for .el\{
      && !tok.is_left_brace())
    error("%1 is not allowed in an identifier", tok.description());
}

symbol read_identifier(bool required)
{
  if (want_att_compat) {
    char buf[3];
    tok.skip_spaces();
    if ((buf[0] = tok.ch()) != 0U) {
      tok.next();
      if ((buf[1] = tok.ch()) != 0U) {
	buf[2] = '\0';
	tok.make_space();
      }
      else
	diagnose_invalid_identifier();
      return symbol(buf);
    }
    else {
      diagnose_missing_identifier(required);
      return NULL_SYMBOL;
    }
  }
  else
    return read_long_identifier(required);
}

symbol read_long_identifier(bool required)
{
  return read_input_until_terminator(required, 0U,
				     true /* want identifier */);
}

// Read bytes from input until reaching a null byte or the specified
// `end_char`; construct and return a `symbol` object therefrom.
static symbol read_input_until_terminator(bool required,
					  unsigned char end_char,
					  bool want_identifier)
{
  tok.skip_spaces();
  int buf_size = default_buffer_size;
  // TODO: grochar
  unsigned char *buf = 0 /* nullptr */;
  try {
    // C++03: new char[buf_size]();
    buf = new unsigned char[buf_size];
  }
  catch (const std::bad_alloc &e) {
    fatal("cannot allocate %1 bytes to read input line", buf_size);
  }
  (void) memset(buf, 0, (buf_size * sizeof(unsigned char)));
  int i = 0;
  const unsigned char terminator = end_char; // TODO: grochar
  for (;;) {
    // If `terminator` != 0U we normally have to append a null byte.
    if ((i + 2) > buf_size) {
      unsigned char *old_buf = buf; // TODO: grochar
      int new_buf_size = buf_size * 2;
      // C++03: new char[new_buf_size]();
      try {
	buf = new unsigned char[new_buf_size];
      }
      catch (const std::bad_alloc &e) {
	fatal("cannot allocate %1 bytes to read input line", buf_size);
      }
      (void) memset(buf, 0, (new_buf_size * sizeof(unsigned char)));
      (void) memcpy(buf, old_buf, (buf_size * sizeof(unsigned char)));
      buf_size = new_buf_size;
      delete[] old_buf;
    }
    buf[i] = tok.ch();
    if ((0U == buf[i]) || (terminator == buf[i]))
      break;
    else if (want_identifier && ((buf[i] < ' ') || (buf[i] > 159))) {
      // Of C0 controls, Solaris, Heirloom, and Plan 9 troff support
      // ^[BCEFG] (only) in identifiers.  DWB 3.3 supports none.
      assert(buf[i] != ' '); // ensure caller handled spaces
      error("character code %1 is not allowed in an identifier",
	    static_cast<int>(buf[i]));
      delete[] buf;
      return NULL_SYMBOL;
    }
    i++;
    tok.next();
  }
  if (0 == i) {
    diagnose_missing_identifier(required);
    delete[] buf;
    return NULL_SYMBOL;
  }
  if ((terminator != 0U) && (terminator == buf[i]))
    buf[i + 1] = '\0';
  else
    diagnose_invalid_identifier();
  char *chbuf = 0 /* nullptr */;
  try {
    // C++03: new char[buf_size]();
    chbuf = new char[buf_size];
  }
  catch (const std::bad_alloc &e) {
    fatal("cannot allocate %1 bytes to copy identifier", buf_size);
  }
  for (int j = 0; j < buf_size; j++)
    chbuf[j] = static_cast<char>(buf[j]);
  delete[] buf;
  symbol s(chbuf);
  delete[] chbuf;
  return s;
}

static void close_all_streams();

void exit_troff()
{
  is_exit_underway = true;
  close_all_streams();
  topdiv->set_last_page();
  if (!end_of_input_macro_name.is_null()) {
    spring_trap(end_of_input_macro_name);
    tok.next();
    process_input_stack();
  }
  curenv->final_break();
  tok.next();
  process_input_stack();
  end_diversions();
  if (topdiv->get_page_length() > 0) {
    is_eoi_macro_finished = true;
    topdiv->set_ejecting();
    static unsigned char buf[2] = { LAST_PAGE_EJECTOR, '\0' };
    // XXX: Ugliness alert.  GNU troff wants to eat its cake and have it
    // too, using the explicit `unsigned char` numeric type to represent
    // input characters while also using C/C++'s `char` type--of
    // undefined signedness--and its literals, including character
    // string literals like `"\n"`, in free admixture therewith.
    //
    // Fixing this the right way means widening the fundamental
    // character type of GNU troff formatting operations, possibly to
    // `char32_t` (C++11).  That's a heavy lift; see Savannah #40720.
    input_stack::push(make_temp_iterator(reinterpret_cast<char *>(buf)));
    topdiv->space(topdiv->get_page_length(), true /* forcing */);
    tok.next();
    process_input_stack();
    // TODO: Resolve the follwing case and enable the assertion.
    // $ printf '.DS\n.DE\n' | ./build/test-groff -ms
    // troff: ../src/roff/troff/input.cpp:2937: void exit_troff():
    //   Assertion `seen_last_page_ejector' failed.
    // .../build/groff: error: troff: Aborted (core dumped)
    //assert(seen_last_page_ejector);
    seen_last_page_ejector = true;	// should be set already
    topdiv->set_ejecting();
    push_page_ejector();
    topdiv->space(topdiv->get_page_length(), true /* forcing */);
    tok.next();
    process_input_stack();
  }
  // TODO: delete pointers in file name set.
  write_any_trailer_and_exit(EXIT_SUCCESS);
}

// This implements .ex.  The input stack must be cleared before calling
// exit_troff().

void exit_request()
{
  input_stack::clear();
  if (is_exit_underway)
    tok.next();
  else
    exit_troff();
}

void return_macro_request()
{
  if (has_arg() && (tok.ch() != 0U))
    input_stack::pop_macro();
  input_stack::pop_macro();
  tok.next();
}

void eoi_macro()
{
  end_of_input_macro_name = read_identifier();
  skip_line();
}

void blank_line_macro()
{
  blank_line_macro_name = read_identifier();
  skip_line();
}

void leading_spaces_macro()
{
  leading_spaces_macro_name = read_identifier();
  skip_line();
}

static void trapping_blank_line()
{
  if (!blank_line_macro_name.is_null())
    spring_trap(blank_line_macro_name);
  else
    blank_line();
}

std::stack<bool> want_att_compat_stack;

void do_request()
{
  if (!has_arg()) {
    warning(WARN_MISSING, "groff syntax interpretation request expects"
	    " a request or macro as argument");
    skip_line();
    return;
  }
  want_att_compat_stack.push(want_att_compat);
  want_att_compat = false;
  symbol nm = read_identifier();
  if (nm.is_null())
    skip_line();
  else
    interpolate_macro(nm, true /* don't want next token */);
  assert(!want_att_compat_stack.empty());
  want_att_compat = want_att_compat_stack.top();
  want_att_compat_stack.pop();
  request_or_macro *p = lookup_request(nm);
  macro *m = p->to_macro();
  if (m != 0 /* nullptr */)
    tok.next();
}

inline bool possibly_handle_first_page_transition()
{
  if ((topdiv->before_first_page_status > 0) && (curdiv == topdiv)
      && !curenv->is_dummy()) {
    handle_first_page_transition();
    return true;
  }
  else
    return false;
}

static int transparent_translate(int cc)
{
  if (!is_invalid_input_char(cc)) {
    charinfo *ci = charset_table[cc];
    switch (ci->get_special_translation(true /* transparently */)) {
    case charinfo::TRANSLATE_SPACE:
      return ' ';
    case charinfo::TRANSLATE_STRETCHABLE_SPACE:
      return ESCAPE_TILDE;
    case charinfo::TRANSLATE_DUMMY:
      return ESCAPE_AMPERSAND;
    case charinfo::TRANSLATE_HYPHEN_INDICATOR:
      return ESCAPE_PERCENT;
    }
    // This is really ugly.
    ci = ci->get_translation(1);
    if (ci != 0 /* nullptr */) {
      unsigned char c = ci->get_ascii_code();
      if (c != 0U)
	return c;
      error("cannot translate %1 to special character '%2' in"
	    " device-independent output", input_char_description(cc),
	    ci->nm.contents());
    }
  }
  return cc;
}

bool node::need_reread(bool *)
{
  return false;
}

int global_diverted_space = 0;

bool diverted_space_node::need_reread(bool *bolp)
{
  global_diverted_space = 1;
  if (curenv->get_fill())
    trapping_blank_line();
  else
    curdiv->space(n);
  global_diverted_space = 0;
  *bolp = true;
  return true;
}

bool diverted_copy_file_node::need_reread(bool *bolp)
{
  curdiv->copy_file(filename.contents());
  *bolp = true;
  return true;
}

bool word_space_node::need_reread(bool *)
{
  if (unformat) {
    for (width_list *w = orig_width; w != 0 /* nullptr */; w = w->next)
      curenv->space(w->width, w->sentence_width);
    unformat = 0;
    return true;
  }
  return false;
}

bool unbreakable_space_node::need_reread(bool *)
{
  return false;
}

bool hmotion_node::need_reread(bool *)
{
  if (unformat && was_tab) {
    curenv->advance_to_tab_stop();
    unformat = 0;
    return true;
  }
  return false;
}

static int leading_spaces_number = 0;
static int leading_spaces_space = 0;

void process_input_stack()
{
  std::stack<int> trap_bol_stack;
  bool reading_beginning_of_input_line = true;
  for (;;) {
    bool ignore_next_token = false;
    switch (tok.type) {
    case token::TOKEN_CHAR:
      {
	unsigned char ch = tok.c;
	if (reading_beginning_of_input_line && !have_formattable_input
	    && (curenv->get_control_character() == ch
		|| curenv->get_no_break_control_character() == ch)) {
	  was_invoked_with_regular_control_character
	    = (curenv->get_control_character() == ch);
	  // skip tabs as well as spaces here
	  do {
	    tok.next();
	  } while (tok.is_horizontal_whitespace());
	  symbol nm = read_identifier();
#if defined(DEBUGGING)
	  if (want_html_debugging) {
	    if (!nm.is_null()) {
	      if (strcmp(nm.contents(), "test") == 0) {
		fprintf(stderr, "found it!\n");
		fflush(stderr);
	      }
	      fprintf(stderr, "interpreting [%s]", nm.contents());
	      if (strcmp(nm.contents(), "di") == 0 && topdiv != curdiv)
		fprintf(stderr, " currently in diversion: %s",
			curdiv->get_diversion_name());
	      fprintf(stderr, "\n");
	      fflush(stderr);
	    }
	  }
#endif
	  if (nm.is_null())
	    skip_line();
	  else {
	    interpolate_macro(nm);
#if defined(DEBUGGING)
	    if (want_html_debugging) {
	      fprintf(stderr, "finished interpreting [%s] and environment state is\n", nm.contents());
	      curenv->dump_troff_state();
	    }
#endif
	  }
	  ignore_next_token = true;
	}
	else {
	  if (possibly_handle_first_page_transition())
	    ;
	  else {
	    for (;;) {
#if defined(DEBUGGING)
	      if (want_html_debugging) {
		fprintf(stderr, "found [%c]\n", ch); fflush(stderr);
	      }
#endif
	      if (curenv->get_was_line_interrupted())
		warning(WARN_SYNTAX, "ignoring %1 on input line after"
			" output line continuation escape sequence",
			tok.description());
	      else
		curenv->add_char(charset_table[ch]);
	      tok.next();
	      if (tok.type != token::TOKEN_CHAR)
		break;
	      ch = tok.c;
	    }
	    ignore_next_token = true;
	    reading_beginning_of_input_line = false;
	  }
	}
	break;
      }
    case token::TOKEN_TRANSPARENT:
      {
	if (reading_beginning_of_input_line) {
	  if (possibly_handle_first_page_transition())
	    ;
	  else {
	    int cc;
	    do {
	      node *n;
	      cc = read_char_in_copy_mode(&n);
	      if (cc != EOF) {
		if (cc != '\0')
		  curdiv->transparent_output(transparent_translate(cc));
		else
		  curdiv->transparent_output(n);
	      }
	    } while (cc != '\n' && cc != EOF);
	    if (cc == EOF)
	      curdiv->transparent_output('\n');
	  }
	}
	break;
      }
    case token::TOKEN_NEWLINE:
      {
	if (reading_beginning_of_input_line
	    && !have_formattable_input_on_interrupted_line
	    && !curenv->get_was_previous_line_interrupted())
	  trapping_blank_line();
	else {
	  curenv->newline();
	  reading_beginning_of_input_line = true;
	}
	break;
      }
    case token::TOKEN_REQUEST:
      {
	int request_code = tok.c;
	tok.next();
	switch (request_code) {
	case TITLE_REQUEST:
	  title();
	  break;
	case COPY_FILE_REQUEST:
	  unsafe_transparent_throughput_file_request();
	  break;
	case TRANSPARENT_FILE_REQUEST:
	  transparent_throughput_file_request();
	  break;
#ifdef COLUMN
	case VJUSTIFY_REQUEST:
	  vjustify();
	  break;
#endif /* COLUMN */
	default:
	  assert(0 == "unhandled case of `request_code` (int)");
	  break;
	}
	ignore_next_token = true;
	break;
      }
    case token::TOKEN_SPACE:
      {
	if (curenv->get_was_line_interrupted())
	  warning(WARN_SYNTAX, "ignoring %1 on input line after"
		  " output line continuation escape sequence",
		  tok.description());
	else if (possibly_handle_first_page_transition())
	  ;
	else if (reading_beginning_of_input_line
		 && !curenv->get_was_previous_line_interrupted()) {
	  int nspaces = 0;
	  // save space_width now so that it isn't changed by \f or \s
	  // which we wouldn't notice here
	  hunits space_width = curenv->get_space_width();
	  do {
	    nspaces += tok.nspaces();
	    tok.next();
	  } while (tok.is_space());
	  if (tok.is_newline())
	    trapping_blank_line();
	  else {
	    push_token(tok);
	    leading_spaces_number = nspaces;
	    leading_spaces_space = space_width.to_units() * nspaces;
	    if (!leading_spaces_macro_name.is_null())
	      spring_trap(leading_spaces_macro_name);
	    else {
	      curenv->do_break();
	      curenv->add_node(new hmotion_node(space_width * nspaces,
						curenv->get_fill_color()));
	    }
	    reading_beginning_of_input_line = false;
	  }
	}
	else {
	  curenv->space();
	  reading_beginning_of_input_line = false;
	}
	break;
      }
    case token::TOKEN_EOF:
      return;
    case token::TOKEN_NODE:
    case token::TOKEN_DELIMITED_HORIZONTAL_MOTION:
    case token::TOKEN_HORIZONTAL_MOTION:
      if (curenv->get_was_line_interrupted()) {
	// We don't want to warn about node types.  They might have been
	// interpolated into the input by the formatter itself, as with
	// the extra vertical space nodes appended to diversions.
	if ((token::TOKEN_HORIZONTAL_MOTION == tok.type)
	    || (token::TOKEN_DELIMITED_HORIZONTAL_MOTION == tok.type))
	  warning(WARN_SYNTAX, "ignoring %1 on input line after"
		  " output line continuation escape sequence",
		  tok.description());
      }
      else if (possibly_handle_first_page_transition())
	;
      else if (tok.nd->need_reread(&reading_beginning_of_input_line)) {
	delete tok.nd;
	tok.nd = 0;
      }
      else {
	curenv->add_node(tok.nd);
	tok.nd = 0;
	reading_beginning_of_input_line = false;
	curenv->possibly_break_line(true /* must break here */);
      }
      break;
    case token::TOKEN_PAGE_EJECTOR:
      {
	continue_page_eject();
	// I think we just want to preserve bol.
	// reading_beginning_of_input_line = true;
	break;
      }
    case token::TOKEN_BEGIN_TRAP:
      {
	trap_bol_stack.push(reading_beginning_of_input_line);
	reading_beginning_of_input_line = true;
	have_formattable_input = false;
	break;
      }
    case token::TOKEN_END_TRAP:
      {
	if (trap_bol_stack.empty())
	  error("spurious end trap token detected!");
	else {
	  reading_beginning_of_input_line = trap_bol_stack.top();
	  trap_bol_stack.pop();
	}
	have_formattable_input = false;

	/* I'm not totally happy about this.  But I can't think of any other
	  way to do it.  Doing an output_pending_lines() whenever a
	  TOKEN_END_TRAP is detected doesn't work: for example,

	  .wh -1i x
	  .de x
	  'bp
	  ..
	  .wh -.5i y
	  .de y
	  .tl ''-%-''
	  ..
	  .br
	  .ll .5i
	  .sp |\n(.pu-1i-.5v
	  a\%very\%very\%long\%word

	  will print all but the first lines from the word immediately
	  after the footer, rather than on the next page. */

	if (trap_bol_stack.empty())
	  curenv->output_pending_lines();
	break;
      }
    case token::TOKEN_INDEXED_CHAR:
    case token::TOKEN_SPECIAL_CHAR:
    case token::TOKEN_DELIMITED_SPECIAL_CHAR:
      if (curenv->get_was_line_interrupted())
	warning(WARN_SYNTAX, "ignoring %1 on input line after output"
		" line continuation escape sequence",
		tok.description());
      else {
	reading_beginning_of_input_line = false;
	tok.process();
      }
      break;
    default:
      {
	reading_beginning_of_input_line = false;
	tok.process();
	break;
      }
    }
    if (!ignore_next_token)
      tok.next();
    was_trap_sprung = false;
  }
}

#ifdef WIDOW_CONTROL

void flush_pending_lines()
{
  while (!tok.is_newline() && !tok.is_eof())
    tok.next();
  curenv->output_pending_lines();
  tok.next();
}

#endif /* WIDOW_CONTROL */

request_or_macro::request_or_macro()
{
}

macro *request_or_macro::to_macro()
{
  return 0 /* nullptr */;
}

request::request(REQUEST_FUNCP pp) : p(pp)
{
}

void request::invoke(symbol, bool)
{
  (*p)();
}

struct char_block {
  enum { SIZE = 128 };
  unsigned char s[SIZE];
  char_block *next;
  char_block();
};

char_block::char_block()
: next(0)
{
}

class char_list {
public:
  char_list();
  ~char_list();
  void append(unsigned char);
  void set(unsigned char, int);
  unsigned char get(int);
  int length();
private:
  unsigned char *ptr;
  int len;
  char_block *head;
  char_block *tail;
  friend class macro_header;
  friend class string_iterator;
};

char_list::char_list()
: ptr(0), len(0), head(0), tail(0)
{
}

char_list::~char_list()
{
  while (head != 0) {
    char_block *tem = head;
    head = head->next;
    delete tem;
  }
}

int char_list::length()
{
  return len;
}

void char_list::append(unsigned char c)
{
  if (tail == 0) {
    head = tail = new char_block;
    ptr = tail->s;
  }
  else {
    if (ptr >= tail->s + char_block::SIZE) {
      tail->next = new char_block;
      tail = tail->next;
      ptr = tail->s;
    }
  }
  *ptr++ = c;
  len++;
}

void char_list::set(unsigned char c, int offset)
{
  assert(len > offset);
  // optimization for access at the end
  int boundary = len - len % char_block::SIZE;
  if (offset >= boundary) {
    *(tail->s + offset - boundary) = c;
    return;
  }
  char_block *tem = head;
  int l = 0;
  for (;;) {
    l += char_block::SIZE;
    if (l > offset) {
      *(tem->s + offset % char_block::SIZE) = c;
      return;
    }
    tem = tem->next;
  }
}

unsigned char char_list::get(int offset)
{
  assert(len > offset);
  // optimization for access at the end
  int boundary = len - len % char_block::SIZE;
  if (offset >= boundary)
    return *(tail->s + offset - boundary);
  char_block *tem = head;
  int l = 0;
  for (;;) {
    l += char_block::SIZE;
    if (l > offset)
      return *(tem->s + offset % char_block::SIZE);
    tem = tem->next;
  }
}

class node_list {
  node *head;
  node *tail;
public:
  node_list();
  ~node_list();
  void append(node *);
  int length();
  node *extract();

  friend class macro_header;
  friend class string_iterator;
};

void node_list::append(node *n)
{
  if (head == 0 /* nullptr */) {
    n->next = 0 /* nullptr */;
    head = tail = n;
  }
  else {
    n->next = 0 /* nullptr */;
    tail = tail->next = n;
  }
}

int node_list::length()
{
  int total = 0 /* nullptr */;
  for (node *n = head; n != 0 /* nullptr */; n = n->next)
    ++total;
  return total;
}

node_list::node_list()
{
  head = tail = 0 /* nullptr */;
}

node *node_list::extract()
{
  node *temp = head;
  head = tail = 0 /* nullptr */;
  return temp;
}

node_list::~node_list()
{
  delete_node_list(head);
}

class macro_header {
public:
  int count;
  char_list cl;
  node_list nl;
  macro_header() { count = 1; }
  macro_header *copy(int);
  void json_dump_macro();
  void json_dump_diversion();
};

macro::~macro()
{
  if (p != 0 /* nullptr */ && --(p->count) <= 0)
    delete p;
}

macro::macro()
: is_a_diversion(false), is_a_string(true)
{
  if (!input_stack::get_location(true /* allow macro */, &filename,
				 &lineno)) {
    filename = 0 /* nullptr */;
    lineno = 0 /* nullptr */;
  }
  len = 0;
  is_empty_macro = true;
  p = 0; /* nullptr */
}

macro::macro(const macro &m)
: filename(m.filename), lineno(m.lineno), len(m.len),
  is_empty_macro(m.is_empty_macro), is_a_diversion(m.is_a_diversion),
  is_a_string(m.is_a_string), p(m.p)
{
  if (p != 0 /* nullptr */)
    p->count++;
}

macro::macro(bool is_div)
: is_a_diversion(is_div)
{
  if (!input_stack::get_location(true /* allow macro */, &filename,
				 &lineno)) {
    filename = 0 /* nullptr */;
    lineno = 0 /* nullptr */;
  }
  len = 0;
  is_empty_macro = true;
  // A macro is a string until it contains a newline.
  is_a_string = true;
  p = 0 /* nullptr */;
}

bool macro::is_diversion()
{
  return is_a_diversion;
}

bool macro::is_string()
{
  return is_a_string;
}

void macro::clear_string_flag()
{
  is_a_string = false;
}

macro &macro::operator=(const macro &m)
{
  // don't assign object
  if (m.p != 0 /* nullptr */)
    m.p->count++;
  if (p != 0 /* nullptr */ && --(p->count) <= 0)
    delete p;
  p = m.p;
  filename = m.filename;
  lineno = m.lineno;
  len = m.len;
  is_empty_macro = m.is_empty_macro;
  is_a_diversion = m.is_a_diversion;
  is_a_string = m.is_a_string;
  return *this;
}

void macro::append(unsigned char c)
{
  assert(c != 0);
  if (p == 0 /* nullptr */)
    p = new macro_header;
  if (p->cl.length() != len) {
    macro_header *tem = p->copy(len);
    if (--(p->count) <= 0)
      delete p;
    p = tem;
  }
  p->cl.append(c);
  ++len;
  if (c != PUSH_GROFF_MODE && c != PUSH_COMP_MODE && c != POP_GROFFCOMP_MODE)
    is_empty_macro = false;
}

void macro::set(unsigned char c, int offset)
{
  assert(p != 0 /* nullptr */);
  assert(c != 0);
  p->cl.set(c, offset);
}

unsigned char macro::get(int offset)
{
  assert(p != 0 /* nullptr */);
  return p->cl.get(offset);
}

int macro::length()
{
  return len;
}

void macro::append_str(const char *s)
{
  int i = 0;

  if (s != 0 /* nullptr */) {
    while (s[i] != '\0') {
      append(s[i]);
      i++;
    }
  }
}

void macro::append(node *n)
{
  assert(n != 0 /* nullptr */);
  if (p == 0 /* nullptr */)
    p = new macro_header;
  if (p->cl.length() != len) {
    macro_header *tem = p->copy(len);
    if (--(p->count) <= 0)
      delete p;
    p = tem;
  }
  p->cl.append(0);
  p->nl.append(n);
  ++len;
  is_empty_macro = false;
}

void macro::append_unsigned(unsigned int i)
{
  unsigned int j = i / 10;
  if (j != 0)
    append_unsigned(j);
  append(((unsigned char)(((int)'0') + i % 10)));
}

void macro::append_int(int i)
{
  if (i < 0) {
    append('-');
    i = -i;
  }
  append_unsigned((unsigned int) i);
}

void macro::print_size()
{
  errprint("%1", len);
}

// Use this only for zero-length macros associated with charinfo objects
// that are character classes.
void macro::dump()
{
  if (filename != 0 /* nullptr */)
    errprint("file name: \"%1\", line number: %2\n", filename, lineno);
}

void macro::json_dump()
{
  bool need_comma = false;
  // XXX: Unfortunately, if you alias or rename a request, the location
  // of its invocation site is used for location information instead of
  // its true origin.
  if (filename != 0 /* nullptr */) {
    symbol fn(filename); // `symbol` because it can't contain nulls.
    const char *jsonfn = fn.json_extract();
    errprint("\"file name\": %1", jsonfn);
    free(const_cast<char *>(jsonfn));
    fflush(stderr);
    errprint(", \"starting line number\": %1", lineno);
    need_comma = true;
  }
  if (need_comma)
    errprint(", ");
  errprint("\"length\": %1", len);
  if (p != 0 /* nullptr */) {
    errprint(", ");
    p->json_dump_macro();
    errprint(", ");
    p->json_dump_diversion();
  }
}

// make a copy of the first n bytes

macro_header *macro_header::copy(int n)
{
  macro_header *p = new macro_header;
  char_block *bp = cl.head;
  unsigned char *ptr = bp->s;
  node *nd = nl.head;
  while (--n >= 0) {
    if (ptr >= bp->s + char_block::SIZE) {
      bp = bp->next;
      ptr = bp->s;
    }
    unsigned char c = *ptr++;
    p->cl.append(c);
    if (c == 0) {
      p->nl.append(nd->copy());
      nd = nd->next;
    }
  }
  return p;
}

extern void dump_node_list(node *);

void macro_header::json_dump_diversion()
{
  errprint("\"node list\": ");
  dump_node_list(nl.head);
  fflush(stderr);
}

void macro_header::json_dump_macro()
{
  errprint("\"contents\": \"");
  int macro_len = cl.length();
  for (int i = 0; i < macro_len; i++) {
    json_char jc = json_encode_char(cl.get(i));
    // Write out its JSON representation by character by character to
    // keep libc string functions from interpreting C escape sequences.
    for (size_t j = 0; j < jc.len; j++)
      fputc(jc.buf[j], stderr);
  }
  errprint("\"");
  fflush(stderr);
}

void print_macro_request()
{
  request_or_macro *rm;
  macro *m = 0 /* nullptr */;
  symbol s;
  if (has_arg()) {
    do {
      s = read_identifier();
      if (s.is_null())
	break;
      rm = static_cast<request_or_macro *>(request_dictionary.lookup(s));
      if (rm != 0 /* nullptr */)
	m = rm->to_macro();
      if (m != 0 /* nullptr */) {
	errprint("{\"name\": ");
	s.json_dump();
	errprint(", ");
	m->json_dump();
	errprint("}\n");
	fflush(stderr);
      }
    } while (has_arg());
  }
  else {
    object_dictionary_iterator iter(request_dictionary);
    // We must use the nuclear `reinterpret_cast` operator because GNU
    // troff's dictionary types use a pre-STL approach to containers.
    while (iter.get(&s, reinterpret_cast<object **>(&rm))) {
      assert(!s.is_null());
      m = rm->to_macro();
      if (m != 0 /* nullptr */) {
	errprint("%1\t", s.contents());
	m->print_size();
	errprint("\n");
      }
    }
  }
  fflush(stderr);
  skip_line();
}

class string_iterator : public input_iterator {
  macro mac;
  const char *how_invoked;
  bool seen_newline;
  int lineno;
  char_block *bp;
  int count;			// of characters remaining
  node *nd;
  bool att_compat;
  bool with_break;		// inherited from the caller
protected:
  symbol nm;
  string_iterator();
public:
  string_iterator(const macro &, const char * = 0 /* nullptr */,
		  symbol = NULL_SYMBOL);
  int fill(node **);
  int peek();
  bool get_location(bool /* allow_macro */, const char ** /* filep */,
		    int * /* linep */);
  void backtrace();
  bool get_break_flag() { return with_break; }
  void set_att_compat(bool b) { att_compat = b; }
  bool get_att_compat() { return att_compat; }
  bool is_diversion();
};

string_iterator::string_iterator(const macro &m, const char *p,
    symbol s)
: input_iterator(m.is_a_diversion), mac(m), how_invoked(p),
  seen_newline(false), lineno(1), nm(s)
{
  count = mac.len;
  if (count != 0) {
    bp = mac.p->cl.head;
    nd = mac.p->nl.head;
    ptr = endptr = bp->s;
  }
  else {
    bp = 0 /* nullptr */;
    nd = 0 /* nullptr */;
    ptr = endptr = 0 /* nullptr */;
  }
  with_break = input_stack::get_break_flag();
}

string_iterator::string_iterator()
{
  bp = 0 /* nullptr */;
  nd = 0 /* nullptr */;
  ptr = endptr = 0 /* nullptr */;
  seen_newline = false;
  how_invoked = 0 /* nullptr */;
  lineno = 1;
  count = 0;
  with_break = input_stack::get_break_flag();
}

bool string_iterator::is_diversion()
{
  return mac.is_diversion();
}

int string_iterator::fill(node **np)
{
  if (seen_newline)
    lineno++;
  seen_newline = false;
  if (count <= 0)
    return EOF;
  const unsigned char *p = endptr;
  if (p >= bp->s + char_block::SIZE) {
    bp = bp->next;
    p = bp->s;
  }
  if (*p == '\0') {
    if (np != 0 /* nullptr */) {
      *np = nd->copy();
      if (is_diversion())
	(*np)->div_nest_level = input_stack::get_div_level();
      else
	(*np)->div_nest_level = 0;
    }
    nd = nd->next;
    endptr = ptr = p + 1;
    count--;
    return 0;
  }
  const unsigned char *e = bp->s + char_block::SIZE;
  if (e - p > count)
    e = p + count;
  ptr = p;
  while (p < e) {
    unsigned char c = *p;
    if (c == '\n' || c == ESCAPE_NEWLINE) {
      seen_newline = true;
      p++;
      break;
    }
    if (c == '\0')
      break;
    p++;
  }
  endptr = p;
  count -= p - ptr;
  return *ptr++;
}

int string_iterator::peek()
{
  if (count <= 0)
    return EOF;
  const unsigned char *p = endptr;
  if (p >= bp->s + char_block::SIZE) {
    p = bp->next->s;
  }
  return *p;
}

bool string_iterator::get_location(bool allow_macro,
				   const char **filep, int *linep)
{
  if (!allow_macro)
    return false;
  if (0 /* nullptr */ == mac.filename)
    return false;
  *filep = mac.filename;
  *linep = mac.lineno + lineno - 1;
  return true;
}

void string_iterator::backtrace()
{
  if (mac.filename != 0 /* nullptr */) {
    if (program_name != 0 /* nullptr */)
      errprint("%1: ", program_name);
    errprint("backtrace: '%1':%2", mac.filename,
	     (mac.lineno + lineno - 1));
    if (how_invoked != 0 /* nullptr */) {
      if (!nm.is_null())
	errprint(": %1 '%2'", how_invoked, nm.contents());
      else
	errprint(": %1", how_invoked);
    }
    errprint("\n");
  }
}

class temp_iterator : public input_iterator {
  unsigned char *base;
  temp_iterator(const char *, int len);
public:
  ~temp_iterator();
  friend input_iterator *make_temp_iterator(const char *);
};

inline temp_iterator::temp_iterator(const char *s, int len)
: base(0 /* nullptr */)
{
  if (len > 0) {
    base = new unsigned char[len + 1];
    (void) memcpy(base, s, len);
    base[len] = '\0';
    ptr = base;
    endptr = base + len;
  }
}

temp_iterator::~temp_iterator()
{
  delete[] base;
}


input_iterator *make_temp_iterator(const char *s)
{
  if (0 /* nullptr */ == s)
    return new temp_iterator(s, 0);
  else {
    size_t n = strlen(s);
    return new temp_iterator(s, n);
  }
}

// this is used when macros with arguments are interpolated

struct arg_list {
  macro mac;
  bool space_follows;
  arg_list *next;
  arg_list(const macro &, bool);
  arg_list(const arg_list *);
  ~arg_list();
};

arg_list::arg_list(const macro &m, bool b)
: mac(m), space_follows(b), next(0 /* nullptr */)
{
}

arg_list::arg_list(const arg_list *al)
: next(0 /* nullptr */)
{
  mac = al->mac;
  space_follows = al->space_follows;
  arg_list **a = &next;
  arg_list *p = al->next;
  while (p != 0 /* nullptr */) {
    *a = new arg_list(p->mac, p->space_follows);
    p = p->next;
    a = &(*a)->next;
  }
}

arg_list::~arg_list()
{
}

class macro_iterator : public string_iterator {
  arg_list *args;
  int argc;
  bool with_break;		// whether called as .foo or 'foo
public:
  macro_iterator(symbol, macro &,
		 const char * /* how_called */ = "macro",
		 bool /* want_arguments_initialized */ = false);
  macro_iterator();
  ~macro_iterator();
  bool has_args() { return true; }
  input_iterator *get_arg(int);
  arg_list *get_arg_list();
  symbol get_macro_name();
  bool space_follows_arg(int);
  bool get_break_flag() { return with_break; }
  int nargs() { return argc; }
  void add_arg(const macro &, int);
  void shift(int);
  bool is_macro() { return true; }
  bool is_diversion();
};

input_iterator *macro_iterator::get_arg(int i)
{
  if (i == 0)
    return make_temp_iterator(nm.contents());
  if (i > 0 && i <= argc) {
    arg_list *p = args;
    for (int j = 1; j < i; j++) {
      assert(p != 0);
      p = p->next;
    }
    return new string_iterator(p->mac);
  }
  else
    return 0 /* nullptr */;
}

arg_list *macro_iterator::get_arg_list()
{
  return args;
}

symbol macro_iterator::get_macro_name()
{
  return nm;
}

bool macro_iterator::space_follows_arg(int i)
{
  if ((i > 0) && (i <= argc)) {
    arg_list *p = args;
    for (int j = 1; j < i; j++) {
      assert(p != 0 /* nullptr */);
      p = p->next;
    }
    return p->space_follows;
  }
  else
    return false;
}

void macro_iterator::add_arg(const macro &m, int s)
{
  arg_list **p;
  for (p = &args; *p != 0 /* nullptr */; p = &((*p)->next))
    ;
  *p = new arg_list(m, s);
  ++argc;
}

void macro_iterator::shift(int n)
{
  while (n > 0 && argc > 0) {
    arg_list *tem = args;
    args = args->next;
    delete tem;
    --argc;
    --n;
  }
}

// This gets used by, e.g., .if '\?xxx\?''.

bool operator==(const macro &m1, const macro &m2)
{
  if (m1.len != m2.len)
    return false;
  string_iterator iter1(m1);
  string_iterator iter2(m2);
  int n = m1.len;
  while (--n >= 0) {
    node *nd1 = 0;
    int c1 = iter1.get(&nd1);
    assert(c1 != EOF);
    node *nd2 = 0;
    int c2 = iter2.get(&nd2);
    assert(c2 != EOF);
    if (c1 != c2) {
      if (c1 == 0)
	delete nd1;
      else if (c2 == 0)
	delete nd2;
      return false;
    }
    if (c1 == 0) {
      assert(nd1 != 0);
      assert(nd2 != 0);
      bool same = nd1->type() == nd2->type() && nd1->is_same_as(nd2);
      delete nd1;
      delete nd2;
      return same;
    }
  }
  return true;
}

static void interpolate_macro(symbol nm, bool do_not_want_next_token)
{
  request_or_macro *p
    = static_cast<request_or_macro *>(request_dictionary.lookup(nm));
  if (0 /* nullptr */ == p) {
    bool was_warned = false;
    const char *s = nm.contents();
    if (strlen(s) > 2) {
      request_or_macro *r;
      char buf[3];
      buf[0] = s[0];
      buf[1] = s[1];
      buf[2] = '\0';
      r = static_cast<request_or_macro *>
	  (request_dictionary.lookup(symbol(buf)));
      if (r != 0 /* nullptr */) {
	macro *m = r->to_macro();
	if ((0 /* nullptr */ == m) || !m->is_empty()) {
	  warning(WARN_SPACE, "name '%1' not defined (possibly missing"
		  " space after '%2')", nm.contents(), buf);
	  was_warned = true;
	}
      }
    }
    if (!was_warned) {
      warning(WARN_MAC, "name '%1' not defined", nm.contents());
      p = new macro;
      request_dictionary.define(nm, p);
    }
  }
  if (p != 0 /* nullptr */)
    p->invoke(nm, do_not_want_next_token);
  else {
    skip_line();
    return;
  }
}

static void decode_macro_call_arguments(macro_iterator *mi)
{
  if (!tok.is_newline() && !tok.is_eof()) {
    node *n;
    int c = read_char_in_copy_mode(&n);
    for (;;) {
      while (c == ' ')
	c = read_char_in_copy_mode(&n);
      if (c == '\n' || c == EOF)
	break;
      macro arg;
      int quote_input_level = 0;
      bool was_warned = false; // about an input tab character
      arg.append(want_att_compat ? PUSH_COMP_MODE : PUSH_GROFF_MODE);
      // we store discarded double quotes for \$^
      if (c == '"') {
	arg.append(DOUBLE_QUOTE);
	quote_input_level = input_stack::get_level();
	c = read_char_in_copy_mode(&n);
      }
      while (c != EOF && c != '\n'
	     && !(c == ' ' && quote_input_level == 0)) {
	if (quote_input_level > 0 && c == '"'
	    && (want_att_compat
		|| input_stack::get_level() == quote_input_level)) {
	  arg.append(DOUBLE_QUOTE);
	  c = read_char_in_copy_mode(&n);
	  if (c == '"') {
	    arg.append(c);
	    c = read_char_in_copy_mode(&n);
	  }
	  else
	    break;
	}
	else {
	  if (c == 0)
	    arg.append(n);
	  else {
	    if (c == '\t' && quote_input_level == 0 && !was_warned) {
	      warning(WARN_TAB, "tab character in unquoted macro"
		      " argument");
	      was_warned = true;
	    }
	    arg.append(c);
	  }
	  c = read_char_in_copy_mode(&n);
	}
      }
      arg.append(POP_GROFFCOMP_MODE);
      mi->add_arg(arg, (c == ' '));
    }
  }
}

static void decode_escape_sequence_arguments(macro_iterator *mi)
{
  node *n;
  int c = read_char_in_copy_mode(&n);
  for (;;) {
    while (c == ' ')
      c = read_char_in_copy_mode(&n);
    if (c == '\n' || c == EOF) {
      error("missing ']' in parameterized escape sequence");
      break;
    }
    if (c == ']')
      break;
    macro arg;
    int quote_input_level = 0;
    bool was_warned = false; // about an input tab character
    if (c == '"') {
      quote_input_level = input_stack::get_level();
      c = read_char_in_copy_mode(&n);
    }
    while (c != EOF && c != '\n'
	   && !(c == ']' && quote_input_level == 0)
	   && !(c == ' ' && quote_input_level == 0)) {
      if (quote_input_level > 0 && c == '"'
	  && input_stack::get_level() == quote_input_level) {
	c = read_char_in_copy_mode(&n);
	if (c == '"') {
	  arg.append(c);
	  c = read_char_in_copy_mode(&n);
	}
	else
	  break;
      }
      else {
	if (c == 0)
	  arg.append(n);
	else {
	  if (c == '\t' && quote_input_level == 0 && !was_warned)
	  {
	    warning(WARN_TAB, "tab character in parameterized escape"
		    " sequence");
	    was_warned = true;
	  }
	  arg.append(c);
	}
	c = read_char_in_copy_mode(&n);
      }
    }
    mi->add_arg(arg, (c == ' '));
  }
}

void macro::invoke(symbol nm, bool do_not_want_next_token)
{
  macro_iterator *mi = new macro_iterator(nm, *this);
  decode_macro_call_arguments(mi);
  input_stack::push(mi);
  // we must delay tok.next() in case the function has been called by
  // do_request to assure proper handling of want_att_compat
  if (!do_not_want_next_token)
    tok.next();
}

macro *macro::to_macro()
{
  return this;
}

bool macro::is_empty()
{
  return (is_empty_macro == true);
}

macro_iterator::macro_iterator(symbol s, macro &m,
			       const char *how_called,
			       bool want_arguments_initialized)
: string_iterator(m, how_called, s), args(0 /* nullptr */), argc(0),
  with_break(was_invoked_with_regular_control_character)
{
  if (want_arguments_initialized) {
    arg_list *al = input_stack::get_arg_list();
    if (al != 0 /* nullptr */) {
      args = new arg_list(al);
      argc = input_stack::nargs();
    }
  }
}

macro_iterator::macro_iterator()
: args(0 /* nullptr */), argc(0),
  with_break(was_invoked_with_regular_control_character)
{
}

macro_iterator::~macro_iterator()
{
  while (args != 0 /* nullptr */) {
    arg_list *tem = args;
    args = args->next;
    delete tem;
  }
}

dictionary composite_dictionary(17);

static void map_composite_character()
{
  symbol from = read_identifier();
  if (from.is_null()) {
    warning(WARN_MISSING, "composite character mapping request expects"
	    " arguments");
    skip_line();
    return;
  }
  const char *fc = from.contents();
  const char *from_gn = glyph_name_to_unicode(fc);
  char errbuf[ERRBUFSZ]; // C++03: char errbuf[ERRBUFSZ]()
  if (0 /* nullptr */ == from_gn) {
    from_gn = valid_unicode_code_sequence(fc, errbuf);
    if (0 /* nullptr */ == from_gn) {
      error("invalid composite glyph name '%1': %2", fc, errbuf);
      skip_line();
      return;
    }
  }
  const char *from_decomposed = decompose_unicode(from_gn);
  if (from_decomposed != 0 /* nullptr */)
    from_gn = &from_decomposed[1];
  symbol to = read_identifier();
  if (to.is_null()) {
    composite_dictionary.remove(symbol(from_gn));
    skip_line();
    return;
  }
  const char *tc = to.contents();
  const char *to_gn = glyph_name_to_unicode(tc);
  if (0 /* nullptr */ == to_gn) {
    to_gn = valid_unicode_code_sequence(tc, errbuf);
    if (0 /* nullptr */ == to_gn) {
      error("invalid composite glyph name '%1': %2", tc, errbuf);
      skip_line();
      return;
    }
  }
  const char *to_decomposed = decompose_unicode(to_gn);
  if (to_decomposed != 0 /* nullptr */)
    to_gn = &to_decomposed[1];
  if (strcmp(from_gn, to_gn) == 0)
    composite_dictionary.remove(symbol(from_gn));
  else
    (void) composite_dictionary.lookup(symbol(from_gn), (void *) to_gn);
  skip_line();
}

static symbol composite_glyph_name(symbol nm)
{
  macro_iterator *mi = new macro_iterator();
  decode_escape_sequence_arguments(mi);
  input_stack::push(mi);
  const char *nc = nm.contents();
  const char *gn = glyph_name_to_unicode(nc);
  if (0 /* nullptr */ == gn) {
    gn = valid_unicode_code_sequence(nc);
    if (0 /* nullptr */ == gn) {
      error("invalid base character '%1' in composite character name",
	    nc);
      return EMPTY_SYMBOL;
    }
  }
  const char *gn_decomposed = decompose_unicode(gn);
  string glyph_name(gn_decomposed ? &gn_decomposed[1] : gn);
  string gl;
  int n = input_stack::nargs();
  for (int i = 1; i <= n; i++) {
    glyph_name += '_';
    input_iterator *p = input_stack::get_arg(i);
    gl.clear();
    int c;
    while ((c = p->get(0)) != EOF)
      if (c != DOUBLE_QUOTE)
	gl += c;
    gl += '\0';
    const char *gc = gl.contents();
    const char *u = glyph_name_to_unicode(gc);
    if (0 /* nullptr */ == u) {
      u = valid_unicode_code_sequence(gc);
      if (0 /* nullptr */ == u) {
	error("invalid component '%1' in composite glyph name", gc);
	return EMPTY_SYMBOL;
      }
    }
    const char *decomposed = decompose_unicode(u);
    if (decomposed != 0 /* nullptr */)
      u = &decomposed[1];
    void *mapped_composite = composite_dictionary.lookup(symbol(u));
    if (mapped_composite != 0 /* nullptr */)
      u = static_cast<const char *>(mapped_composite);
    glyph_name += u;
  }
  glyph_name += '\0';
  const char *groff_gn = unicode_to_glyph_name(glyph_name.contents());
  if (groff_gn != 0 /* nullptr */)
    return symbol(groff_gn);
  gl.clear();
  gl += 'u';
  gl += glyph_name;
  return symbol(gl.contents());
}

static void print_composite_character_request()
{
  dictionary_iterator iter(composite_dictionary);
  symbol key;
  char *value;
  // We must use the nuclear `reinterpret_cast` operator because GNU
  // troff's dictionary types use a pre-STL approach to containers.
  while (iter.get(&key, reinterpret_cast<void **>(&value))) {
    assert(!key.is_null());
    assert(value != 0 /* nullptr */);
    errprint("%1\t%2\n", key.contents(), value);
  }
  fflush(stderr);
  skip_line();
}

bool was_trap_sprung = false;
static bool are_traps_postponed = false;
symbol postponed_trap;

void spring_trap(symbol nm)
{
  assert(!nm.is_null());
  was_trap_sprung = true;
  if (are_traps_postponed) {
    postponed_trap = nm;
    return;
  }
  static char buf[2] = { BEGIN_TRAP, '\0' };
  static char buf2[2] = { END_TRAP, '\0' };
  input_stack::push(make_temp_iterator(buf2));
  request_or_macro *p = lookup_request(nm);
  // We don't perform this validation at the time the trap is planted
  // because a request name might be replaced by a macro by the time the
  // trap springs.
  macro *m = p->to_macro();
  if (m != 0 /* nullptr */)
    input_stack::push(new macro_iterator(nm, *m, "trap-called macro"));
  else
    error("trap failed to spring: '%1' is a request", nm.contents());
  input_stack::push(make_temp_iterator(buf));
}

void postpone_traps()
{
  are_traps_postponed = true;
}

bool unpostpone_traps()
{
  are_traps_postponed = false;
  if (!postponed_trap.is_null()) {
    spring_trap(postponed_trap);
    postponed_trap = NULL_SYMBOL;
    return true;
  }
  else
    return false;
}

void read_request()
{
  macro_iterator *mi = new macro_iterator;
  int reading_from_terminal = isatty(fileno(stdin));
  int had_prompt = 0;
  if (has_arg(true /* peek */)) {
    int c = read_char_in_copy_mode(0 /* nullptr */);
    while (c == ' ')
      c = read_char_in_copy_mode(0 /* nullptr */);
    while (c != EOF && c != '\n' && c != ' ') {
      if (!is_invalid_input_char(c)) {
	if (reading_from_terminal)
	  fputc(c, stderr);
	had_prompt = 1;
      }
      c = read_char_in_copy_mode(0 /* nullptr */);
    }
    if (c == ' ') {
      tok.make_space();
      decode_macro_call_arguments(mi);
    }
  }
  if (reading_from_terminal) {
    fputc(had_prompt ? ':' : '\a', stderr);
    fflush(stderr);
  }
  input_stack::push(mi);
  macro mac;
  int nl = 0;
  int c;
  while ((c = getchar()) != EOF) {
    if (is_invalid_input_char(c))
      warning(WARN_INPUT, "invalid input character code %1", int(c));
    else {
      if (c == '\n') {
	if (nl != 0 /* nullptr */)
	  break;
	else
	  nl = 1;
      }
      else
	nl = 0;
      mac.append(c);
    }
  }
  if (reading_from_terminal)
    clearerr(stdin);
  input_stack::push(new string_iterator(mac));
  tok.next();
}

enum define_mode { DEFINE_NORMAL, DEFINE_APPEND, DEFINE_IGNORE };
enum calling_mode { CALLING_NORMAL, CALLING_INDIRECT };
enum comp_mode { COMP_IGNORE, COMP_DISABLE, COMP_ENABLE };

static void do_define_string(define_mode mode, comp_mode comp)
{
  symbol nm;
  node *n = 0 /* nullptr */;
  int c;
  nm = read_identifier(true /* required */);
  if (nm.is_null()) {
    skip_line();
    return;
  }
  if (tok.is_newline())
    c = '\n';
  else if (tok.is_tab())
    c = '\t';
  else if (!tok.is_space()) {
    skip_line();
    return;
  }
  else
    c = read_char_in_copy_mode(&n);
  while (c == ' ')
    c = read_char_in_copy_mode(&n);
  if (c == '"')
    c = read_char_in_copy_mode(&n);
  macro mac;
  request_or_macro *rm
    = static_cast<request_or_macro *>(request_dictionary.lookup(nm));
  macro *mm = rm ? rm->to_macro() : 0 /* nullptr */;
  if (mode == DEFINE_APPEND && mm)
    mac = *mm;
  if (comp == COMP_DISABLE)
    mac.append(PUSH_GROFF_MODE);
  else if (comp == COMP_ENABLE)
    mac.append(PUSH_COMP_MODE);
  while (c != '\n' && c != EOF) {
    if (c == 0)
      mac.append(n);
    else
      mac.append((unsigned char) c);
    c = read_char_in_copy_mode(&n);
  }
  if (comp == COMP_DISABLE || comp == COMP_ENABLE)
    mac.append(POP_GROFFCOMP_MODE);
  if (!mm) {
    mm = new macro;
    request_dictionary.define(nm, mm);
  }
  *mm = mac;
  tok.next();
}

static void define_string()
{
  do_define_string(DEFINE_NORMAL,
		   want_att_compat ? COMP_ENABLE : COMP_IGNORE);
}

static void define_nocomp_string()
{
  do_define_string(DEFINE_NORMAL, COMP_DISABLE);
}

static void append_string()
{
  do_define_string(DEFINE_APPEND,
		   want_att_compat ? COMP_ENABLE : COMP_IGNORE);
}

static void append_nocomp_string()
{
  do_define_string(DEFINE_APPEND, COMP_DISABLE);
}

static const char *character_mode_description(char_mode mode)
{
  // C++11: There may be a better way to do this with an enum class;
  // we could then store these string literals inside `char_mode`.
  const char *modestr = 0 /* nullptr */;
  switch (mode) {
    case CHAR_NORMAL:
      modestr = "";
      break;
    case CHAR_FALLBACK:
      modestr = " fallback";
      break;
    case CHAR_SPECIAL_FALLBACK:
      modestr = " special fallback";
      break;
    case CHAR_FONT_SPECIFIC_FALLBACK:
      modestr = " font-specific fallback";
      break;
    default:
      assert(0 == "unhandled case of character mode");
      break;
  }
  return modestr;
}

void define_character(char_mode mode, const char *font_name)
{
  const char *modestr = character_mode_description(mode);
  tok.skip_spaces();
  charinfo *ci = tok.get_charinfo(true /* required */);
  if (0 /* nullptr */ == ci) {
    assert(0 == "attempted to use token without charinfo in character"
	   " definition request");
    skip_line();
    return;
  }
  // TODO: If `ci` is already a character class, clobber it.
  if (font_name != 0 /* nullptr */) {
    string s(font_name);
    s += ' ';
    s += ci->nm.contents();
    s += '\0';
    ci = lookup_charinfo(symbol(s.contents()));
  }
  tok.next();
  int c;
  node *n = 0 /* nullptr */;
  if (tok.is_newline())
    c = '\n';
  else if (tok.is_tab())
    c = '\t';
  else if (!tok.is_space()) {
    error("ignoring invalid%1 character definition; expected an"
	  " ordinary, indexed, or special character to define, got %2",
	  modestr, tok.description());
    skip_line();
    return;
  }
  else
    c = read_char_in_copy_mode(&n);
  while (c == ' ' || c == '\t')
    c = read_char_in_copy_mode(&n);
  if (c == '"')
    c = read_char_in_copy_mode(&n);
  macro *m = new macro;
  // Construct a macro from input characters; if the input character
  // code is 0, we've read a node--append that.
  while (c != '\n' && c != EOF) {
    if (c != 0)
      m->append(static_cast<unsigned char>(c));
    else
      m->append(n);
    c = read_char_in_copy_mode(&n);
  }
  // Assign the macro to the character, discarding any previous macro.
  m = ci->set_macro(m, mode);
  if (m != 0 /* nullptr */)
    delete m;
  tok.next();
}

static void define_character_request()
{
  if (!has_arg(true /* peek; we want to read in copy mode */)) {
    warning(WARN_MISSING, "character definition request expects"
	    " arguments");
    skip_line();
    return;
  }
  define_character(CHAR_NORMAL);
}

static void define_fallback_character_request()
{
  if (!has_arg(true /* peek; we want to read in copy mode */)) {
    warning(WARN_MISSING, "fallback character definition request"
	    " expects arguments");
    skip_line();
    return;
  }
  define_character(CHAR_FALLBACK);
}

static void define_special_character_request()
{
  if (!has_arg(true /* peek; we want to read in copy mode */)) {
    warning(WARN_MISSING, "special character definition request expects"
	    " arguments");
    skip_line();
    return;
  }
  define_character(CHAR_SPECIAL_FALLBACK);
}

static void print_character_request()
{
  if (!has_arg()) {
    warning(WARN_MISSING, "character report request expects arguments");
    skip_line();
    return;
  }
  charinfo *ci;
  do {
    tok.skip_spaces();
    if (tok.is_newline() || tok.is_eof())
      break;
    if (!tok.is_any_character()) {
      error("character report request expects characters or character"
	    " classes as arguments; got %1", tok.description());
      break;
    }
    ci = tok.get_charinfo(false /* required */,
			  true /* suppress creation */);
    if (ci != 0 /* nullptr */) {
      errprint("%1\n", tok.description());
      fflush(stderr);
      ci->dump();
    }
    tok.next();
  } while (true);
  skip_line();
}

static void remove_character()
{
  if (!has_arg()) {
    warning(WARN_MISSING, "character definition removal request expects"
	    " arguments");
    skip_line();
    return;
  }
  while (!tok.is_newline() && !tok.is_eof()) {
    if (!tok.is_space() && !tok.is_tab()) {
      if (tok.is_any_character()) {
	charinfo *ci = tok.get_charinfo(true /* required */,
					true /* suppress creation */);
	if (0 /* nullptr */ == ci)
	   warning(WARN_CHAR, "%1 is not defined", tok.description());
	else {
	  macro *m = ci->set_macro(0 /* nullptr */);
	  if (m != 0 /* nullptr */)
	    delete m;
	}
      }
      else {
	error("cannot remove character; %1 is not a character",
	      tok.description());
	break;
      }
    }
    tok.next();
  }
  skip_line();
}

static void interpolate_string(symbol nm)
{
  request_or_macro *p = lookup_request(nm);
  macro *m = p->to_macro();
  if (0 /* nullptr */ == m)
    error("cannot interpolate request '%1'", nm.contents());
  else {
    if (m->is_string()) {
      string_iterator *si = new string_iterator(*m, "string", nm);
      input_stack::push(si);
     }
    else {
      // if a macro is called as a string, \$0 doesn't get changed
      macro_iterator *mi = new macro_iterator(input_stack::get_macro_name(),
					      *m, "string", 1);
      input_stack::push(mi);
    }
  }
}

static void interpolate_string_with_args(symbol nm)
{
  request_or_macro *p = lookup_request(nm);
  macro *m = p->to_macro();
  if (0 /* nullptr */ == m)
    error("cannot interpolate request '%1'", nm.contents());
  else {
    macro_iterator *mi = new macro_iterator(nm, *m);
    decode_escape_sequence_arguments(mi);
    input_stack::push(mi);
  }
}

static void interpolate_positional_parameter(symbol nm)
{
  const char *s = nm.contents();
  if (0 /* nullptr */ == s || '\0' == *s)
    copy_mode_error("missing positional argument number in copy mode");
  else if (s[1] == 0 && csdigit(s[0]))
    input_stack::push(input_stack::get_arg(s[0] - '0'));
  else if (s[0] == '*' && s[1] == '\0') {
    int limit = input_stack::nargs();
    string args;
    for (int i = 1; i <= limit; i++) {
      input_iterator *p = input_stack::get_arg(i);
      int c;
      while ((c = p->get(0)) != EOF)
	if (c != DOUBLE_QUOTE)
	  args += c;
      if (i != limit)
	args += ' ';
      delete p;
    }
    if (limit > 0) {
      args += '\0';
      input_stack::push(make_temp_iterator(args.contents()));
    }
  }
  else if (s[0] == '@' && s[1] == '\0') {
    int limit = input_stack::nargs();
    string args;
    for (int i = 1; i <= limit; i++) {
      args += '"';
      args += char(BEGIN_QUOTE);
      input_iterator *p = input_stack::get_arg(i);
      int c;
      while ((c = p->get(0)) != EOF)
	if (c != DOUBLE_QUOTE)
	  args += c;
      args += char(END_QUOTE);
      args += '"';
      if (i != limit)
	args += ' ';
      delete p;
    }
    if (limit > 0) {
      args += '\0';
      input_stack::push(make_temp_iterator(args.contents()));
    }
  }
  else if (s[0] == '^' && s[1] == '\0') {
    int limit = input_stack::nargs();
    string args;
    int c = input_stack::peek();
    for (int i = 1; i <= limit; i++) {
      input_iterator *p = input_stack::get_arg(i);
      while ((c = p->get(0)) != EOF) {
	if (c == DOUBLE_QUOTE)
	  c = '"';
	args += c;
      }
      if (input_stack::space_follows_arg(i))
	args += ' ';
      delete p;
    }
    if (limit > 0) {
      args += '\0';
      input_stack::push(make_temp_iterator(args.contents()));
    }
  }
  else {
    const char *p;
    bool is_valid = true;
    bool is_printable = true;
    for (p = s; p != 0 /* nullptr */ && *p != '\0'; p++) {
      if (!csdigit(*p))
	is_valid = false;
      if (!csprint(*p))
	is_printable = false;
    }
    if (!is_valid) {
      static const char msg[] = "invalid positional argument number in"
				" copy mode";
      if (is_printable)
	copy_mode_error("%1 '%2'", msg, s);
      else
	copy_mode_error("%1 (unprintable)", msg);
    }
    else
      input_stack::push(input_stack::get_arg(atoi(s)));
  }
}

void handle_first_page_transition()
{
  push_token(tok);
  topdiv->begin_page();
}

// We push back a token by wrapping it up in a token_node, and
// wrapping that up in a string_iterator.

static void push_token(const token &t)
{
  macro m;
  m.append(new token_node(t));
  input_stack::push(new string_iterator(m));
}

void push_page_ejector()
{
  static char buf[2] = { PAGE_EJECTOR, '\0' };
  input_stack::push(make_temp_iterator(buf));
}

void handle_initial_request(unsigned char code)
{
  char buf[2];
  buf[0] = code;
  buf[1] = '\0';
  macro mac;
  mac.append(new token_node(tok));
  input_stack::push(new string_iterator(mac));
  input_stack::push(make_temp_iterator(buf));
  topdiv->begin_page();
  tok.next();
}

void handle_initial_title()
{
  handle_initial_request(TITLE_REQUEST);
}

static void do_define_macro(define_mode mode, calling_mode calling,
			    comp_mode comp)
{
  symbol nm, term, dot_symbol(".");
  if (calling == CALLING_INDIRECT) {
    symbol temp1 = read_identifier(true /* required */);
    if (temp1.is_null()) {
      skip_line();
      return;
    }
    symbol temp2 = read_identifier();
    input_stack::push(make_temp_iterator("\n"));
    if (!temp2.is_null()) {
      interpolate_string(temp2);
      input_stack::push(make_temp_iterator(" "));
    }
    interpolate_string(temp1);
    input_stack::push(make_temp_iterator(" "));
    tok.next();
  }
  if (mode == DEFINE_NORMAL || mode == DEFINE_APPEND) {
    nm = read_identifier(true /* required */);
    if (nm.is_null()) {
      skip_line();
      return;
    }
  }
  term = read_identifier(); // terminating name
  if (term.is_null())
    term = dot_symbol;
  while (!tok.is_newline() && !tok.is_eof())
    tok.next();
  const char *start_filename;
  int start_lineno;
  bool have_start_location
    = input_stack::get_location(false /* allow_macro */,
				&start_filename,
				&start_lineno);
  node *n;
  // doing this here makes the line numbers come out right
  int c = read_char_in_copy_mode(&n, true /* is_defining */);
  macro mac;
  macro *mm = 0 /* nullptr */;
  if (mode == DEFINE_NORMAL || mode == DEFINE_APPEND) {
    request_or_macro *rm =
      static_cast<request_or_macro *>(request_dictionary.lookup(nm));
    if (rm != 0 /* nullptr */)
      mm = rm->to_macro();
    if (mm != 0 /* nullptr */ && mode == DEFINE_APPEND)
      mac = *mm;
  }
  bool reading_beginning_of_input_line = true;
  if (comp == COMP_DISABLE)
    mac.append(PUSH_GROFF_MODE);
  else if (comp == COMP_ENABLE)
    mac.append(PUSH_COMP_MODE);
  for (;;) {
    if (c == '\n')
      mac.clear_string_flag();
    while (c == ESCAPE_NEWLINE) {
      if (mode == DEFINE_NORMAL || mode == DEFINE_APPEND)
	// TODO: grochar; may need NFD decomposition and UTF-8 encoding
	mac.append(static_cast<unsigned char>(c));
      c = read_char_in_copy_mode(&n, true /* is_defining */);
    }
    if (reading_beginning_of_input_line && (c == '.')) {
      const char *s = term.contents();
      int d = '\0';
      // see if it matches term
      int i = 0;
      if (s[0] != '\0') {
	while (((d = read_char_in_copy_mode(&n)) == ' ') || (d == '\t'))
	  ;
	if (s[0] == d) {
	  for (i = 1; s[i] != '\0'; i++) {
	    d = read_char_in_copy_mode(&n);
	    if (s[i] != d)
	      break;
	  }
	}
      }
      if (s[i] == '\0'
	  && (((i == 2) && want_att_compat)
	      || ((d = read_char_in_copy_mode(&n)) == ' ')
	      || (d == '\n'))) { // we found it
	if (d == '\n')
	  tok.make_newline();
	else
	  tok.make_space();
	if (mode == DEFINE_APPEND || mode == DEFINE_NORMAL) {
	  if (!mm) {
	    mm = new macro;
	    request_dictionary.define(nm, mm);
	  }
	  if (comp == COMP_DISABLE || comp == COMP_ENABLE)
	    mac.append(POP_GROFFCOMP_MODE);
	  *mm = mac;
	}
	if (term != dot_symbol) {
	  want_input_ignored = false;
	  interpolate_macro(term);
	}
	else
	  skip_line();
	return;
      }
      if ((mode == DEFINE_APPEND) || (mode == DEFINE_NORMAL)) {
	// TODO: grochar; may need NFD decomposition and UTF-8 encoding
	mac.append(static_cast<unsigned char>(c));
	for (int j = 0; j < i; j++)
	  // TODO: grochar; may need NFD decomposition & UTF-8 encoding
	  mac.append(static_cast<unsigned char>(s[j]));
      }
      c = d;
    }
    if (c == EOF) {
      if ((mode == DEFINE_APPEND) || (mode == DEFINE_NORMAL)) {
	if (have_start_location)
	  error_with_file_and_line(start_filename, start_lineno,
				   "encountered end of file"
				   " while defining macro '%1'",
				   nm.contents());
	else
	  error("end of file while defining macro '%1'", nm.contents());
      }
      else {
	static const char msg[] = "encountered end of file while"
				  " ignoring input";
	if (have_start_location)
	  error_with_file_and_line(start_filename, start_lineno, msg);
	else
	  error(msg);
      }
      tok.next();
      return;
    }
    if ((mode == DEFINE_NORMAL) || (mode == DEFINE_APPEND)) {
      if (c == '\0')
	mac.append(n);
      else
	// TODO: grochar; may need NFD decomposition and UTF-8 encoding
	mac.append(static_cast<unsigned char>(c));
    }
    reading_beginning_of_input_line = (c == '\n');
    c = read_char_in_copy_mode(&n, true /* is_defining */);
  }
}

static void define_macro()
{
  do_define_macro(DEFINE_NORMAL, CALLING_NORMAL,
		  want_att_compat ? COMP_ENABLE : COMP_IGNORE);
}

static void define_nocomp_macro()
{
  do_define_macro(DEFINE_NORMAL, CALLING_NORMAL, COMP_DISABLE);
}

static void define_indirect_macro()
{
  do_define_macro(DEFINE_NORMAL, CALLING_INDIRECT,
		  want_att_compat ? COMP_ENABLE : COMP_IGNORE);
}

static void define_indirect_nocomp_macro()
{
  do_define_macro(DEFINE_NORMAL, CALLING_INDIRECT, COMP_DISABLE);
}

static void append_macro()
{
  do_define_macro(DEFINE_APPEND, CALLING_NORMAL,
		  want_att_compat ? COMP_ENABLE : COMP_IGNORE);
}

static void append_nocomp_macro()
{
  do_define_macro(DEFINE_APPEND, CALLING_NORMAL, COMP_DISABLE);
}

static void append_indirect_macro()
{
  do_define_macro(DEFINE_APPEND, CALLING_INDIRECT,
		  want_att_compat ? COMP_ENABLE : COMP_IGNORE);
}

static void append_indirect_nocomp_macro()
{
  do_define_macro(DEFINE_APPEND, CALLING_INDIRECT, COMP_DISABLE);
}

void ignore()
{
  want_input_ignored = true;
  do_define_macro(DEFINE_IGNORE, CALLING_NORMAL, COMP_IGNORE);
  want_input_ignored = false;
}

void remove_macro()
{
  if (!has_arg()) {
    warning(WARN_MISSING, "name removal request expects arguments");
    skip_line();
    return;
  }
  for (;;) {
    symbol s = read_identifier();
    if (s.is_null())
      break;
    request_dictionary.remove(s);
  }
  skip_line();
}

void rename_macro()
{
  if (!has_arg()) {
    warning(WARN_MISSING, "renaming request expects arguments");
    skip_line();
    return;
  }
  symbol s1 = read_identifier();
  assert(s1 != 0 /* nullptr */);
  if (!s1.is_null()) {
    symbol s2 = read_identifier();
    if (s2.is_null())
      warning(WARN_MISSING, "renaming request expects identifier of"
	      " existing request, macro, string, or diversion as"
	      " second argument");
    else
      request_dictionary.rename(s1, s2);
  }
  skip_line();
}

void alias_macro()
{
  if (!has_arg()) {
    warning(WARN_MISSING, "name aliasing request expects arguments");
    skip_line();
    return;
  }
  symbol s1 = read_identifier();
  assert(s1 != 0 /* nullptr */);
  if (!s1.is_null()) {
    symbol s2 = read_identifier();
    if (s2.is_null())
      warning(WARN_MISSING, "name aliasing request expects identifier"
	      " of existing request, macro, string, or diversion as"
	      " second argument");
    else {
      if (!request_dictionary.alias(s1, s2))
	error("cannot alias undefined name '%1'", s2.contents());
    }
  }
  skip_line();
}

void chop_macro()
{
  if (!has_arg()) {
    warning(WARN_MISSING, "chop request expects an argument");
    skip_line();
    return;
  }
  symbol s = read_identifier();
  assert(s != 0 /* nullptr */);
  if (!s.is_null()) {
    request_or_macro *p = lookup_request(s);
    macro *m = p->to_macro();
    if (0 /* nullptr */ == m)
      error("cannot chop request '%1'", s.contents());
    else if (m->is_empty())
      error("cannot chop empty %1 '%2'",
	    (m->is_diversion() ? "diversion" : "macro or string"),
	    s.contents());
    else {
      int have_restore = 0;
      // We have to check for additional save/restore pairs which could
      // be there due to empty am1 requests.
      for (;;) {
	if (m->get(m->len - 1) != POP_GROFFCOMP_MODE)
	  break;
	have_restore = 1;
	m->len -= 1;
	if (m->get(m->len - 1) != PUSH_GROFF_MODE
	    && m->get(m->len - 1) != PUSH_COMP_MODE)
	  break;
	have_restore = 0;
	m->len -= 1;
	if (m->len == 0)
	  break;
      }
      if (m->len == 0)
	error("cannot chop empty object '%1'", s.contents());
      else {
	if (have_restore)
	  m->set(POP_GROFFCOMP_MODE, m->len - 1);
	else
	  m->len -= 1;
      }
    }
  }
  skip_line();
}

enum case_xform_mode { STRING_UPCASE, STRING_DOWNCASE };

// Case-transform each byte of the string argument's contents.
void do_string_case_transform(case_xform_mode mode)
{
  assert((mode == STRING_DOWNCASE) || (mode == STRING_UPCASE));
  symbol s = read_identifier();
  assert(s != 0 /* nullptr */);
  if (s.is_null()) {
    skip_line();
    return;
  }
  request_or_macro *p = lookup_request(s);
  macro *m = p->to_macro();
  if (0 /* nullptr */ == m) {
    error("cannot apply string case transformation to request '%1'",
	  s.contents());
    skip_line();
    return;
  }
  string_iterator iter1(*m);
  macro *mac = new macro;
  int len = m->macro::length();
  for (int l = 0; l < len; l++) {
    int nc, c = iter1.get(0);
    if (c == PUSH_GROFF_MODE
	|| c == PUSH_COMP_MODE
	|| c == POP_GROFFCOMP_MODE)
      nc = c;
    else if (c == EOF)
      break;
    else
      if (mode == STRING_DOWNCASE)
	nc = cmlower(c);
      else
	nc = cmupper(c);
    mac->append(nc);
  }
  request_dictionary.define(s, mac);
  tok.next();
}

// Uppercase-transform each byte of the string argument's contents.
void stringdown_request() {
  if (!has_arg()) {
    warning(WARN_MISSING, "string downcasing request expects an"
	    " argument");
    skip_line();
    return;
  }
  do_string_case_transform(STRING_DOWNCASE);
}

// Lowercase-transform each byte of the string argument's contents.
void stringup_request() {
  if (!has_arg()) {
    warning(WARN_MISSING, "string upcasing request expects an"
	    " argument");
    skip_line();
    return;
  }
  do_string_case_transform(STRING_UPCASE);
}

void substring_request()
{
  if (!has_arg()) {
    warning(WARN_MISSING, "substring request expects arguments");
    skip_line();
    return;
  }
  int start;			// 0, 1, ..., n-1  or  -1, -2, ...
  symbol s = read_identifier();
  assert(s != 0 /* nullptr */);
  if (!s.is_null() && read_integer(&start)) {
    request_or_macro *p = lookup_request(s);
    macro *m = p->to_macro();
    if (0 /* nullptr */ == m)
      error("cannot extract substring of request '%1'", s.contents());
    else {
      int end = -1;
      if (!has_arg() || read_integer(&end)) {
	int real_length = 0;			// 1, 2, ..., n
	string_iterator iter1(*m);
	for (int l = 0; l < m->len; l++) {
	  int c = iter1.get(0);
	  if (c == PUSH_GROFF_MODE
	      || c == PUSH_COMP_MODE
	      || c == POP_GROFFCOMP_MODE)
	    continue;
	  if (c == EOF)
	    break;
	  real_length++;
	}
	if (start < 0)
	  start += real_length;
	if (end < 0)
	  end += real_length;
	if (start > end) {
	  int tem = start;
	  start = end;
	  end = tem;
	}
	if (start >= real_length || end < 0) {
	  warning(WARN_RANGE,
		  "start and end index of substring out of range");
	  m->len = 0;
	  if (m->p) {
	    if (--(m->p->count) <= 0)
	      delete m->p;
	    m->p = 0;
	  }
	  skip_line();
	  return;
	}
	if (start < 0) {
	  warning(WARN_RANGE,
		  "start index of substring out of range, set to 0");
	  start = 0;
	}
	if (end >= real_length) {
	  warning(WARN_RANGE,
		  "end index of substring out of range, set to string length");
	  end = real_length - 1;
	}
	// now extract the substring
	string_iterator iter(*m);
	int i;
	for (i = 0; i < start; i++) {
	  int c = iter.get(0 /* nullptr */);
	  while (c == PUSH_GROFF_MODE
		 || c == PUSH_COMP_MODE
		 || c == POP_GROFFCOMP_MODE)
	    c = iter.get(0 /* nullptr */);
	  if (c == EOF)
	    break;
	}
	macro mac;
	for (; i <= end; i++) {
	  node *nd = 0 /* nullptr */;
	  int c = iter.get(&nd);
	  while (c == PUSH_GROFF_MODE
		 || c == PUSH_COMP_MODE
		 || c == POP_GROFFCOMP_MODE)
	    c = iter.get(0 /* nullptr */);
	  if (c == EOF)
	    break;
	  if (c == 0)
	    mac.append(nd);
	  else
	    mac.append((unsigned char) c);
	}
	*m = mac;
      }
    }
  }
  skip_line();
}

void length_request()
{
  if (!has_arg()) {
    warning(WARN_MISSING, "length computation request expects"
	    " arguments");
    skip_line();
    return;
  }
  symbol ret;
  ret = read_identifier();
  if (ret.is_null()) {
    // The identifier was garbage, like `a\&b`.
    skip_line();
    return;
  }
  int c;
  node *n;
  if (tok.is_newline())
    c = '\n';
  else if (tok.is_tab())
    c = '\t';
  else if (!tok.is_space()) {
    skip_line();
    return;
  }
  else
    c = read_char_in_copy_mode(&n);
  while (c == ' ')
    c = read_char_in_copy_mode(&n);
  if (c == '"')
    c = read_char_in_copy_mode(&n);
  int len = 0;
  while (c != '\n' && c != EOF) {
    ++len;
    c = read_char_in_copy_mode(&n);
  }
  reg *r = static_cast<reg *>(register_dictionary.lookup(ret));
  if (r != 0 /* nullptr */)
    r->set_value(len);
  else
    set_register(ret, len);
  tok.next();
}

static void asciify_request()
{
  if (!has_arg()) {
    warning(WARN_MISSING, "diversion asciification request expects a"
	    " diversion identifier as argument");
    skip_line();
    return;
  }
  symbol s = read_identifier();
  if (!s.is_null()) {
    request_or_macro *p = lookup_request(s);
    macro *m = p->to_macro();
    if (0 /* nullptr */ == m)
      error("cannot asciify request '%1'", s.contents());
    else {
      macro am;
      string_iterator iter(*m);
      for (;;) {
	node *nd = 0 /* nullptr */;
	int c = iter.get(&nd);
	if (c == EOF)
	  break;
	if (c != 0)
	  am.append(c);
	else {
	  node *newnd = nd->copy();
	  newnd->asciify(&am);
	  delete nd;
	}
      }
      *m = am;
    }
  }
  skip_line();
}

void unformat_macro()
{
  if (!has_arg()) {
    warning(WARN_MISSING, "diversion unformatting request expects a"
	    " diversion identifier as argument");
    skip_line();
    return;
  }
  symbol s = read_identifier();
  if (!s.is_null()) {
    request_or_macro *p = lookup_request(s);
    macro *m = p->to_macro();
    if (0 /* nullptr */ == m)
      error("cannot unformat request '%1'", s.contents());
    else {
      macro am;
      string_iterator iter(*m);
      for (;;) {
	node *nd = 0 /* nullptr */;
	int c = iter.get(&nd);
	if (c == EOF)
	  break;
	if (c != 0)
	  am.append(c);
	else {
	  if (nd->set_unformat_flag())
	    am.append(nd);
	}
      }
      *m = am;
    }
  }
  skip_line();
}

static void interpolate_environment_variable(symbol nm)
{
  const char *s = getenv(nm.contents());
  if ((s != 0 /* nullptr */) && (*s != 0 /* nullptr */))
    input_stack::push(make_temp_iterator(s));
}

void interpolate_register(symbol nm, int inc)
{
  reg *r = look_up_register(nm);
  assert(r != 0 /* nullptr */);
  if (inc < 0)
    r->decrement();
  else if (inc > 0)
    r->increment();
  input_stack::push(make_temp_iterator(r->get_string()));
}

static void interpolate_number_format(symbol nm)
{
  reg *r = static_cast<reg *>(register_dictionary.lookup(nm));
  if (r != 0 /* nullptr */)
    input_stack::push(make_temp_iterator(r->get_format()));
}

static bool read_delimited_measurement(units *n,
				       unsigned char si,
				       units prev_value)
{
  token start_token;
  start_token.next();
  if (start_token.is_eof()) {
    error("end of input at start of delimited numeric expression");
    return false;
  }
  bool is_valid = false;
  if (!want_att_compat && start_token.is_usable_as_delimiter())
    is_valid = true;
  else if (want_att_compat
           && start_token.is_usable_as_delimiter(false,
		  DELIMITER_ATT_NUMERIC_EXPRESSION))
    is_valid = true;
  if (!is_valid) {
    warning(WARN_DELIM, "cannot use %1 to delimit a numeric expression",
	    start_token.description());
    return false;
  }
  tok.next();
  if (read_measurement(n, si, prev_value)) {
    if (start_token != tok) {
      // token::description() writes to static, class-wide storage, so
      // we must allocate a copy of it before issuing the next
      // diagnostic.
      char *delimdesc = strdup(start_token.description());
      warning(WARN_DELIM, "closing delimiter does not match;"
	      " expected %1, got %2", delimdesc, tok.description());
      free(delimdesc);
    }
    return true;
  }
  return false;
}

// TODO: Merge into other `read_delimited_measurement()`, using default
// argument of 0 for `prev_value`.
static bool read_delimited_measurement(units *n, unsigned char si)
{
  token start_token;
  start_token.next();
  bool is_valid = false;
  if (!want_att_compat && start_token.is_usable_as_delimiter())
    is_valid = true;
  else if (want_att_compat
           && start_token.is_usable_as_delimiter(false,
		  DELIMITER_ATT_NUMERIC_EXPRESSION))
    is_valid = true;
  if (!is_valid) {
    warning(WARN_DELIM, "cannot use %1 to delimit a numeric expression",
	    start_token.description());
    return false;
  }
  tok.next();
  if (read_measurement(n, si)) {
    if (start_token != tok) {
      // token::description() writes to static, class-wide storage, so
      // we must allocate a copy of it before issuing the next
      // diagnostic.
      char *delimdesc = strdup(start_token.description());
      warning(WARN_DELIM, "closing delimiter does not match;"
	      " expected %1, got %2", delimdesc, tok.description());
      free(delimdesc);
    }
    return true;
  }
  return false;
}

// \l, \L
//
// Here's some syntax unique to these escape sequences: a horizontal
// measurment followed immediately by a character.
static bool read_line_rule_expression(units *n, unsigned char si,
				      charinfo **cip)
{
  assert(cip != 0 /* nullptr */);
  token start_token;
  start_token.next();
  if (!want_att_compat
      && !start_token.is_usable_as_delimiter(true /* report error */))
    return false;
  else if (want_att_compat
           && !start_token.is_usable_as_delimiter(true,
		  DELIMITER_ATT_NUMERIC_EXPRESSION)) {
    warning(WARN_DELIM, "line-drawing escape sequence"
	    " does not accept %1 as a delimiter",
	    start_token.description());
    return false;
  }
  int start_level = input_stack::get_level();
  tok.next();
  if (read_measurement(n, si)) {
    if (tok.is_dummy() || tok.is_transparent_dummy())
      tok.next();
    if (!(start_token == tok
	  && input_stack::get_level() == start_level)) {
      *cip = tok.get_charinfo(true /* required */);
      if (0 /* nullptr */ == *cip)
	assert(0 == "attempted to use token without charinfo in"
	       " line-drawing escape sequence");
      tok.next();
    }
    if (!(start_token == tok
	  && input_stack::get_level() == start_level)) {
      // token::description() writes to static, class-wide storage, so
      // we must allocate a copy of it before issuing the next
      // diagnostic.
      char *delimdesc = strdup(start_token.description());
      warning(WARN_DELIM, "closing delimiter does not match; expected"
	      " %1, got %2", delimdesc, tok.description());
      free(delimdesc);
    }
    return true;
  }
  return false;
}

static bool read_size(int *x) // \s
{
  tok.next();
  int c = tok.ch(); // safely compares to char literals; TODO: grochar
  int inc = 0;
  if (c == int('-')) { // TODO: grochar
    inc = -1;
    tok.next();
    c = tok.ch();
  }
  else if (c == int('+')) { // TODO: grochar
    inc = 1;
    tok.next();
    c = tok.ch();
  }
  int val = 0;		// pacify compiler
  bool contains_invalid_digit = false;
  if (c == int('(')) { // TODO: grochar
    tok.next();
    c = tok.ch();
    if (!inc) {
      // allow an increment either before or after the left parenthesis
      if (c == int('-')) { // TODO: grochar
	inc = -1;
	tok.next();
	c = tok.ch();
      }
      else if (c == int('+')) { // TODO: grochar
	inc = 1;
	tok.next();
	c = tok.ch();
      }
    }
    if (!csdigit(c))
      contains_invalid_digit = true;
    else {
      val = c - '0';
      tok.next();
      c = tok.ch();
      if (!csdigit(c))
	contains_invalid_digit = true;
      else {
	val = val * 10 + (c - '0');
	val *= sizescale;
      }
    }
  }
  else if (csdigit(c)) {
    val = c - '0';
    if (want_att_compat && !inc && c != '0' && c < '4') {
      // Support legacy \sNN syntax.
      tok.next();
      c = tok.ch();
      if (!csdigit(c))
	contains_invalid_digit = true;
      else {
	val = val * 10 + (c - '0');
	error("ambiguous type size in escape sequence; rewrite to use"
	      " '%1s(%2' or similar", static_cast<char>(escape_char),
	      val);
      }
    }
    val *= sizescale;
  }
  else if (!want_att_compat && !tok.is_usable_as_delimiter())
    warning(WARN_DELIM, "using %1 as an escape sequence delimiter"
			" is deprecated", tok.description());
  else if (want_att_compat
           && !tok.is_usable_as_delimiter(false,
		  DELIMITER_ATT_STRING_EXPRESSION)) {
    warning(WARN_DELIM, "type size escape sequence"
	    " does not accept %1 as a delimiter",
	    tok.description());
    return false;
  }
  // TODO: groff 1.24.0 release + 2 years?
#if 0
  else if (!tok.is_usable_as_delimiter(true /* report error */))
    return false;
#endif
  else {
    token start(tok);
    tok.next();
    c = tok.ch();
    if ((inc == 0) && ((c == '-') || (c == '+'))) {
      inc = (c == '+') ? 1 : -1;
      tok.next();
    }
    if (!read_measurement(&val, (unsigned char)('z'))) // TODO: grochar
      return false;
    // safely compares to char literals; TODO: grochar
    int s = start.ch();
    int t = tok.ch();
    if (!((s == int('[')) && (t == int(']'))) && (start != tok)) {
      if (s == int('['))
	error("missing ']' in type size escape sequence");
      else {
	// token::description() writes to static, class-wide storage, so
	// we must allocate a copy of it before issuing the next
	// diagnostic.
	char *delimdesc = strdup(start.description());
	if (s != t)
	  error("closing delimiter does not match; expected %1, got %2",
		delimdesc, tok.description());
	free(delimdesc);
      }
      return false;
    }
  }
  if (contains_invalid_digit) {
    if (c != 0U)
      error("expected valid digit in type size escape sequence, got %1",
	    input_char_description(c));
    else
      error("invalid digit in type size escape sequence");
    return false;
  }
  else {
    switch (inc) {
    case 0:
      if (val == 0) {
	// special case -- point size 0 means "revert to previous size"
	*x = 0;
	return true;
      }
      *x = val;
      break;
    case 1:
      *x = curenv->get_requested_point_size() + val;
      break;
    case -1:
      *x = curenv->get_requested_point_size() - val;
      break;
    default:
      assert(0 == "unhandled case of type size increment operator");
    }
    if (*x <= 0) {
      warning(WARN_RANGE,
	      "type size escape sequence results in non-positive size"
	      " %1u; setting it to 1u", *x);
      *x = 1;
    }
    return true;
  }
}

static symbol read_delimited_identifier()
{
  token start_token;
  start_token.next();
  if (start_token.is_eof()) {
    error("end of input at start of delimited name");
    return NULL_SYMBOL;
  }
  bool is_valid = false;
  if (!want_att_compat && start_token.is_usable_as_delimiter())
    is_valid = true;
  else if (want_att_compat
           && start_token.is_usable_as_delimiter(false,
		  DELIMITER_ATT_STRING_EXPRESSION))
    is_valid = true;
  if (!is_valid) {
    warning(WARN_DELIM, "cannot use %1 to delimit an identifier",
	    start_token.description());
    return NULL_SYMBOL;
  }
  int start_level = input_stack::get_level();
  int buf_size = default_buffer_size;
  char *buf = 0 /* nullptr */;
  try {
    // C++03: new char[buf_size]();
    buf = new char[buf_size];
  }
  catch (const std::bad_alloc &e) {
    fatal("cannot allocate %1 bytes to read input line", buf_size);
  }
  (void) memset(buf, 0, (buf_size * sizeof(char)));
  int i = 0;
  for (;;) {
    if ((i + 1) > buf_size) {
      char *old_buf = buf;
      int new_buf_size = buf_size * 2;
      // C++03: new char[new_buf_size]();
      try {
	buf = new char[new_buf_size];
      }
      catch (const std::bad_alloc &e) {
	fatal("cannot allocate %1 bytes to read input line", buf_size);
      }
      (void) memset(buf, 0, (new_buf_size * sizeof(char)));
      (void) memcpy(buf, old_buf, buf_size);
      buf_size = new_buf_size;
      delete[] old_buf;
    }
    tok.next();
    if ((tok == start_token)
	&& (want_att_compat
	    || (input_stack::get_level() == start_level)))
      break;
    if ((buf[i] = tok.ch()) == 0U) {
      // token::description() writes to static, class-wide storage, so
      // we must allocate a copy of it before issuing the next
      // diagnostic.
      char *delimdesc = strdup(start_token.description());
      if (start_token != tok)
	error("closing delimiter does not match; expected %1, got %2",
	      delimdesc, tok.description());
      free(delimdesc);
      delete[] buf;
      return NULL_SYMBOL;
    }
    i++;
  }
  buf[i] = '\0';
  if (0 == i) {
    error("empty delimited name");
    return NULL_SYMBOL;
  }
  symbol s(buf);
  delete[] buf;
  return s;
}

static void do_register() // \R
{
  token start_token;
  start_token.next();
  if (!want_att_compat && !start_token.is_usable_as_delimiter())
    warning(WARN_DELIM, "using %1 as an escape sequence delimiter"
			" is deprecated", tok.description());
  else if (want_att_compat
           && !start_token.is_usable_as_delimiter(false,
		  DELIMITER_ATT_STRING_EXPRESSION)) {
    warning(WARN_DELIM, "register assignment escape sequence"
	    " does not accept %1 as a delimiter",
	    start_token.description());
    return;
  }
  // TODO: groff 1.24.0 release + 2 years?
#if 0
  if (!start_token.is_usable_as_delimiter(true /* report error */)) {
    return;
#endif
  tok.next();
  symbol nm = read_long_identifier(true /* required */);
  if (nm.is_null())
    return;
  tok.skip_spaces();
  reg *r = static_cast<reg *>(register_dictionary.lookup(nm));
  int prev_value;
  if ((0 /* nullptr */ == r) || !r->get_value(&prev_value))
    prev_value = 0;
  int val;
  // TODO: grochar
  if (!read_measurement(&val, (unsigned char)('u'), prev_value))
    return;
  // token::description() writes to static, class-wide storage, so we
  // must allocate a copy of it before issuing the next diagnostic.
  char *delimdesc = strdup(start_token.description());
  if (start_token != tok)
    warning(WARN_DELIM, "closing delimiter does not match; expected %1,"
	    " got %2", delimdesc, tok.description());
  free(delimdesc);
  if (r != 0 /* nullptr */)
    r->set_value(val);
  else
    set_register(nm, val);
}

static void do_width() // \w
{
  token start_token;
  start_token.next();
  if (!want_att_compat && !start_token.is_usable_as_delimiter())
    warning(WARN_DELIM, "using %1 as an escape sequence delimiter"
			" is deprecated", start_token.description());
  else if (want_att_compat
           && !start_token.is_usable_as_delimiter(false,
		  DELIMITER_ATT_STRING_EXPRESSION)) {
    warning(WARN_DELIM, "width computation escape sequence"
	    " does not accept %1 as a delimiter",
	    start_token.description());
    return;
  }
  // TODO: groff 1.24.0 release + 2 years?
#if 0
  if (!start_token.is_usable_as_delimiter(true /* report error */))
    return;
#endif
  int start_level = input_stack::get_level();
  environment env(curenv);
  environment *oldenv = curenv;
  curenv = &env;
  for (;;) {
    tok.next();
    if (tok.is_newline() || tok.is_eof()) {
      // token::description() writes to static, class-wide storage, so
      // we must allocate a copy of it before issuing the next
      // diagnostic.
      char *delimdesc = strdup(start_token.description());
      warning(WARN_DELIM, "missing closing delimiter in width"
	      " computation escape sequence; expected %1, got %2",
	      delimdesc, tok.description());
      free(delimdesc);
      break;
    }
    if (tok == start_token
	&& (want_att_compat || input_stack::get_level() == start_level))
      break;
    tok.process();
  }
  env.wrap_up_tab();
  units x = env.get_input_line_position().to_units();
  input_stack::push(make_temp_iterator(i_to_a(x)));
  env.width_registers();
  curenv = oldenv;
  have_formattable_input = false;
}

charinfo *page_character;

// XXX: The page character is global; shouldn't it be environmental?
// Its idiomatic use is in `tl` requests when formatting titles (headers
// or footers), which full-service macro packages typically put in their
// own environment anyway to ensure that a consistent typeface is used
// there regardless of how body text is styled.
static void page_character_request()
{
  page_character = read_character();
  // TODO?: If null pointer, set to `percent_symbol` (see below),
  // eliminating test in `read_title_parts()` (also below)?
  skip_line();
}

static const symbol percent_symbol("%");

void read_title_parts(node **part, hunits *part_width)
{
  if (!has_arg())
    return;
  token start(tok);
  if (!want_att_compat && !tok.is_usable_as_delimiter())
    warning(WARN_DELIM, "using %1 as a title request delimiter"
			" is deprecated", tok.description());
  int start_level = input_stack::get_level();
  tok.next();
  for (int i = 0; i < 3; i++) {
    while (!tok.is_newline() && !tok.is_eof()) {
      if ((tok == start)
	  && (want_att_compat
	      || input_stack::get_level() == start_level)) {
	tok.next();
	break;
      }
      charinfo *ci = tok.get_charinfo();
      // It's okay for `ci` to be a null pointer; that will be the case
      // if the token is a node: italic corrections, horizontal motions,
      // and so forth.  TODO: Is it worth warning about some node types?
      if ((ci != 0 /* nullptr */)
	  && (page_character != 0 /* nullptr */)
	  && (page_character == ci))
	interpolate_register(percent_symbol, 0);
      else
	tok.process();
      tok.next();
    }
    curenv->wrap_up_tab();
    part_width[i] = curenv->get_input_line_position();
    part[i] = curenv->extract_output_line();
  }
  while (!tok.is_newline() && !tok.is_eof())
    tok.next();
}

// contents of `\?...\?`
class non_interpreted_node : public node {
  macro mac;
public:
  non_interpreted_node(const macro &);
  bool interpret(macro *);
  void asciify(macro *);
  node *copy();
  int ends_sentence();
  bool is_same_as(node *);
  const char *type();
  bool causes_tprint();
  bool is_tag();
};

non_interpreted_node::non_interpreted_node(const macro &m) : mac(m)
{
}

int non_interpreted_node::ends_sentence()
{
  return 2;
}

bool non_interpreted_node::is_same_as(node *nd)
{
  return (mac == static_cast<non_interpreted_node *>(nd)->mac);
}

const char *non_interpreted_node::type()
{
  return "non-interpreted node";
}

bool non_interpreted_node::causes_tprint()
{
  return false;
}

bool non_interpreted_node::is_tag()
{
  return false;
}

void non_interpreted_node::asciify(macro *)
{
  delete this;
}

node *non_interpreted_node::copy()
{
  return new non_interpreted_node(mac);
}

bool non_interpreted_node::interpret(macro *m)
{
  string_iterator si(mac);
  node *n = 0 /* nullptr */;
  for (;;) {
    int c = si.get(&n);
    if (c == EOF)
      break;
    if (c == 0)
      m->append(n);
    else
      m->append(c);
  }
  return true;
}

static node *do_non_interpreted() // \?
{
  node *n;
  int c;
  macro mac;
  while (((c = read_char_in_copy_mode(&n)) != ESCAPE_QUESTION)
	 && (c != EOF)
	 && (c != '\n'))
    if (c == 0)
      mac.append(n);
    else
      mac.append(c);
  if (c == EOF || c == '\n') {
    error("unterminated transparent embedding escape sequence");
    return 0 /* nullptr */;
  }
  return new non_interpreted_node(mac);
}

static void map_special_character_for_device_output(macro *mac,
						    const char *sc)
{
  if (strcmp("-", sc) == 0)
    mac->append('-');
  else if (strcmp("dq", sc) == 0)
    mac->append('"');
  else if (strcmp("sh", sc) == 0)
    mac->append('#');
  else if (strcmp("Do", sc) == 0)
    mac->append('$');
  else if (strcmp("aq", sc) == 0)
    mac->append('\'');
  else if (strcmp("sl", sc) == 0)
    mac->append('/');
  else if (strcmp("at", sc) == 0)
    mac->append('@');
  else if (strcmp("lB", sc) == 0)
    mac->append('[');
  else if (strcmp("rs", sc) == 0)
    mac->append('\\');
  else if (strcmp("rB", sc) == 0)
    mac->append(']');
  else if (strcmp("ha", sc) == 0)
    mac->append('^');
  else if (strcmp("lC", sc) == 0)
    mac->append('{');
  else if (strcmp("ba", sc) == 0)
    mac->append('|');
  else if (strcmp("or", sc) == 0)
    mac->append('|');
  else if (strcmp("rC", sc) == 0)
    mac->append('}');
  else if (strcmp("ti", sc) == 0)
    mac->append('~');
  else {
    if (font::use_charnames_in_special) {
      if (sc[0] != '\0') {
	mac->append('\\');
	mac->append('[');
	int i = 0;
	while (sc[i] != '\0') {
	  mac->append(sc[i]);
	  i++;
	}
	mac->append(']');
      }
    }
    else {
      char errbuf[ERRBUFSZ]; // C++03: char errbuf[ERRBUFSZ]()
      (void) memset(errbuf, '\0', ERRBUFSZ);
      const size_t unibufsz = UNIBUFSZ + 1 /* '\0' */;
      char character[unibufsz]; // C++03: char errbuf[ERRBUFSZ]()
      (void) memset(character, '\0', UNIBUFSZ);
      // If it looks like something other than an attempt at a Unicode
      // special character escape sequence already, try to convert it
      // into one.  Output drivers don't (and shouldn't) know anything
      // about a troff formatter's special character identifiers.
      if ((strlen(sc) < 3) || (sc[0] != 'u')) {
	const char *un = glyph_name_to_unicode(sc);
	if (un != 0 /* nullptr */)
	  strncpy(character, un, unibufsz);
	else {
	  warning(WARN_CHAR, "special character '%1' is not encodable"
	       " in device-independent output", sc);
	  return;
	}
      }
      else {
	const char *un = valid_unicode_code_sequence(sc, errbuf);
	if (0 /* nullptr */ == un) {
	  warning(WARN_CHAR, "special character '%1' is not encodable"
	       " in device-independent output: %2", sc, errbuf);
	  return;
	}
	strncpy(character, un, unibufsz);
      }
      mac->append_str("\\[u");
      mac->append_str(character);
      mac->append(']');
    }
  }
}

static void encode_special_character_for_device_output(macro *mac)
{
  const char *sc;
  charinfo *ci = tok.get_charinfo(true /* required */);
  if (0 /* nullptr */ == ci) {
    assert(0 == "attempted to encode token without charinfo for"
	   " device extension command output");
    return;
  }
  sc = ci->get_symbol()->contents();
  if (0 /* nullptr */ == sc) {
    assert(0 == "attempted to encode token containing charinfo with"
	   " null symbol for device extension command output");
    return;
  }
  map_special_character_for_device_output(mac, sc);
}

// In troff output, we translate the escape character to '\', but it is
// up to the postprocessor to interpret it as such.  (This mostly
// matters for device extension commands.)
static void encode_character_for_device_output(macro *mac, const char c)
{
  if ('\0' == c) {
    // It's a special token, not a character we can write as-is.
    if (tok.is_stretchable_space()
	     || tok.is_unstretchable_space())
      mac->append(' ');
    else if ((tok.is_hyphen_indicator())
	     || tok.is_zero_width_break()
	     || tok.is_dummy()
	     || tok.is_transparent_dummy())
      /* do nothing */;
    else if (tok.is_special_character())
      encode_special_character_for_device_output(mac);
    else
      warning(WARN_CHAR, "%1 is not encodable in device-independent"
	      " output ('asciify' might help)", tok.description());
  }
  else {
    if (c == escape_char)
      mac->append('\\');
    else
      mac->append(c);
  }
}

static node *do_device_extension() // \X
{
  token start_token;
  start_token.next();
  if (!want_att_compat && !start_token.is_usable_as_delimiter())
    warning(WARN_DELIM, "using %1 as an escape sequence delimiter"
			" is deprecated", tok.description());
  else if (want_att_compat
           && !start_token.is_usable_as_delimiter(false,
		  DELIMITER_ATT_STRING_EXPRESSION)) {
    warning(WARN_DELIM, "device extension command escape sequence"
	    " does not accept %1 as a delimiter",
	    start_token.description());
    return 0 /* nullptr */;
  }
  // TODO: groff 1.24.0 release + 2 years?
#if 0
  if (!start_token.is_usable_as_delimiter(true /* report error */))
    return 0 /* nullptr */;
#endif
  int start_level = input_stack::get_level();
  macro mac;
  if ((curdiv == topdiv) && (topdiv->before_first_page_status > 0))
    topdiv->begin_page();
  for (;;) {
    tok.next();
    if (tok.is_newline() || tok.is_eof()) {
      // token::description() writes to static, class-wide storage, so
      // we must allocate a copy of it before issuing the next
      // diagnostic.
      char *delimdesc = strdup(start_token.description());
      warning(WARN_DELIM, "missing closing delimiter in device"
	      " extension escape sequence; expected %1, got %2",
	      delimdesc, tok.description());
      free(delimdesc);
      break;
    }
    if (tok == start_token
	&& (want_att_compat || input_stack::get_level() == start_level))
      break;
    unsigned char c; // TODO: grochar
    if (tok.is_space())
      c = ' ';
    // TODO: Stop silently ignoring these when we have a string
    // iterator for users and can externalize "sanitization" operations.
    // See <https://savannah.gnu.org/bugs/?62264>.
    else if (tok.is_hyphen_indicator())
      continue;
    else if (tok.is_dummy())
      continue;
    else if (tok.is_zero_width_break())
      continue;
    else
      c = tok.ch();
    encode_character_for_device_output(&mac, c);
  }
  return new device_extension_node(mac);
}

static void device_request()
{
  if (!has_arg(true /* peek; we want to read in copy mode */)) {
    warning(WARN_MISSING, "device extension request expects an"
	    " argument");
    skip_line();
    return;
  }
  macro mac;
  int c;
  for (;;) {
    c = read_char_in_copy_mode(0 /* nullptr */);
    if ('"' == c) {
      c = read_char_in_copy_mode(0 /* nullptr */);
      break;
    }
    if (c != ' ' && c != '\t')
      break;
  }
  if ((curdiv == topdiv) && (topdiv->before_first_page_status > 0))
    topdiv->begin_page();
  for (;
      (c != '\0') && (c != '\n') && (c != EOF);
       c = read_char_in_copy_mode(0 /* nullptr */)) {
    // We may encounter some of the C0 and C1 character codes GNU troff
    // uses for special purposes; see src/roff/troff/input.h.  They
    // produce nothing in grout.  Warn only about the ones that are left
    // for the user's purposes.  Use octal because input.h does.  Ignore
    // 8-bit codes in general.  grout is an ISO 646 file format.
    if (ESCAPE_TILDE == c) {
      mac.append('\\');
      mac.append('~');
    }
    else if ((c < 015) || (c >= 0177))
      warning (WARN_SYNTAX, "ignoring character code %1 in device"
	       " extension command request argument", c);
    else if (c != '\\')
      mac.append(c);
    else {
      int c1 = read_char_in_copy_mode(0 /* nullptr */);
      if (c1 != '[') {
	mac.append(c);
	mac.append(c1);
	string chardesc = "";
	if (csprint(c1)) {
	  chardesc += "'";
	  chardesc += char(c1);
	  chardesc += "'";
	}
	else {
	  chardesc += "character code ";
	  chardesc += i_to_a(c1);
	}
	chardesc += '\0'; // make it safe for .contents()
	warning (WARN_SYNTAX, "not interpreting escaped %1 in device"
		   " extension command request argument",
		   chardesc.contents());
      }
      else {
	// Does the input resemble a valid (bracket-form) special
	// character escape sequence?
	bool is_valid = false;
	string sc = "";
	int c2 = read_char_in_copy_mode(0 /* nullptr */);
	for (; (c2 != '\0') && (c2 != '\n') && (c2 != EOF);
	     c2 = read_char_in_copy_mode(0 /* nullptr */)) {
	  // XXX: `map_special_character_for_device_output()` will need
	  // the closing bracket in the iterator we construct, but a
	  // composite character mapping mustn't see it.
	  sc += c2;
	  if (']' == c2) {
	    is_valid = true;
	    break;
	  }
	}
	sc += '\0';
	if (sc.search(' ') > 0) {
	  // XXX: TODO
	  error("composite special character escape sequences not yet"
	        " supported in device extension command arguments");
	  is_valid = false;
	}
	if (is_valid) {
	  input_stack::push(make_temp_iterator(sc.contents()));
	  symbol s = read_long_escape_parameters(WITH_ARGS);
	  map_special_character_for_device_output(&mac, s.contents());
	}
	else {
	  // We couldn't make sense of it.  Write it out as-is.
	  mac.append(c);
	  mac.append(c1);
	  mac.append_str(sc.contents());
	}
      }
    }
  }
  curenv->add_node(new device_extension_node(mac));
  tok.next();
}

static void device_macro_request()
{
  symbol s = read_identifier(true /* required */);
  if (!(s.is_null() || s.is_empty())) {
    request_or_macro *p = lookup_request(s);
    macro *m = p->to_macro();
    if (m != 0 /* nullptr */)
      curenv->add_node(new device_extension_node(*m));
    else
      error("cannot interpolate '%1' to device-independent output;"
	    " it is a request, not a macro", s.contents());
  }
  skip_line();
}

static void output_request()
{
  if (!has_arg(true /* peek; we want to read in copy mode */)) {
    warning(WARN_MISSING, "output request expects arguments");
    skip_line();
    return;
  }
  int c;
  for (;;) {
    c = read_char_in_copy_mode(0 /* nullptr */);
    if ('"' == c) {
      c = read_char_in_copy_mode(0 /* nullptr */);
      break;
    }
    if (c != ' ' && c != '\t')
      break;
  }
  for (;
       (c != '\n') && (c != EOF);
       (c = read_char_in_copy_mode(0 /* nullptr */)))
    topdiv->transparent_output(c);
  topdiv->transparent_output('\n');
  tok.next();
}

extern int image_no;		// from node.cpp

static node *do_suppress(symbol nm) // \O
{
  if (nm.is_null() || nm.is_empty()) {
    error("output suppression escape sequence requires an argument");
    return 0 /* nullptr */;
  }
  const char *s = nm.contents();
  switch (*s) {
  case '0':
    if (0 == suppression_level)
      // suppress generation of glyphs
      return new suppress_node(0, 0);
    break;
  case '1':
    if (0 == suppression_level)
      // enable generation of glyphs
      return new suppress_node(1, 0);
    break;
  case '2':
    if (0 == suppression_level)
      return new suppress_node(1, 1);
    break;
  case '3':
    have_formattable_input = true;
    suppression_level++;
    break;
  case '4':
    have_formattable_input = true;
    suppression_level--;
    break;
  case '5':
    {
      s++;			// move over '5'
      char position = *s;
      if ('\0' == *s) {
	error("missing position and file name in output suppression"
	      " escape sequence");
	return 0 /* nullptr */;
      }
      if ((position != 'l')
	  && (position != 'r')
	  && (position != 'c')
	  && (position != 'i')) {
	error("expected position 'l', 'r', 'c', or 'i' in output"
	      " suppression escape sequence, got '%1'", position);
	return 0 /* nullptr */;
      }
      s++;			// onto image name
      if (0 == s /* nullptr */) {
	error("missing image name in output suppression escape"
	      " sequence");
	return 0 /* nullptr */;
      }
      image_no++;
      if (0 == suppression_level)
	return new suppress_node(symbol(s), position, image_no);
      else
	have_formattable_input = true;
    }
    break;
  default:
    char qc = '\'';
    if (strchr(s, '\'') != 0 /* nullptr */)
      qc = '"';
    error("invalid argument %1%2%3 to output suppression escape"
	  " sequence", qc, *s, qc);
  }
  return 0 /* nullptr */;
}

void device_extension_node::tprint(troff_output_file *out)
{
  tprint_start(out);
  string_iterator iter(mac);
  for (;;) {
    int c = iter.get(0 /* nullptr */);
    if (c != EOF)
      for (const char *s = encode_for_stream_output(c);
	   *s != 0 /* nullptr */;
	   s++)
	tprint_char(out, *s);
    else
      break;
  }
  tprint_end(out);
}

int get_file_line(const char **filename, int *lineno)
{
  return input_stack::get_location(false /* allow macro */, filename,
				   lineno);
}

void line_file()
{
  int n;
  if (read_integer(&n)) {
    if (has_arg(true /* peek */)) {
      const char *reported_file_name = read_rest_of_line_as_argument();
      (void) input_stack::set_location(reported_file_name, (n - 1));
      // TODO: Add `reported_file_name` to file name set.
      tok.next();
      return;
    }
    (void) input_stack::set_location(0 /* nullptr */, (n - 1));
  }
  skip_line();
}

static void nroff_request()
{
  in_nroff_mode = true;
  skip_line();
}

static void troff_request()
{
  in_nroff_mode = false;
  skip_line();
}

static void skip_branch()
{
  if (tok.is_newline()) {
    tok.next();
    return;
  }
  int level = 0;
  // ensure that ".if 0\{" works as expected
  if (tok.is_left_brace())
    level++;
  int c;
  for (;;) {
    c = input_stack::get(0 /* nullptr */);
    if (c == EOF)
      break;
    if (c == ESCAPE_LEFT_BRACE)
      ++level;
    else if (c == ESCAPE_RIGHT_BRACE)
      --level;
    else if ((c == escape_char) && (escape_char != 0U))
      switch (input_stack::get(0 /* nullptr */)) {
      case '{':
	++level;
	break;
      case '}':
	--level;
	break;
      case '"':
	while ((c = input_stack::get(0 /* nullptr */)) != '\n'
		&& c != EOF)
	  ;
      }
    /*
      Note that the level can properly be < 0, e.g.

	.if 1 \{\
	.if 0 \{\
	.\}\}

      So don't give an error message in this case.
    */
    if (level <= 0 && c == '\n')
      break;
  }
  tok.next();
}

static void take_branch()
{
  while (tok.is_space() || tok.is_left_brace())
    tok.next();
}

static void nop_request()
{
  tok.skip_spaces();
}

// Perform a (formatted) output comparison operation, as found in
//   .if 'foo'bar'
// ...for example.
static bool are_comparands_equal()
{
  token delim = tok;
  int delim_level = input_stack::get_level();
  environment env1(curenv);
  environment env2(curenv);
  environment *oldenv = curenv;
  curenv = &env1;
  suppress_push = true;
  for (int i = 0; i < 2; i++) {
    for (;;) {
      tok.next();
      if (tok.is_newline() || tok.is_eof()) {
	// token::description() writes to static, class-wide storage,
	// so we must allocate a copy of it before issuing the next
	// diagnostic.
	char *delimdesc = strdup(delim.description());
	warning(WARN_DELIM, "missing closing delimiter in output"
		" comparison operator; expected %1, got %2",
		delimdesc, tok.description());
	free(delimdesc);
	tok.next();
	curenv = oldenv;
	return false;
      }
      if ((tok == delim)
	  && (want_att_compat
	      || (input_stack::get_level() == delim_level)))
        break;
      tok.process();
    }
    curenv = &env2;
  }
  node *n1 = env1.extract_output_line();
  node *n2 = env2.extract_output_line();
  bool result = same_node_list(n1, n2);
  delete_node_list(n1);
  delete_node_list(n2);
  curenv = oldenv;
  have_formattable_input = false;
  suppress_push = false;
  tok.next();
  return result;
}

static std::stack<bool> if_else_stack;

static bool is_conditional_expression_true()
{
  bool perform_output_comparison = false;
  bool want_test_sense_inverted = false;
  tok.skip_spaces();
  while (tok.ch() == int('!')) { // TODO: grochar
    tok.next();
    want_test_sense_inverted = !want_test_sense_inverted;
  }
  bool result;
  int c = tok.ch(); // safely compares to char literals; TODO: grochar
  if (want_att_compat)
    switch (c) {
    case int('F'): // TODO: grochar
    case int('S'): // TODO: grochar
    case int('c'): // TODO: grochar
    case int('d'): // TODO: grochar
    case int('m'): // TODO: grochar
    case int('r'): // TODO: grochar
    case int('v'): // TODO: grochar
      warning(WARN_SYNTAX,
	      "conditional expression operator '%1' is not portable to"
	      " AT&T troff",
	      char(c));
	      // TODO: "; treating as output comparison delimiter", c);
      break;
    default:
      break;
    }
  if (c == int('t')) { // TODO: grochar
    tok.next();
    result = !in_nroff_mode;
  }
  else if (c == int('n')) { // TODO: grochar
    tok.next();
    result = in_nroff_mode;
  }
  else if (c == int('o')) { // TODO: grochar
    result = (topdiv->get_page_number() & 1); // TODO: dump cleverness
    tok.next();
  }
  else if (c == int('e')) { // TODO: grochar
    result = !(topdiv->get_page_number() & 1); // TODO: dump cleverness
    tok.next();
  }
  // TODO: else if (!want_att_compat) {
  // Check for GNU troff extended conditional expression operators.
  else if ((c == int('d') || (c == int('r')))) { // TODO: grochar
    tok.next();
    symbol nm = read_identifier(true /* required */);
    if (nm.is_null()) {
      skip_branch();
      return false;
    }
    result = ((c == 'd')
	      ? request_dictionary.lookup(nm) != 0 /* nullptr */
	      : register_dictionary.lookup(nm) != 0 /* nullptr */);
  }
  else if (c == 'm') {
    tok.next();
    symbol nm = read_long_identifier(true /* required */);
    if (nm.is_null()) {
      skip_branch();
      return false;
    }
    result = ((nm == default_symbol)
	      || color_dictionary.lookup(nm) != 0 /* nullptr */);
  }
  else if (c == 'c') {
    tok.next();
    tok.skip_spaces();
    // XXX: Mystery: the presence of a character (fortunately) doesn't
    // create it if nonexistent even though the default second argument
    // to `token::get_charinfo()` (`suppress_creation`) is `false` (see
    // "token.h").  Why?
    charinfo *ci = tok.get_charinfo(true /* required */);
    if (0 == ci /* nullptr */) {
      skip_branch();
      return false;
    }
    result = character_exists(ci, curenv);
    tok.next();
  }
  else if (c == 'F') {
    tok.next();
    symbol nm = read_long_identifier(true /* required */);
    if (nm.is_null()) {
      skip_branch();
      return false;
    }
    result = is_font_name(curenv->get_family()->nm, nm);
  }
  else if (c == 'S') {
    tok.next();
    symbol nm = read_long_identifier(true /* required */);
    if (nm.is_null()) {
      skip_branch();
      return false;
    }
    result = is_abstract_style(nm);
  }
  // vtroff extension
  else if (c == 'v') {
    tok.next();
    result = false;
  }
  else if (tok.is_space())
    result = false;
  else if (!want_att_compat
	   && tok.is_usable_as_delimiter())
    perform_output_comparison = true;
  else if (want_att_compat
	   && tok.is_usable_as_delimiter(false /* report error */,
		  DELIMITER_ATT_OUTPUT_COMPARISON_EXPRESSION))
    perform_output_comparison = true;
  else {
    // Evaluate numeric expression.
    units n;
    if (!read_measurement(&n, (unsigned char)('u'))) { // TODO: grochar
      skip_branch();
      return false;
    }
    else
      result = (n > 0);
  }
  if (perform_output_comparison)
    result = are_comparands_equal();
  if (want_test_sense_inverted)
    result = !result;
  if (result)
    take_branch();
  else
    skip_branch();
  return result;
}

static void if_else_request()
{
  if (!has_arg()) {
    warning(WARN_MISSING, "if-else request expects arguments");
    skip_line();
    return;
  }
  if_else_stack.push(is_conditional_expression_true());
}

static void if_request()
{
  if (!has_arg()) {
    warning(WARN_MISSING, "if-then request expects arguments");
    skip_line();
    return;
  }
  (void) is_conditional_expression_true();
}

static void else_request()
{
  if (if_else_stack.empty())
    skip_branch();
  else {
    bool predicate = if_else_stack.top();
    if_else_stack.pop();
    if (predicate)
      skip_branch();
    else
      take_branch();
  }
}

static int while_depth = 0;
static bool want_loop_break = false;

static void while_request()
{
  if (!has_arg(true /* peek */)) {
    warning(WARN_MISSING, "while loop request expects arguments");
    skip_line();
    return;
  }
  macro mac;
  bool is_char_escaped = false;
  int level = 0;
  mac.append(new token_node(tok));
  for (;;) {
    node *n = 0 /* nullptr */;
    int c = input_stack::get(&n);
    if (c == EOF)
      break;
    if (c == 0) {
      is_char_escaped = false;
      mac.append(n);
    }
    else if (is_char_escaped) {
      if (c == '{')
	level += 1;
      else if (c == '}')
	level -= 1;
      is_char_escaped = false;
      mac.append(c);
    }
    else {
      if (c == ESCAPE_LEFT_BRACE)
	level += 1;
      else if (c == ESCAPE_RIGHT_BRACE)
	level -= 1;
      else if (c == escape_char)
	is_char_escaped = true;
      mac.append(c);
      if (c == '\n' && level <= 0)
	break;
    }
  }
  if (level != 0)
    error("unbalanced brace escape sequences");
  else {
    while_depth++;
    input_stack::add_boundary();
    for (;;) {
      input_stack::push(new string_iterator(mac, "while loop"));
      tok.next();
      if (!is_conditional_expression_true()) {
	while (input_stack::get(0 /* nullptr */) != EOF)
	  ;
	break;
      }
      process_input_stack();
      if (want_loop_break || input_stack::is_return_boundary()) {
	want_loop_break = false;
	break;
      }
    }
    input_stack::remove_boundary();
    while_depth--;
  }
  tok.next();
}

static void while_break_request()
{
  if (!while_depth) {
    error("cannot 'break' when not in a 'while' loop");
    skip_line();
  }
  else {
    want_loop_break = true;
    while (input_stack::get(0 /* nullptr */) != EOF)
      ;
    tok.next();
  }
}

static void while_continue_request()
{
  if (!while_depth) {
    error("cannot 'continue' when not in a 'while' loop");
    skip_line();
  }
  else {
    while (input_stack::get(0 /* nullptr */) != EOF)
      ;
    tok.next();
  }
}

void do_source(bool quietly)
{
  char *filename = read_rest_of_line_as_argument();
  errno = 0;
  FILE *fp = include_search_path.open_file_cautiously(filename);
  if (fp != 0 /* nullptr */)
    input_stack::push(new file_iterator(fp, filename));
  else
    // Suppress diagnostic only if we're operating quietly and it's an
    // expected problem.
    if (!(quietly && (ENOENT == errno)))
      error("cannot open '%1': %2", filename, strerror(errno));
  // TODO: Add `filename` to file name set.
  tok.next();
}

void source_request() // .so
{
  if (!has_arg(true /* peek */)) {
    warning(WARN_MISSING, "file sourcing request expects an argument");
    skip_line();
    return;
  }
  do_source(false /* quietly */ );
}

// like .so, but silently ignore files that can't be opened due to their
// nonexistence
void source_quietly_request() // .soquiet
{
  if (!has_arg(true /* peek */)) {
    warning(WARN_MISSING, "quiet file sourcing request expects an"
	    " argument");
    skip_line();
    return;
  }
  do_source(true /* quietly */ );
}

void pipe_source_request() // .pso
{
  if (!has_arg(true /* peek */)) {
    warning(WARN_MISSING, "piped command source request expects"
	    " arguments");
    skip_line();
    return;
  }
  if (!want_unsafe_requests) {
    error("piped command source request is not allowed in safer mode");
    skip_line();
    return;
  }
  char *pcmd = read_rest_of_line_as_argument();
  // `has_arg()` should have ensured that this pointer is non-null.
  assert(pcmd != 0 /* nullptr */);
  if (0 /* nullptr */ == pcmd)
    error("cannot apply piped command source request to empty"
	  " argument");
  errno = 0;
  FILE *fp = popen(pcmd, POPEN_RT);
  if (fp != 0 /* nullptr */)
    input_stack::push(new file_iterator(fp, pcmd, true /* popened */));
  else
    error("cannot open pipe to process '%1': %2", pcmd,
	  strerror(errno));
  delete[] pcmd;
  tok.next();
}

// .psbb
//
// Extract bounding box limits from PostScript file, and assign
// them to the following four gtroff registers:--
//
static int llx_reg_contents = 0;
static int lly_reg_contents = 0;
static int urx_reg_contents = 0;
static int ury_reg_contents = 0;

// Manifest constants to specify the status of bounding box range
// acquisition; (note that PSBB_RANGE_IS_BAD is also suitable for
// assignment as a default ordinate property value).
//
#define PSBB_RANGE_IS_BAD   0
#define PSBB_RANGE_IS_SET   1
#define PSBB_RANGE_AT_END   2

// Maximum input line length, for DSC conformance, and options to
// control how it will be enforced; caller should select either of
// DSC_LINE_MAX_IGNORED, to allow partial line collection spread
// across multiple calls, or DSC_LINE_MAX_ENFORCE, to truncate
// excess length lines at the DSC limit.
//
// Note that DSC_LINE_MAX_CHECKED is reserved for internal use by
// ps_locator::get_line(), and should not be specified in any call;
// also, handling of DSC_LINE_MAX_IGNORED, as a get_line() option,
// is currently unimplemented.
//
#define DSC_LINE_MAX          255
#define DSC_LINE_MAX_IGNORED   -1
#define DSC_LINE_MAX_ENFORCE    0
#define DSC_LINE_MAX_CHECKED    1

// Input characters to be considered as whitespace, when reading
// PostScript file comments.
//
cset white_space("\n\r \t");

// Class psbb_locator
//
// This locally declared and implemented class provides the methods
// to be used for retrieval of bounding box properties from a specified
// PostScript or PDF file.
//
class psbb_locator
{
  public:
    // Only the class constructor is exposed publicly; instantiation of
    // a class object will retrieve the requisite bounding box properties
    // from the specified file, and assign them to gtroff registers.
    //
    psbb_locator(const char *);

  private:
    FILE *fp;
    const char *filename;
    char buf[2 + DSC_LINE_MAX];
    int llx, lly, urx, ury;

    // CRLF handling hook, for get_line() function.
    //
    int lastc;

    // Private method functions facilitate implementation of the
    // class constructor; none are used in any other context.
    //
    int get_line(int);
    inline bool get_header_comment(void);
    inline const char *context_args(const char *);
    inline const char *context_args(const char *, const char *);
    inline const char *bounding_box_args(void);
    int parse_bounding_box(const char *);
    inline void assign_registers(void);
    inline int skip_to_trailer(void);
};

// psbb_locator class constructor.
//
psbb_locator::psbb_locator(const char *fname):
filename(fname), llx(0), lly(0), urx(0), ury(0), lastc(EOF)
{
  // PS files might contain non-printable characters, such as ^Z
  // and CRs not followed by an LF, so open them in binary mode.
  //
  fp = include_search_path.open_file_cautiously(filename, 0, FOPEN_RB);
  if (fp != 0 /* nullptr */) {
    // After successfully opening the file, acquire the first
    // line, whence we may determine the file format...
    //
    if (get_line(DSC_LINE_MAX_ENFORCE) == 0)
      //
      // ...except in the case of an empty file, which we are
      // unable to process further.
      //
      error("file '%1' is empty", filename);

# if 0
    else if (context_args("%PDF-")) {
      // TODO: PDF files specify a /MediaBox, as the equivalent
      // of %%BoundingBox; we must implement a handler for this.
    }
# endif

    else if (context_args("%!PS-Adobe-")) {
      //
      // PostScript files -- strictly, we expect EPS -- should
      // specify a %%BoundingBox comment; locate it, initially
      // expecting to find it in the comments header...
      //
      const char *context = 0 /* nullptr */;
      while ((context == 0 /* nullptr */) && get_header_comment()) {
	if ((context = bounding_box_args()) != 0 /* nullptr */) {

	  // When the "%%BoundingBox" comment is found, it may simply
	  // specify the bounding box property values, or it may defer
	  // assignment to a similar trailer comment...
	  //
	  int status = parse_bounding_box(context);
	  if (status == PSBB_RANGE_AT_END) {
	    //
	    // ...in which case we must locate the trailer, and search
	    // for the appropriate specification within it.
	    //
	    if (skip_to_trailer() > 0) {
	      while ((context = bounding_box_args()) == 0 /* nullptr */
		     && get_line(DSC_LINE_MAX_ENFORCE) > 0)
		;
	      if (context != 0 /* nullptr */) {
		//
		// When we find a bounding box specification here...
		//
		if ((status = parse_bounding_box(context)) == PSBB_RANGE_AT_END)
		  //
		  // ...we must ensure it is not a further attempt to defer
		  // assignment to a trailer, (which we are already parsing).
		  //
		  error("'(atend)' is not allowed in trailer of '%1'",
			filename);
	      }
	    }
	    else
	      // The trailer could not be found, so there is no context in
	      // which a trailing %%BoundingBox comment might be located.
	      //
	      context = 0 /* nullptr */;
	  }
	  if (status == PSBB_RANGE_IS_BAD) {
	    //
	    // This arises when we found a %%BoundingBox comment, but
	    // we were unable to extract a valid set of range values from
	    // it; all we can do is diagnose this.
	    //
	    error("the arguments to the %%%%BoundingBox comment in '%1' are bad",
		  filename);
	  }
	}
      }
      if (context == 0 /* nullptr */)
	//
	// Conversely, this arises when no value specifying %%BoundingBox
	// comment has been found, in any appropriate location...
	//
	error("%%%%BoundingBox comment not found in '%1'", filename);
    }
    else
      // ...while this indicates that there was no appropriate file format
      // identifier, on the first line of the input file.
      //
      error("'%1' does not conform to the Document Structuring Conventions",
	    filename);

    // Regardless of success or failure of bounding box property acquisition,
    // we did successfully open an input file, so we must now close it...
    //
    fclose(fp);
  }
  else
    // ...but in this case, we did not successfully open any input file.
    //
    error("cannot open '%1': %2", filename, strerror(errno));

  // Irrespective of whether or not we were able to successfully acquire the
  // bounding box properties, we ALWAYS update the associated gtroff registers.
  //
  assign_registers();
}

// psbb_locator::parse_bounding_box()
//
// Parse the argument to a %%BoundingBox comment, returning:
//   PSBB_RANGE_IS_SET if it contains four numbers,
//   PSBB_RANGE_AT_END if it contains "(atend)", or
//   PSBB_RANGE_IS_BAD otherwise.
//
int psbb_locator::parse_bounding_box(const char *context)
{
  // The Document Structuring Conventions say that the numbers
  // should be integers.
  //
  int status = PSBB_RANGE_IS_SET;
  if (sscanf(context, "%d %d %d %d", &llx, &lly, &urx, &ury) != 4) {
    //
    // Unfortunately some broken applications get this wrong;
    // try to parse them as doubles instead...
    //
    double x1, x2, x3, x4;
    if (sscanf(context, "%lf %lf %lf %lf", &x1, &x2, &x3, &x4) == 4) {
      llx = (int) x1;
      lly = (int) x2;
      urx = (int) x3;
      ury = (int) x4;
    }
    else {
      // ...but if we can't parse four numbers, skip over any
      // initial whitespace...
      //
      while (*context == '\x20' || *context == '\t')
	context++;

      // ...before checking for "(atend)", and setting the
      // appropriate exit status accordingly.
      //
      status = (context_args("(atend)", context) == 0 /* nullptr */)
		 ? llx = lly = urx = ury = PSBB_RANGE_IS_BAD
		 : PSBB_RANGE_AT_END;
    }
  }
  return status;
}

// ps_locator::get_line()
//
// Collect an input record from a PostScript or PDF file.
//
// Inputs:
//   buf       pointer to caller's input buffer.
//   fp        FILE stream pointer, whence input is read.
//   filename  name of input file, (for diagnostic use only).
//   dscopt    DSC_LINE_MAX_ENFORCE or DSC_LINE_MAX_IGNORED.
//
// Returns the number of input characters stored into caller's
// buffer, or zero at end of input stream.
//
// FIXME: Currently, get_line() always scans an entire line of
// input, but returns only as much as will fit in caller's buffer;
// the return value is always a positive integer, or zero, with no
// way of indicating to caller, that there was more data than the
// buffer could accommodate.  A future enhancement could mitigate
// this, returning a negative value in the event of truncation, or
// even allowing for piecewise retrieval of excessively long lines
// in successive reads; (this may be necessary to properly support
// DSC_LINE_MAX_IGNORED, which is currently unimplemented).
//
int psbb_locator::get_line(int dscopt)
{
  int c, count = 0;
  do {
    // Collect input characters into caller's buffer, until we
    // encounter a line terminator, or end of file...
    //
    while (((c = getc(fp)) != '\n') && (c != '\r') && (c != EOF)) {
      if ((((lastc = c) < 0x1b) && !white_space(c)) || (c == 0x7f))
	//
	// ...rejecting any which may be designated as invalid.
	//
	error("invalid input character code %1 in '%2'", int(c), filename);

      // On reading a valid input character, and when there is
      // room in caller's buffer...
      //
      else if (count < DSC_LINE_MAX)
	//
	// ...store it.
	//
	buf[count++] = c;

      // We have a valid input character, but it will not fit
      // into caller's buffer; if enforcing DSC conformity...
      //
      else if (dscopt == DSC_LINE_MAX_ENFORCE) {
	//
	// ...diagnose and truncate.
	//
	dscopt = DSC_LINE_MAX_CHECKED;
	error("PostScript file '%1' is non-conforming "
	      "because length of line exceeds 255", filename);
      }
    }
    // Reading LF may be a special case: when it immediately
    // follows a CR which terminated the preceding input line,
    // we deem it to complete a CRLF terminator for the already
    // collected preceding line; discard it, and restart input
    // collection for the current line.
    //
  } while ((lastc == '\r') && ((lastc = c) == '\n'));

  // For each collected input line, record its actual terminator,
  // substitute our preferred LF terminator...
  //
  if (((lastc = c) != EOF) || (count > 0))
    buf[count++] = '\n';

  // ...and append the required C-string (NUL) terminator, before
  // returning the actual count of input characters stored.
  //
  buf[count] = '\0';
  return count;
}

// psbb_locator::context_args()
//
// Inputs:
//   tag   literal text to be matched at start of input line
//
// Returns a pointer to the trailing substring of the current
// input line, following an initial substring matching the "tag"
// argument, or 0 if "tag" is not matched.
//
inline const char *psbb_locator::context_args(const char *tag)
{
  return context_args(tag, buf);
}

// psbb_locator::context_args()
//
// Overloaded variant of the preceding function, operating on
// an alternative input buffer, (which may represent a terminal
// substring of the psbb_locator's primary input line buffer).
//
// Inputs:
//   tag   literal text to be matched at start of buffer
//   p     pointer to text to be checked for "tag" match
//
// Returns a pointer to the trailing substring of the specified
// text buffer, following an initial substring matching the "tag"
// argument, or 0 if "tag" is not matched.
//
inline const char *psbb_locator::context_args(const char *tag, const char *p)
{
  size_t len = strlen(tag);
  return (strncmp(tag, p, len) == 0) ? p + len : 0 /* nullptr */;
}

// psbb_locator::bounding_box_args()
//
// Returns a pointer to the arguments string, within the current
// input line, when this represents a PostScript "%%BoundingBox:"
// comment, or 0 otherwise.
//
inline const char *psbb_locator::bounding_box_args(void)
{
  return context_args("%%BoundingBox:");
}

// psbb_locator::assign_registers()
//
// Copies the bounding box properties established within the
// class object, to the associated gtroff registers.
//
inline void psbb_locator::assign_registers(void)
{
  llx_reg_contents = llx;
  lly_reg_contents = lly;
  urx_reg_contents = urx;
  ury_reg_contents = ury;
}

// psbb_locator::get_header_comment()
//
// Fetch a line of PostScript input; return true if it complies with
// the formatting requirements for header comments, and it is not an
// "%%EndComments" line; otherwise return false.
//
inline bool psbb_locator::get_header_comment(void)
{
  return
    // The first necessary requirement, for returning true,
    // is that the input line is not empty, (i.e. not EOF).
    //
    get_line(DSC_LINE_MAX_ENFORCE) != 0

    // In header comments, '%X' ('X' any printable character
    // except whitespace) is also acceptable.
    //
    && (buf[0] == '%') && !white_space(buf[1])

    // Finally, the input line must not say "%%EndComments".
    //
    && context_args("%%EndComments") == 0 /* nullptr */;
}

// psbb_locator::skip_to_trailer()
//
// Reposition the PostScript input stream, such that the next get_line()
// will retrieve the first line, if any, following a "%%Trailer" comment;
// returns a positive integer value if the "%%Trailer" comment is found,
// or zero if it is not.
//
inline int psbb_locator::skip_to_trailer(void)
{
  // Begin by considering a chunk of the input file starting 512 bytes
  // before its end, and search it for a "%%Trailer" comment; if none is
  // found, incrementally double the chunk size while it remains within
  // a 32768L byte range, and search again...
  //
  for (ssize_t offset = 512L; offset > 0L; offset <<= 1) {
    int status, failed;
    if ((offset > 32768L) || ((failed = fseek(fp, -offset, SEEK_END)) != 0))
      //
      // ...ultimately resetting the offset to zero, and simply seeking
      // to the start of the file, to terminate the cycle and do a "last
      // ditch" search of the entire file, if any backward seek fails, or
      // if we reach the arbitrary 32768L byte range limit.
      //
      failed = fseek(fp, offset = 0L, SEEK_SET);

    // Following each successful seek...
    //
    if (!failed) {
      //
      // ...perform a search by reading lines from the input stream...
      //
      do { status = get_line(DSC_LINE_MAX_ENFORCE);
	   //
	   // ...until we either exhaust the available stream data, or
	   // we have located a "%%Trailer" comment line.
	   //
	 } while ((status != 0)
	          && (context_args("%%Trailer") == 0 /* nullptr */));
      if (status > 0)
	//
	// We found the "%%Trailer" comment, so we may immediately
	// return, with the stream positioned appropriately...
	//
	return status;
    }
  }
  // ...otherwise, we report that no "%%Trailer" comment was found.
  //
  return 0;
}

void ps_bbox_request() // .psbb
{
  if (!has_arg(true /* peek */)) {
    warning(WARN_MISSING, "PostScript file bounding box extraction"
	    " request expects an argument");
    skip_line();
    return;
  }
  // Parse input line, to extract file name.
  //
  symbol nm = read_long_identifier(true /* required */);
  if (nm.is_null())
    //
    // No file name specified: ignore the entire request.
    //
    skip_line();
  else {
    // File name acquired: swallow the rest of the line.
    //
    while (!tok.is_newline() && !tok.is_eof())
      tok.next();
    errno = 0;

    // Update {llx,lly,urx,ury}_reg_contents:
    // declaring this class instance achieves this, as an
    // intentional side effect of object construction.
    //
    psbb_locator do_ps_file(nm.contents());

    // All done for .psbb; move on, to continue
    // input stream processing.
    //
    tok.next();
  }
}

// Encode a token for output to an operating system file stream.
// Express unencodable tokens as null characters.
const char *encode_for_stream_output(int c)
{
  static char buf[3];
  buf[0] = (0U == escape_char) ? '\\' : escape_char;
  buf[1] = buf[2] = '\0';
  switch (c) {
  case ESCAPE_QUESTION:
    buf[1] = '?';
    break;
  case ESCAPE_AMPERSAND:
    buf[1] = '&';
    break;
  case ESCAPE_RIGHT_PARENTHESIS:
    buf[1] = ')';
    break;
  case ESCAPE_UNDERSCORE:
    buf[1] = '_';
    break;
  case ESCAPE_BAR:
    buf[1] = '|';
    break;
  case ESCAPE_CIRCUMFLEX:
    buf[1] = '^';
    break;
  case ESCAPE_LEFT_BRACE:
    buf[1] = '{';
    break;
  case ESCAPE_RIGHT_BRACE:
    buf[1] = '}';
    break;
  case ESCAPE_LEFT_QUOTE:
    buf[1] = '`';
    break;
  case ESCAPE_RIGHT_QUOTE:
    buf[1] = '\'';
    break;
  case ESCAPE_HYPHEN:
    buf[1] = '-';
    break;
  case ESCAPE_BANG:
    buf[1] = '!';
    break;
  case ESCAPE_c:
    buf[1] = 'c';
    break;
  case ESCAPE_e:
    buf[1] = 'e';
    break;
  case ESCAPE_E:
    buf[1] = 'E';
    break;
  case ESCAPE_PERCENT:
    buf[1] = '%';
    break;
  case ESCAPE_SPACE:
    buf[1] = ' ';
    break;
  case ESCAPE_TILDE:
    buf[1] = '~';
    break;
  case ESCAPE_COLON:
    buf[1] = ':';
    break;
  case PUSH_GROFF_MODE:
  case PUSH_COMP_MODE:
  case POP_GROFFCOMP_MODE:
    buf[0] = '\0';
    break;
  default:
    if (is_invalid_input_char(c))
      buf[0] = '\0';
    else
      buf[0] = c;
    break;
  }
  return buf;
}

const char *input_char_description(int c)
{
  switch (c) {
  case '\n':
    return "a newline character";
  case '\b':
    return "a backspace character";
  case '\001':
    return "a leader character";
  case '\t':
    return "a tab character";
  case ' ':
    return "a space character";
  case '\0':
    return "a node";
  }
  const size_t bufsz = sizeof "magic character code " + INT_DIGITS + 1;
  static char buf[bufsz];
  (void) memset(buf, 0, bufsz);
  if (is_invalid_input_char(c)) {
    const char *s = encode_for_stream_output(c);
    if (*s) {
      buf[0] = '\'';
      strcpy(buf + 1, s);
      strcat(buf, "'");
      return buf;
    }
    sprintf(buf, "magic character code %d", c);
    return buf;
  }
  if (csprint(c)) {
    if ('\'' == c) {
      buf[0] = '"';
      buf[1] = c;
      buf[2] = '"';
    }
    else {
      buf[0] = '\'';
      buf[1] = c;
      buf[2] = '\'';
    }
    return buf;
  }
  sprintf(buf, "character code %d", c);
  return buf;
}

void tag()
{
  if (has_arg(true /* peek */)) {
    string s;
    int c;
    for (;;) {
      c = read_char_in_copy_mode(0 /* nullptr */);
      if (c == '"') {
	c = read_char_in_copy_mode(0 /* nullptr */);
	break;
      }
      if (c != ' ' && c != '\t')
	break;
    }
    s = "x X ";
    for (;
	 (c != '\n') && (c != EOF);
	 (c = read_char_in_copy_mode(0 /* nullptr */)))
      s += (char) c;
    s += '\n';
    curenv->add_node(new tag_node(s, 0));
  }
  tok.next();
}

void taga()
{
  if (has_arg(true /* peek */)) {
    string s;
    int c;
    for (;;) {
      c = read_char_in_copy_mode(0 /* nullptr */);
      if (c == '"') {
	c = read_char_in_copy_mode(0 /* nullptr */);
	break;
      }
      if (c != ' ' && c != '\t')
	break;
    }
    s = "x X ";
    for (;
	 (c != '\n') && (c != EOF);
	 (c = read_char_in_copy_mode(0 /* nullptr */)))
      s += (char) c;
    s += '\n';
    curenv->add_node(new tag_node(s, 1));
  }
  tok.next();
}

// .tm, .tm1, and .tmc

// TODO: Migrate `tm` (and `ab`) to work like `tm1`, interpreting a
// leading `"` as `ds` does (and a bunch of other requests do).
//
// This would leave `tm1` without a distinct function, so we could
// retire it.
//
// Separately, we could make `tm` (and/or `ab`) do old-style argument
// interpretation only in compatibility mode.  We still wouldn't need
// `tm1` because a compatibility mode document could say ".do tm foo".

static void terminal_write(bool do_append_newline,
			   bool interpret_leading_spaces)
{
  if (has_arg(true /* peek */)) {
    int c;
    for (;;) {
      c = read_char_in_copy_mode(0 /* nullptr */);
      if (interpret_leading_spaces && ('"' == c)) {
	c = read_char_in_copy_mode(0 /* nullptr */);
	break;
      }
      if ((c != ' ') && (c != '\t'))
	break;
    }
    for (;
	 (c != '\n') && (c != EOF);
	 (c = read_char_in_copy_mode(0 /* nullptr */)))
      fputs(encode_for_stream_output(c), stderr);
  }
  if (do_append_newline)
    fputc('\n', stderr);
  fflush(stderr);
  tok.next();
}

// old and busted
static void terminal_message_request() // .tm
{
  terminal_write(true /* do append newline */ ,
		 false /* interpret leading spaces */);
}

// the new hotness
static void terminal_message1_request() // .tm1
{
  terminal_write(true /* do append newline */ ,
		 true /* interpret leading spaces */);
}

static void terminal_message_continuation_request() // .tmc
{
  terminal_write(false /* do append newline */ ,
		 true /* interpret leading spaces */);
}

struct grostream : object {
  const symbol filename;
  const symbol mode;
  FILE * const file;
  grostream(const char *fn, symbol m, FILE *fp);
  ~grostream();
};

grostream::grostream(const char *fn, symbol m, FILE *fp)
: filename(fn), mode(m), file(fp)
{
}

grostream::~grostream()
{
}

object_dictionary stream_dictionary(20);

static void print_stream_request() // .pstream
{
  object_dictionary_iterator iter(stream_dictionary);
  symbol stream_name;
  grostream *grost;
  errprint("[");
  bool need_comma = false;
  while (iter.get(&stream_name, (object **)&grost)) {
    assert(!stream_name.is_null());
    if (stream_name != 0 /* nullptr */) {
      if (need_comma)
	errprint(", ");
      errprint("{\"stream\": ");
      stream_name.json_dump();
      errprint(", \"file name\": ");
      grost->filename.json_dump();
      errprint(", \"mode\": ");
      grost->mode.json_dump();
      errprint("}");
      fflush(stderr);
      need_comma = true;
    }
  }
  // !need_comma implies that the list was empty.  JSON convention is to
  // put a space between an empty pair of square brackets.
  if (!need_comma)
    errprint(" ");
  errprint("]\n");
  fflush(stderr);
  skip_line();
}

static void open_file(bool appending)
{
  symbol stream = read_identifier(true /* required */);
  if (!stream.is_null()) {
    char *filename = read_rest_of_line_as_argument();
    if (filename != 0 /* nullptr */) {
      const char *mode = appending ? "appending" : "writing";
      errno = 0;
      FILE *fp = fopen(filename, appending ? "a" : "w");
      if (0 /* nullptr */ == fp) {
	error("cannot open file '%1' for %2: %3", filename, mode,
	      strerror(errno));
	// If we already had a key of this name in the dictionary, it's
	// invalid now.
	stream_dictionary.remove(stream);
      }
      else {
	grostream *oldgrost = static_cast<grostream *>(stream_dictionary
						       .lookup(stream));
	if (oldgrost != 0 /* nullptr */) {
	  FILE *oldfp = oldgrost->file;
	  assert(oldfp != 0 /* nullptr */);
	  if (oldfp != 0 /* nullptr */ && (fclose(oldfp) != 0)) {
	    error("cannot close file '%1' already associated with"
		  " stream '%2': %3", filename, strerror(errno));
	    return;
	  }
	}
	stream_dictionary.define(stream,
				 new grostream(filename, mode, &*fp));
      }
    }
    tok.next();
  }
}

static void open_request() // .open
{
  if (!has_arg(true /* peek */)) {
    warning(WARN_MISSING, "file writing request expects arguments");
    skip_line();
    return;
  }
  if (!want_unsafe_requests) {
    error("file writing request is not allowed in safer mode");
    skip_line();
  }
  else
    open_file(false /* appending */);
  // No skip_line() here; open_file() calls
  // read_rest_of_line_as_argument(), tok.next().
}

static void opena_request() // .opena
{
  if (!has_arg(true /* peek */)) {
    warning(WARN_MISSING, "file appending request expects arguments");
    skip_line();
    return;
  }
  if (!want_unsafe_requests) {
    error("file appending request is not allowed in safer mode");
    skip_line();
  }
  else
    open_file(true /* appending */);
  // No skip_line() here; open_file() calls
  // read_rest_of_line_as_argument(), tok.next().
}

static void close_stream(symbol &stream)
{
  assert(!stream.is_null());
  bool is_valid = false;
  FILE *fp = 0 /* nullptr */;
  grostream *grost = static_cast<grostream *>(stream_dictionary
					      .lookup(stream));
  if (grost != 0 /* nullptr */) {
    fp = grost->file;
    // We shouldn't have stored a null pointer in the first place.
    assert(fp != 0 /* nullptr */);
    if (fp != 0 /* nullptr */)
      is_valid = true;
  }
  if (!is_valid) {
    error("cannot close nonexistent stream '%1'", stream.contents());
    return;
  }
  else {
    if (fclose(fp) != 0) {
      error("cannot close stream '%1': %2", stream.contents(),
	    strerror(errno));
      return;
    }
  }
  stream_dictionary.remove(stream);
}

// Call this from exit_troff().
static void close_all_streams()
{
  object_dictionary_iterator iter(stream_dictionary);
  FILE *filestream;
  symbol stream;
  while (iter.get(&stream, (object **)&filestream)) {
    assert(!stream.is_null());
    if (stream != 0 /* nullptr */) {
      warning(WARN_FILE, "stream '%1' still open; closing",
	      stream.contents());
      close_stream(stream);
    }
  }
}

static void close_request() // .close
{
  if (!has_arg(true /* peek */)) {
    warning(WARN_MISSING, "stream closing request expects an argument");
    skip_line();
    return;
  }
  symbol stream = read_identifier();
  // Testing has_arg() should have ensured this.
  assert(stream != 0 /* nullptr */);
  if (!stream.is_null())
    close_stream(stream);
  skip_line();
}

// .write and .writec

static void do_write_request(bool do_append_newline)
{
  symbol stream = read_identifier(true /* required */);
  if (stream.is_null()) {
    skip_line();
    return;
  }
  grostream *grost = static_cast<grostream *>(stream_dictionary
					      .lookup(stream));
  if (0 /* nullptr */ == grost) {
    error("cannot write to nonexistent stream '%1'", stream.contents());
    skip_line();
    return;
  }
  // Invariant: if the groff stream exists, the backing C stream must.
  assert(grost->file != 0 /* nullptr */);
  FILE *fp = grost->file;
  if (0 /* nullptr */ == fp) {
    error("cannot write to nonexistent stream '%1'", stream.contents());
    skip_line();
    return;
  }
  if (has_arg(true /* peek */)) {
    int c = read_char_in_copy_mode(0 /* nullptr */);
    while (' ' == c)
      c = read_char_in_copy_mode(0 /* nullptr */);
    if ('"' == c)
      c = read_char_in_copy_mode(0 /* nullptr */);
    while (c != '\n' && c != EOF) {
      fputs(encode_for_stream_output(c), fp);
      c = read_char_in_copy_mode(0 /* nullptr */);
    }
  }
  if (do_append_newline)
    fputc('\n', fp);
  fflush(fp);
  tok.next();
}

static void stream_write_request() // .write
{
  do_write_request(true /* do append newline */);
}

static void stream_write_continuation_request() // .writec
{
  do_write_request(false /* do append newline */);
}

static void stream_write_macro_request() // .writem
{
  symbol stream = read_identifier(true /* required */);
  if (stream.is_null()) {
    skip_line();
    return;
  }
  grostream *grost = static_cast<grostream *>(stream_dictionary
					      .lookup(stream));
  FILE *fp = grost->file;
  if (0 /* nullptr */ == fp) {
    error("no stream named '%1'", stream.contents());
    skip_line();
    return;
  }
  symbol s = read_identifier(true /* required */);
  if (s.is_null()) {
    skip_line();
    return;
  }
  request_or_macro *p = lookup_request(s);
  macro *m = p->to_macro();
  if (0 /* nullptr */ == m)
    error("cannot write request '%1' to a stream", s.contents());
  else {
    string_iterator iter(*m);
    for (;;) {
      int c = iter.get(0 /* nullptr */);
      if (c == EOF)
	break;
      fputs(encode_for_stream_output(c), fp);
    }
    fflush(fp);
  }
  skip_line();
}

void warnscale_request() // .warnscale
{
  if (!has_arg()) {
    warning(WARN_MISSING, "warning scaling unit configuration request"
	    " expects a scaling unit argument");
    skip_line();
    return;
  }
  int c = tok.ch(); // safely compares to char literals; TODO: grochar
  if ('u' == c)
    warn_scale = 1.0;
  else if ('i' == c)
    warn_scale = double(units_per_inch);
  else if ('c' == c)
    warn_scale = double(units_per_inch / 2.54);
  else if ('p' == c)
    warn_scale = double(units_per_inch / 72.0);
  else if ('P' == c)
    warn_scale = double(units_per_inch / 6.0);
  else {
    warning(WARN_SCALE,
	    "%1 is not a valid scaling unit; using 'i'",
	    tok.description());
    c = 'i';
    warn_scale = double(units_per_inch);
  }
  warn_scaling_unit = c;
  skip_line();
}

void spreadwarn_request() // .spreadwarn
{
  hunits n;
  if (has_arg() && read_hunits(&n, 'm')) {
    if (n < 0)
      n = 0;
    hunits em = curenv->get_size();
    spread_limit = (double) n.to_units()
		   / (em.is_zero() ? hresolution : em.to_units());
  }
  else
    spread_limit = -spread_limit - 1;	// no arg toggles on/off without
					// changing value; we mirror at
					// -0.5 to make zero a valid value
  skip_line();
}

// Keep this in sync with "src/libs/libgroff/nametoindex.cpp".
// constexpr // C++11
static const char char_prefix[] = { 'c', 'h', 'a', 'r' };
// constexpr // C++11
static const size_t char_prefix_len = sizeof char_prefix;

static void init_charset_table()
{
  char buf[16];
  (void) strncpy(buf, char_prefix, char_prefix_len);
  for (int i = 0; i < 256; i++) {
    (void) strcpy((buf + char_prefix_len), i_to_a(i));
    charset_table[i] = lookup_charinfo(symbol(buf));
    charset_table[i]->set_ascii_code(i);
    if (csalpha(i))
      charset_table[i]->set_hyphenation_code(cmlower(i));
  }
  charset_table['.']->set_flags(charinfo::ENDS_SENTENCE);
  charset_table['?']->set_flags(charinfo::ENDS_SENTENCE);
  charset_table['!']->set_flags(charinfo::ENDS_SENTENCE);
  charset_table['-']->set_flags(charinfo::ALLOWS_BREAK_AFTER);
  charset_table['"']->set_flags(charinfo::IS_TRANSPARENT_TO_END_OF_SENTENCE);
  charset_table['\'']->set_flags(charinfo::IS_TRANSPARENT_TO_END_OF_SENTENCE);
  charset_table[')']->set_flags(charinfo::IS_TRANSPARENT_TO_END_OF_SENTENCE);
  charset_table[']']->set_flags(charinfo::IS_TRANSPARENT_TO_END_OF_SENTENCE);
  charset_table['*']->set_flags(charinfo::IS_TRANSPARENT_TO_END_OF_SENTENCE);
  lookup_charinfo(symbol("dg"))->set_flags(charinfo::IS_TRANSPARENT_TO_END_OF_SENTENCE);
  lookup_charinfo(symbol("dd"))->set_flags(charinfo::IS_TRANSPARENT_TO_END_OF_SENTENCE);
  lookup_charinfo(symbol("rq"))->set_flags(charinfo::IS_TRANSPARENT_TO_END_OF_SENTENCE);
  lookup_charinfo(symbol("cq"))->set_flags(charinfo::IS_TRANSPARENT_TO_END_OF_SENTENCE);
  lookup_charinfo(symbol("em"))->set_flags(charinfo::ALLOWS_BREAK_AFTER);
  lookup_charinfo(symbol("hy"))->set_flags(charinfo::ALLOWS_BREAK_AFTER);
  lookup_charinfo(symbol("ul"))->set_flags(charinfo::OVERLAPS_HORIZONTALLY);
  lookup_charinfo(symbol("rn"))->set_flags(charinfo::OVERLAPS_HORIZONTALLY);
  lookup_charinfo(symbol("radicalex"))->set_flags(charinfo::OVERLAPS_HORIZONTALLY);
  lookup_charinfo(symbol("sqrtex"))->set_flags(charinfo::OVERLAPS_HORIZONTALLY);
  lookup_charinfo(symbol("ru"))->set_flags(charinfo::OVERLAPS_HORIZONTALLY);
  lookup_charinfo(symbol("br"))->set_flags(charinfo::OVERLAPS_VERTICALLY);
  page_character = charset_table['%'];
}

static void init_hpf_code_table()
{
  for (int i = 0; i < 256; i++)
    hpf_code_table[i] = cmlower(i);
}

static void do_translate(bool transparently, bool as_input)
{
  tok.skip_spaces();
  while (!tok.is_newline() && !tok.is_eof()) {
    if (tok.is_space()) {
      // This is a really bizarre troff feature.
      tok.next();
      translate_space_to_dummy = tok.is_dummy();
      if (tok.is_newline() || tok.is_eof())
	break;
      error("cannot translate space character; ignoring");
      tok.next();
      continue;
    }
    charinfo *ci1 = tok.get_charinfo(true /* required */);
    if (0 /* nullptr */ == ci1) {
      assert(0 == "attempted to use token without charinfo in character"
	     " translation request");
      break;
    }
    tok.next();
    if (tok.is_newline() || tok.is_eof()) {
      ci1->set_special_translation(charinfo::TRANSLATE_SPACE,
				   transparently);
      break;
    }
    if (tok.is_space())
      ci1->set_special_translation(charinfo::TRANSLATE_SPACE,
				   transparently);
    else if (tok.is_stretchable_space())
      ci1->set_special_translation(charinfo::TRANSLATE_STRETCHABLE_SPACE,
				   transparently);
    else if (tok.is_dummy())
      ci1->set_special_translation(charinfo::TRANSLATE_DUMMY,
				   transparently);
    else if (tok.is_hyphen_indicator())
      ci1->set_special_translation(charinfo::TRANSLATE_HYPHEN_INDICATOR,
				   transparently);
    else {
      charinfo *ci2 = tok.get_charinfo(true /* required */);
      if (0 /* nullptr */ == ci2) {
	assert(0 == "attempted to use token without charinfo in"
	       " character translation request");
	break;
      }
      if (ci1 == ci2)
	ci1->set_translation(0 /* nullptr */, transparently, as_input);
      else
	ci1->set_translation(ci2, transparently, as_input);
    }
    tok.next();
  }
  skip_line();
}

void translate() // .tr
{
  if (!has_arg()) {
    warning(WARN_MISSING, "character translation request expects"
	    " sequence of character pairs as argument");
    skip_line();
    return;
  }
  do_translate(true /* transparently */, false /* as_input */);
}

void translate_no_transparent() // .trnt
{
  if (!has_arg()) {
    warning(WARN_MISSING, "character non-diversion translation request"
	    " expects sequence of character pairs as argument");
    skip_line();
    return;
  }
  do_translate(false /* transparently */, false /* as_input */);
}

void translate_input() // .trin
{
  if (!has_arg()) {
    warning(WARN_MISSING, "character non-asciification translation"
	    " request expects sequence of character pairs as argument");
    skip_line();
    return;
  }
  do_translate(true /* transparently */, true /* as_input */);
}

static void set_character_flags_request() // .cflags
{
  if (!has_arg()) {
    warning(WARN_MISSING, "character flags configuration request"
	    " expects arguments");
    skip_line();
    return;
  }
  int flags;
  if (read_integer(&flags)) {
    if ((flags < 0) || (flags > charinfo::CFLAGS_MAX)) {
      warning(WARN_RANGE, "character flags must be in range 0..%1,"
	      " got %2", charinfo::CFLAGS_MAX, flags);
      skip_line();
      return;
    }
    if (((flags & charinfo::ENDS_SENTENCE)
	  && (flags & charinfo::IS_TRANSPARENT_TO_END_OF_SENTENCE))
	|| ((flags & charinfo::ALLOWS_BREAK_BEFORE)
	  && (flags & charinfo::PROHIBITS_BREAK_BEFORE))
	|| ((flags & charinfo::ALLOWS_BREAK_AFTER)
	  && (flags & charinfo::PROHIBITS_BREAK_AFTER))) {
      warning(WARN_SYNTAX, "ignoring contradictory character flags: "
	      "%1", flags);
      skip_line();
      return;
    }
    if (!has_arg()) {
      warning(WARN_MISSING, "character flags configuration request"
	      " expects one or more characters to configure");
      skip_line();
      return;
    }
    while (has_arg()) {
      charinfo *ci = tok.get_charinfo(true /* required */);
      if (0 /* nullptr */ == ci)
	assert(0 == "attempted to use token without charinfo in"
	       " character flags assignment request");
      else {
	charinfo *tem = ci->get_translation();
	if (tem != 0 /* nullptr */)
	  ci = tem;
	ci->set_flags(flags);
      }
      tok.next();
    }
  }
  skip_line();
}

static void set_hyphenation_codes() // .hcode
{
  if (!has_arg()) {
    warning(WARN_MISSING, "hyphenation code assignment request expects"
	    " arguments");
    skip_line();
    return;
  }
  while (has_arg()) {
    unsigned char cdst = tok.ch();
    if (csdigit(cdst)) {
      error("cannot apply a hyphenation code to a numeral");
      break;
    }
    charinfo *cidst = tok.get_charinfo();
    if ('\0' == cdst) {
      if (0 /* nullptr */ == cidst) {
	error("expected ordinary, special, or indexed character,"
	      " got %1", tok.description());
	break;
      }
    }
    tok.next();
    if (!has_arg()) {
      error("hyphenation codes must be specified in pairs");
      break;
    }
    unsigned char csrc = tok.ch();
    if (csdigit(csrc)) {
      error("cannot use the hyphenation code of a numeral");
      break;
    }
    unsigned char new_code = 0U;
    charinfo *cisrc = tok.get_charinfo();
    if (cisrc != 0 /* nullptr */)
      // Common case: assign destination character the hyphenation code
      // of the source character.
      new_code = cisrc->get_hyphenation_code();
    if ('\0' == csrc) {
      if (0 /* nullptr */ == cisrc) {
	error("expected ordinary, special, or indexed character,"
	      " got %1", tok.description());
	break;
      }
      new_code = cisrc->get_hyphenation_code();
    }
    else {
      // If assigning a ordinary character's hyphenation code to itself,
      // use its character code point as the value.
      if (csrc == cdst)
	new_code = tok.ch();
    }
    cidst->set_hyphenation_code(new_code);
    if (cidst->get_translation()
	&& cidst->get_translation()->is_translatable_as_input())
      cidst->get_translation()->set_hyphenation_code(new_code);
    tok.next();
    tok.skip_spaces();
  }
  skip_line();
}

void hyphenation_patterns_file_code() // .hpfcode
{
  error("hyphenation pattern file code assignment request will be"
	" withdrawn in a future groff release; migrate to 'hcode'");
  if (!has_arg()) {
    warning(WARN_MISSING, "hyphenation pattern file code assignment"
	    " request expects arguments");
    skip_line();
    return;
  }
  while (!tok.is_newline() && !tok.is_eof()) {
    int n1, n2;
    if (read_integer(&n1) && ((0 <= n1) && (n1 <= 255))) {
      if (!has_arg()) {
	error("missing output hyphenation code");
	break;
      }
      if (read_integer(&n2) && ((0 <= n2) && (n2 <= 255))) {
	hpf_code_table[n1] = n2;
	tok.skip_spaces();
      }
      else {
	error("output hyphenation code must be integer in the range 0..255");
	break;
      }
    }
    else {
      error("input hyphenation code must be integer in the range 0..255");
      break;
    }
  }
  skip_line();
}

dictionary char_class_dictionary(501);

static void define_class_request() // .class
{
  tok.skip_spaces();
  symbol nm = read_identifier(true /* required */);
  if (nm.is_null()) {
    skip_line();
    return;
  }
  charinfo *ci = lookup_charinfo(nm);
  // Assign the charinfo an empty macro as a hack to record the
  // file:line location of its definition.
  macro *m = new macro;
  (void) ci->set_macro(m);
  charinfo *child1 = 0 /* nullptr */, *child2 = 0 /* nullptr */;
  bool just_chained_a_range_expression = false;
  while (!tok.is_newline() && !tok.is_eof()) {
    tok.skip_spaces();
    // Chained range expressions like
    //   \[u3041]-\[u3096]-\[u30FF]
    // are not valid.
    // TODO: use grochar
    if ((child1 != 0 /* nullptr */) && (tok.ch() == int('-'))) {
      tok.next();
      child2 = tok.get_charinfo();
      if (0 /* nullptr */ == child2) {
	warning(WARN_MISSING,
		"missing end of character range in class '%1'",
		nm.contents());
	skip_line();
	return;
      }
      if (child1->is_class() || child2->is_class()) {
	warning(WARN_SYNTAX,
		"a nested character class is not allowed in a range"
		" definition");
	skip_line();
	return;
      }
      int u1 = child1->get_unicode_mapping();
      int u2 = child2->get_unicode_mapping();
      if (u1 < 0) {
	warning(WARN_SYNTAX,
		"invalid start value in character range");
	skip_line();
	return;
      }
      if (u2 < 0) {
	warning(WARN_SYNTAX,
		"invalid end value in character range");
	skip_line();
	return;
      }
      ci->add_to_class(u1, u2);
      child1 = child2 = 0 /* nullptr */;
      just_chained_a_range_expression = true;
    }
    else if (child1 != 0 /* nullptr */) {
      if (child1->is_class()) {
	if (ci == child1) {
	  warning(WARN_SYNTAX, "cannot nest character classes");
	  skip_line();
	  return;
	}
	ci->add_to_class(child1);
      }
      else {
	int u1 = child1->get_unicode_mapping();
	if (u1 < 0) {
	  warning(WARN_SYNTAX,
		  "invalid character value in class '%1'",
		  nm.contents());
	  skip_line();
	  return;
	}
	ci->add_to_class(u1);
      }
      child1 = 0 /* nullptr */;
    }
    if (tok.is_any_character())
      child1 = tok.get_charinfo(true /* required */);
    else
      // If we encountered a space or nonsense, we cannot be
      // interpreting a range expression; there should be no "child1".
      assert(0 /* nullptr */ == child1);
    tok.next();
    if (0 /* nullptr */ == child1) {
      if (!tok.is_newline())
	skip_line();
      break;
    }
    if (just_chained_a_range_expression) {
      // Throw away `child1` so we don't duplicatively add the second
      // end point of a range as a singleton.  See Savannah #67718.
      child1 = 0 /* nullptr */;
      just_chained_a_range_expression = false;
    }
  }
  if (child1 != 0 /* nullptr */) {
    if (child1->is_class()) {
      if (ci == child1) {
	warning(WARN_SYNTAX, "cannot nest character classes");
	skip_line();
	return;
      }
      ci->add_to_class(child1);
    }
    else {
      int u1 = child1->get_unicode_mapping();
      if (u1 < 0) {
	warning(WARN_SYNTAX,
		"invalid character value in class '%1'",
		nm.contents());
	skip_line();
	return;
      }
      ci->add_to_class(u1);
    }
    child1 = 0 /* nullptr */;
  }
  assert(ci != 0 /* nullptr */);
  if (ci != 0 /* nullptr */ && !ci->is_class()) {
    warning(WARN_SYNTAX,
	    "empty class definition for '%1'",
	    nm.contents());
    skip_line();
    return;
  }
  (void) char_class_dictionary.lookup(nm, ci);
  skip_line();
}

// forward declaration
static charinfo *get_charinfo_by_index(int n,
				       bool suppress_creation = false);

charinfo *token::get_charinfo(bool required, bool suppress_creation)
{
  if (TOKEN_CHAR == type)
    return charset_table[c];
  if ((TOKEN_SPECIAL_CHAR == type)
      || (TOKEN_DELIMITED_SPECIAL_CHAR == type))
    return lookup_charinfo(nm, suppress_creation);
  if (TOKEN_INDEXED_CHAR == type)
    return get_charinfo_by_index(val, suppress_creation);
  if (TOKEN_ESCAPE == type) {
    if (escape_char != 0U)
      return charset_table[escape_char];
    else {
      // XXX: Is this possible?  token::add_to_zero_width_node_list()
      // and token::process() don't add this token type if the escape
      // character is null.  If not, this should be an assert().  Also
      // see escape_off_request().
      error("escaped 'e' used while escape sequences disabled");
      return 0 /* nullptr */;
    }
  }
  if (required) {
    if (TOKEN_EOF == type || TOKEN_NEWLINE == type)
      warning(WARN_MISSING, "missing ordinary, special, or indexed"
			    " character");
    else
      error("expected ordinary, special, or indexed character, got %1",
	    description());
  }
  return 0 /* nullptr */;
}

charinfo *read_character(/* TODO?: bool required */)
{
  tok.skip_spaces();
  charinfo *ci = tok.get_charinfo();
  // TODO?: if (required && (0 /* nullptr */ == ci))
  if (0 /* nullptr */ == ci)
    tok.diagnose_non_character();
  else
    tok.next();
  return ci;
}

// this is for \Z

bool token::add_to_zero_width_node_list(node **pp)
{
  hunits w;
  int s = 0; /* space count, possibly populated by `nspaces()` */
  node *n = 0 /* nullptr */;
  switch (type) {
  case TOKEN_CHAR:
    *pp = (*pp)->add_char(charset_table[c], curenv, &w, &s);
    break;
  case TOKEN_DUMMY:
    n = new dummy_node;
    break;
  case TOKEN_ESCAPE:
    if (escape_char != 0U)
      *pp = (*pp)->add_char(charset_table[escape_char], curenv, &w, &s);
    break;
  case TOKEN_HYPHEN_INDICATOR:
    *pp = (*pp)->add_discretionary_hyphen();
    break;
  case TOKEN_ITALIC_CORRECTION:
    *pp = (*pp)->add_italic_correction(&w);
    break;
  case TOKEN_LEFT_BRACE:
    break;
  case TOKEN_MARK_INPUT:
    set_register(nm, curenv->get_input_line_position().to_units());
    break;
  case TOKEN_NODE:
  case TOKEN_DELIMITED_HORIZONTAL_MOTION:
  case TOKEN_HORIZONTAL_MOTION:
    n = nd;
    nd = 0 /* nullptr */;
    break;
  case TOKEN_INDEXED_CHAR:
    *pp = (*pp)->add_char(get_charinfo_by_index(val), curenv, &w, &s);
    break;
  case TOKEN_RIGHT_BRACE:
    break;
  case TOKEN_SPACE:
    n = new hmotion_node(curenv->get_space_width(),
			 curenv->get_fill_color());
    break;
  case TOKEN_SPECIAL_CHAR:
  case TOKEN_DELIMITED_SPECIAL_CHAR:
    *pp = (*pp)->add_char(lookup_charinfo(nm), curenv, &w, &s);
    break;
  case TOKEN_STRETCHABLE_SPACE:
    n = new unbreakable_space_node(curenv->get_space_width(),
				   curenv->get_fill_color());
    break;
  case TOKEN_UNSTRETCHABLE_SPACE:
    n = new space_char_hmotion_node(curenv->get_space_width(),
				    curenv->get_fill_color());
    break;
  case TOKEN_TRANSPARENT_DUMMY:
    n = new transparent_dummy_node;
    break;
  case TOKEN_ZERO_WIDTH_BREAK:
    n = new space_node(H0, curenv->get_fill_color());
    n->freeze_space();
    n->is_escape_colon();
    break;
  default:
    return false;
  }
  if (n != 0 /* nullptr */) {
    n->next = *pp;
    *pp = n;
  }
  return true;
}

void token::process()
{
  if (possibly_handle_first_page_transition())
    return;
  switch (type) {
  case TOKEN_BACKSPACE:
    curenv->add_node(new hmotion_node(-curenv->get_space_width(),
				      curenv->get_fill_color()));
    break;
  case TOKEN_CHAR:
    // Optimize `curenv->add_char(get_charinfo())` for token type.
    curenv->add_char(charset_table[c]);
    break;
  case TOKEN_DUMMY:
    curenv->add_node(new dummy_node);
    break;
  case TOKEN_EMPTY:
    assert(0 == "unhandled empty token");
    break;
  case TOKEN_EOF:
    assert(0 == "unhandled end-of-file token");
    break;
  case TOKEN_ESCAPE:
    if (escape_char != 0U)
      curenv->add_char(charset_table[escape_char]);
    break;
  case TOKEN_BEGIN_TRAP:
  case TOKEN_END_TRAP:
  case TOKEN_PAGE_EJECTOR:
    // these are all handled in process_input_stack()
    break;
  case TOKEN_HYPHEN_INDICATOR:
    curenv->add_hyphen_indicator();
    break;
  case TOKEN_INTERRUPT:
    curenv->interrupt();
    break;
  case TOKEN_ITALIC_CORRECTION:
    curenv->add_italic_correction();
    break;
  case TOKEN_LEADER:
    curenv->advance_to_tab_stop(true /* use_leader */);
    break;
  case TOKEN_LEFT_BRACE:
    break;
  case TOKEN_MARK_INPUT:
    set_register(nm, curenv->get_input_line_position().to_units());
    break;
  case TOKEN_NEWLINE:
    curenv->newline();
    break;
  case TOKEN_NODE:
  case TOKEN_DELIMITED_HORIZONTAL_MOTION:
  case TOKEN_HORIZONTAL_MOTION:
    curenv->add_node(nd);
    nd = 0 /* nullptr */;
    break;
  case TOKEN_INDEXED_CHAR:
    // Optimize `curenv->add_char(get_charinfo())` for token type.
    curenv->add_char(get_charinfo_by_index(val));
    break;
  case TOKEN_REQUEST:
    // handled in process_input_stack()
    break;
  case TOKEN_RIGHT_BRACE:
    break;
  case TOKEN_SPACE:
    curenv->space();
    break;
  case TOKEN_SPECIAL_CHAR:
  case TOKEN_DELIMITED_SPECIAL_CHAR:
    // Optimize `curenv->add_char(get_charinfo())` for token type.
    curenv->add_char(lookup_charinfo(nm));
    break;
  case TOKEN_SPREAD:
    curenv->spread();
    break;
  case TOKEN_STRETCHABLE_SPACE:
    curenv->add_node(new unbreakable_space_node(curenv->get_space_width(),
						curenv->get_fill_color()));
    break;
  case TOKEN_UNSTRETCHABLE_SPACE:
    curenv->add_node(new space_char_hmotion_node(curenv->get_space_width(),
						 curenv->get_fill_color()));
    break;
  case TOKEN_TAB:
    curenv->advance_to_tab_stop();
    break;
  case TOKEN_TRANSPARENT:
    break;
  case TOKEN_TRANSPARENT_DUMMY:
    curenv->add_node(new transparent_dummy_node);
    break;
  case TOKEN_ZERO_WIDTH_BREAK:
    {
      node *tmp = new space_node(H0, curenv->get_fill_color());
      tmp->freeze_space();
      tmp->is_escape_colon();
      curenv->add_node(tmp);
      break;
    }
  default:
    assert(0 == "unhandled token type");
  }
}

class nargs_reg : public reg {
public:
  const char *get_string();
};

const char *nargs_reg::get_string()
{
  return i_to_a(input_stack::nargs());
}

class lineno_reg : public reg {
public:
  const char *get_string();
};

const char *lineno_reg::get_string()
{
  int line;
  const char *file;
  if (!input_stack::get_location(false /* allow macro */, &file, &line))
    line = 0;
  return i_to_a(line);
}

class writable_lineno_reg : public general_reg {
public:
  writable_lineno_reg();
  void set_value(units);
  bool get_value(units *);
};

writable_lineno_reg::writable_lineno_reg()
{
}

bool writable_lineno_reg::get_value(units *res)
{
  int line;
  const char *file;
  if (!input_stack::get_location(false /* allow macro */, &file, &line))
    return false;
  *res = line;
  return true;
}

void writable_lineno_reg::set_value(units n)
{
  (void) input_stack::set_location(0, n);
}

class filename_reg : public reg {
public:
  const char *get_string();
};

const char *filename_reg::get_string()
{
  int line;
  const char *file;
  if (input_stack::get_location(false /* allow macro */, &file, &line))
    return file;
  else
    return 0 /* nullptr */;
}

class break_flag_reg : public reg {
public:
  const char *get_string();
};

const char *break_flag_reg::get_string()
{
  return i_to_a(input_stack::get_break_flag());
}

class enclosing_want_att_compat_reg : public reg {
public:
  const char *get_string();
};

const char *enclosing_want_att_compat_reg::get_string()
{
  return i_to_a(want_att_compat_stack.empty() ? 0
		: want_att_compat_stack.top());
}

class readonly_text_register : public reg {
  const char *s;
public:
  readonly_text_register(const char *);
  readonly_text_register(int);
  const char *get_string();
};

readonly_text_register::readonly_text_register(const char *p) : s(p)
{
}

readonly_text_register::readonly_text_register(int i)
{
  s = strdup(i_to_a(i));
}

const char *readonly_text_register::get_string()
{
  return s;
}

readonly_register::readonly_register(int *q) : p(q)
{
}

const char *readonly_register::get_string()
{
  return i_to_a(*p);
}

readonly_boolean_register::readonly_boolean_register(bool *q): p(q)
{
}

const char *readonly_boolean_register::get_string()
{
  return i_to_a(*p);
}

class readonly_mask_register : public reg {
  unsigned int *mask;
public:
  readonly_mask_register(unsigned int *);
  const char *get_string();
};

readonly_mask_register::readonly_mask_register(unsigned int * m)
: mask(m)
{
}

const char *readonly_mask_register::get_string()
{
  return ui_to_a(*mask);
}

void abort_request()
{
  int c;
  if (tok.is_eof())
    c = EOF;
  else if (tok.is_newline())
    c = '\n';
  else {
    while ((c = read_char_in_copy_mode(0 /* nullptr */)) == ' ')
      ;
  }
  if (!(c == EOF || c == '\n')) {
    for (;
	 (c != '\n') && (c != EOF);
	 (c = read_char_in_copy_mode(0 /* nullptr */)))
      fputs(encode_for_stream_output(c), stderr);
    fputc('\n', stderr);
  }
  fflush(stderr);
  write_any_trailer_and_exit(EXIT_FAILURE);
}

// Consume the rest of the input line in copy mode and return it as a C
// string; if, after spaces, the argument starts with a `"`, discard it,
// letting any immediately subsequent spaces populate the returned
// string.
//
// The caller must subsequently call `tok.next()` to advance the input
// stream pointer.
//
// The caller has responsibility for `delete`ing the returned buffer.
char *read_rest_of_line_as_argument()
{
  int buf_size = 256;
  char *s = new char[buf_size]; // C++03: new char[buf_size]();
  (void) memset(s, 0, (buf_size * sizeof(char)));
  int c = read_char_in_copy_mode(0 /* nullptr */);
  while (' ' == c)
    c = read_char_in_copy_mode(0 /* nullptr */);
  if ('"' == c)
    c = read_char_in_copy_mode(0 /* nullptr */);
  int i = 0;
  while ((c != '\n') && (c != EOF)) {
    if (!is_invalid_input_char(c)) {
      if ((i + 2) > buf_size) {
	char *tem = s;
	s = new char[buf_size * 2]; // C++03: new char[buf_size * 2]();
	(void) memset(s, 0, (buf_size * 2 * sizeof(char)));
	memcpy(s, tem, buf_size);
	buf_size *= 2;
	delete[] tem;
      }
      s[i++] = c;
    }
    c = read_char_in_copy_mode(0 /* nullptr */);
  }
  s[i] = '\0';
  if (0 == i) {
    delete[] s;
    return 0 /* nullptr */;
  }
  return s;
}

void pipe_output()
{
  if (!has_arg(true /* peek */)) {
    warning(WARN_MISSING, "output piping request expects a system"
	    " command as argument");
    skip_line();
    return;
  }
  if (!want_unsafe_requests) {
    error("output piping request is not allowed in safer mode");
    skip_line();
    return;
  }
  if (the_output != 0 /* nullptr */) {
    error("cannot honor pipe request: output already started");
    skip_line();
    return;
  }
  char *pc = read_rest_of_line_as_argument();
  // `has_arg()` should have ensured that this pointer is non-null.
  assert(pc != 0 /* nullptr */);
  if (0 /* nullptr */ == pc)
    error("cannot apply pipe request to empty command");
  // Are we adding to an existing pipeline?
  if (pipe_command != 0 /* nullptr */) {
    // ISO C++ does not permit VLAs on the stack.
    // C++03: new char[strlen(pipe_command) + strlen(pc) + 1 + 1]();
    char *s = new char[strlen(pipe_command) + strlen(pc) + 1 + 1];
    (void) memset(s, 0, ((strlen(pipe_command) + strlen(pc) + 1 + 1)
			 * sizeof(char)));
    strcpy(s, pipe_command);
    strcat(s, "|");
    strcat(s, pc);
    delete[] pipe_command;
    delete[] pc;
    pipe_command = s;
  }
  else
    pipe_command = pc;
  delete[] pc;
  tok.next();
}

static int system_status;

void system_request()
{
  if (!has_arg(true /* peek */)) {
    warning(WARN_MISSING, "system command execution request expects a"
	    " system command as argument");
    skip_line();
    return;
  }
  if (!want_unsafe_requests) {
    error("system command execution request is not allowed in safer"
	  " mode");
    skip_line();
    return;
  }
  char *command = read_rest_of_line_as_argument();
  // `has_arg()` should have ensured that this pointer is non-null.
  assert(command != 0 /* nullptr */);
  if (0 /* nullptr */ == command)
    error("cannot apply system request to empty command");
  else
    system_status = system(command);
  delete[] command;
  tok.next();
}

static void unsafe_transparent_throughput_file_request()
{
  if (!has_arg(true /* peek */)) {
    warning(WARN_MISSING, "file throughput request expects a file name"
	    " as argument");
    skip_line();
    return;
  }
  if (!want_unsafe_requests) {
    error("file throughput request is not allowed in safer mode");
    skip_line();
    return;
  }
  if ((curdiv == topdiv) && (topdiv->before_first_page_status > 0)) {
    handle_initial_request(COPY_FILE_REQUEST);
    return;
  }
  char *filename = read_rest_of_line_as_argument();
  if (was_invoked_with_regular_control_character)
    curenv->do_break();
  if (filename != 0 /* nullptr */)
    curdiv->copy_file(filename);
  // TODO: Add `filename` to file name set.
  tok.next();
}

#ifdef COLUMN

void vjustify()
{
  if (!has_arg()) {
    warning(WARN_MISSING, "vertical adjustment request expects an"
	    " argument");
    skip_line();
    return;
  }
  if (curdiv == topdiv && topdiv->before_first_page) {
    handle_initial_request(VJUSTIFY_REQUEST);
    return;
  }
  symbol type = read_long_identifier(true /* required */);
  if (!type.is_null())
    curdiv->vjustify(type);
  skip_line();
}

#endif /* COLUMN */

static void transparent_throughput_file_request()
{
  if (!has_arg(true /* peek */)) {
    warning(WARN_MISSING, "transparent file throughput request expects"
	    " a file name as argument");
    skip_line();
    return;
  }
  if ((curdiv == topdiv) && (topdiv->before_first_page_status > 0)) {
    handle_initial_request(TRANSPARENT_FILE_REQUEST);
    return;
  }
  char *filename = read_rest_of_line_as_argument();
  if (was_invoked_with_regular_control_character)
    curenv->do_break();
  if (filename != 0 /* nullptr */) {
    errno = 0;
    FILE *fp = include_search_path.open_file_cautiously(filename);
    if (0 /* nullptr */ == fp)
      error("cannot open '%1': %2", filename, strerror(errno));
    else {
      if (curdiv != topdiv)
	curdiv->copy_file(filename);
      else {
	bool reading_beginning_of_input_line = true;
	for (;;) {
	  int c = getc(fp);
	  if (c == EOF)
	    break;
	  if (is_invalid_input_char(c))
	    warning(WARN_INPUT, "invalid input character code %1",
		    int(c));
	  else {
	    curdiv->transparent_output(c);
	    reading_beginning_of_input_line = c == '\n';
	  }
	}
	if (!reading_beginning_of_input_line)
	  curdiv->transparent_output('\n');
	fclose(fp);
      }
    }
  }
  tok.next();
}

class page_range {
  int first;
  int last;
public:
  page_range *next;
  page_range(int, int, page_range *);
  int contains(int n);
};

page_range::page_range(int i, int j, page_range *p)
: first(i), last(j), next(p)
{
}

int page_range::contains(int n)
{
  return n >= first && (last <= 0 || n <= last);
}

page_range *output_page_list = 0 /* nullptr */;

bool in_output_page_list(int n)
{
  if (!output_page_list)
    return true;
  for (page_range *p = output_page_list;
       p != 0 /* nullptr */;
       p = p->next)
    if (p->contains(n))
      return true;
  return false;
}

static void parse_output_page_list(const char *p)
{
  const char *pstart = p; // for diagnostic message
  for (;;) {
    int i;
    if (*p == '-')
      i = 1;
    else if (csdigit(*p)) {
      i = 0;
      do
	i = i*10 + *p++ - '0';
      while (csdigit(*p));
    }
    else
      break;
    int j;
    if (*p == '-') {
      p++;
      j = 0;
      if (csdigit(*p)) {
	do
	  j = j*10 + *p++ - '0';
	while (csdigit(*p));
      }
    }
    else
      j = i;
    if (j == 0)
      last_page_number = -1;
    else if (last_page_number >= 0 && j > last_page_number)
      last_page_number = j;
    output_page_list = new page_range(i, j, output_page_list);
    if (*p != ',')
      break;
    ++p;
  }
  if (*p != '\0') {
    error("ignoring invalid output page list argument '%1'", pstart);
    output_page_list = 0 /* nullptr */;
  }
}

static FILE *open_macro_package(const char *mac, char **path)
{
  // Try `mac`.tmac first, then tmac.`mac`.  Expect ENOENT errors.
  // ISO C++ does not permit VLAs on the stack.
  // C++03: new char[strlen(mac) + strlen(MACRO_POSTFIX) + 1]();
  char *s1 = new char[strlen(mac) + strlen(MACRO_POSTFIX) + 1];
  (void) memset(s1, 0, ((strlen(mac) + strlen(MACRO_POSTFIX) + 1)
			* sizeof(char)));
  strcpy(s1, mac);
  strcat(s1, MACRO_POSTFIX);
  FILE *fp = mac_path->open_file(s1, path);
  if ((0 /* nullptr */ == fp) && (ENOENT != errno))
    error("cannot open macro file '%1': %2", s1, strerror(errno));
  delete[] s1;
  if (0 /* nullptr */ == fp) {
    // ISO C++ does not permit VLAs on the stack.
    // C++03: new char[strlen(mac) + strlen(MACRO_PREFIX) + 1]();
    char *s2 = new char[strlen(mac) + strlen(MACRO_PREFIX) + 1];
    (void) memset(s2, 0, ((strlen(mac) + strlen(MACRO_PREFIX) + 1)
			  * sizeof(char)));
    strcpy(s2, MACRO_PREFIX);
    strcat(s2, mac);
    fp = mac_path->open_file(s2, path);
    if ((0 /* nullptr */ == fp) && (ENOENT != errno))
      error("cannot open macro file '%1': %2", s2, strerror(errno));
    delete[] s2;
  }
  return fp;
}

static void process_macro_package_argument(const char *mac)
{
  char *path;
  FILE *fp = open_macro_package(mac, &path);
  if (0 /* nullptr */ == fp)
    fatal("cannot open macro file named in '-m' command-line argument"
	  " '%1': %2", mac, strerror(errno));
  const char *s = symbol(path).contents();
  free(path);
  input_stack::push(new file_iterator(fp, s));
  tok.next();
  process_input_stack();
}

static void process_startup_file(const char *filename)
{
  char *path;
  search_path *orig_mac_path = mac_path;
  mac_path = &config_macro_path;
  FILE *fp = mac_path->open_file(filename, &path);
  if (fp != 0 /* nullptr */) {
    input_stack::push(new file_iterator(fp, symbol(path).contents()));
    free(path);
    tok.next();
    process_input_stack();
  }
  else if (errno != ENOENT)
    error("cannot open startup file '%1': %2", filename,
	  strerror(errno));
  mac_path = orig_mac_path;
}

void do_macro_source(bool quietly)
{
  char *macro_filename = read_rest_of_line_as_argument();
  char *path;
  FILE *fp = mac_path->open_file(macro_filename, &path);
  if (fp != 0 /* nullptr */) {
    input_stack::push(new file_iterator(fp, macro_filename));
    free(path);
  }
  else
    // Suppress diagnostic only if we're operating quietly and it's an
    // expected problem.
    if (!quietly && (ENOENT == errno))
      warning(WARN_FILE, "cannot open macro file '%1': %2",
	      macro_filename, strerror(errno));
  // TODO: Add `macro_filename` to file name set.
  tok.next();
}

void macro_source_request() // .mso
{
  if (!has_arg(true /* peek */)) {
    warning(WARN_MISSING, "macro file sourcing request expects an"
	    " argument");
    skip_line();
    return;
  }
  do_macro_source(false /* quietly */ );
}

// like .mso, but silently ignore files that can't be opened due to
// their nonexistence
void macro_source_quietly_request() // .msoquiet
{
  if (!has_arg(true /* peek */)) {
    warning(WARN_MISSING, "quiet macro file sourcing request expects an"
	    " argument");
    skip_line();
    return;
  }
  do_macro_source(true /* quietly */ );
}

static void process_input_file(const char *name)
{
  FILE *fp;
  if (strcmp(name, "-") == 0) {
    clearerr(stdin);
    fp = stdin;
  }
  else {
    errno = 0;
    fp = include_search_path.open_file_cautiously(name);
    if (0 /* nullptr */ == fp)
      fatal("cannot open '%1': %2", name, strerror(errno));
  }
  input_stack::push(new file_iterator(fp, name));
  tok.next();
  process_input_stack();
}

// make sure the_input is empty before calling this

static int evaluate_expression(const char *expr, units *res)
{
  input_stack::push(make_temp_iterator(expr));
  tok.next();
  // TODO: grochar
  int success = read_measurement(res, (unsigned char)('u'));
  while (input_stack::get(0 /* nullptr */) != EOF)
    ;
  return success;
}

static void do_register_assignment(const char *s)
{
  const char *p = strchr(s, '=');
  if (!p) {
    char buf[2];
    buf[0] = s[0];
    buf[1] = 0;
    units n;
    if (evaluate_expression(s + 1, &n))
      set_register(buf, n);
  }
  else {
    // ISO C++ does not permit VLAs on the stack.
    char *buf = new char[p - s + 1]; // C++03: new char[p - s + 1]();
    (void) memset(buf, 0, ((p - s + 1) * sizeof(char)));
    (void) memcpy(buf, s, p - s);
    buf[p - s] = 0;
    units n;
    if (evaluate_expression(p + 1, &n))
      set_register(buf, n);
    delete[] buf;
  }
}

static void set_string(const char *name, const char *value)
{
  macro *m = new macro;
  for (const char *p = value; *p != 0 /* nullptr */; p++)
    if (!is_invalid_input_char(static_cast<unsigned char>(*p)))
      m->append(*p);
  request_dictionary.define(name, m);
}

static void do_string_assignment(const char *s)
{
  const char *p = strchr(s, '=');
  if (!p) {
    char buf[2];
    buf[0] = s[0];
    buf[1] = 0;
    set_string(buf, s + 1);
  }
  else {
    // ISO C++ does not permit VLAs on the stack.
    char *buf = new char[p - s + 1]; // C++03: new char[p - s + 1]();
    (void) memset(buf, 0, ((p - s + 1) * sizeof(char)));
    (void) memcpy(buf, s, p - s);
    buf[p - s] = 0;
    set_string(buf, p + 1);
    delete[] buf;
  }
}

struct string_list {
  const char *s;
  string_list *next;
  string_list(const char *ss) : s(ss), next(0) {}
};

#if 0
static void prepend_string(const char *s, string_list **p)
{
  string_list *l = new string_list(s);
  l->next = *p;
  *p = l;
}
#endif

static void add_string(const char *s, string_list **p)
{
  while (*p)
    p = &((*p)->next);
  *p = new string_list(s);
}

void usage(FILE *stream, const char *prog)
{
  fprintf(stream,
"usage: %s [-abcCEiRSUz] [-d ctext] [-d string=text] [-f font-family]"
" [-F font-directory] [-I inclusion-directory] [-m macro-package]"
" [-M macro-directory] [-n page-number] [-o page-list]"
" [-r cnumeric-expression] [-r register=numeric-expression]"
" [-T output-device] [-w warning-category] [-W warning-category]"
" [file ...]\n"
"usage: %s {-v | --version}\n"
"usage: %s --help\n",
	  prog, prog, prog);
  if (stdout == stream)
    fputs(
"\n"
"GNU troff transforms groff(7) language input into the device-\n"
"independent page description language detailed in groff_out(5); it\n"
"is the heart of the GNU roff document formatting system.  Many\n"
"people prefer to use the groff(1) command, a front end that also\n"
"runs preprocessors and output drivers in the appropriate order and\n"
"with appropriate options.  See the troff(1) manual page.\n",
	  stream);
}

int main(int argc, char **argv)
{
  program_name = argv[0];
  static char stderr_buf[BUFSIZ];
  setbuf(stderr, stderr_buf);
  int c;
  string_list *macros = 0 /* nullptr */;
  string_list *register_assignments = 0 /* nullptr */;
  string_list *string_assignments = 0 /* nullptr */;
  bool want_stdin_read_last = false;
  bool have_explicit_device_argument = false;
  bool have_explicit_default_family = false;
  bool have_explicit_first_page_number = false;
  bool want_startup_macro_files_skipped = false;
  bool is_safer_mode_locked = false; // made true if `-S` explicit
  int next_page_number = 0;	// pacify compiler
  hresolution = vresolution = 1;
  if (getenv("GROFF_DUMP_NODES") != 0 /* nullptr */)
    want_nodes_dumped = true;
  // restore $PATH if called from groff
  char* groff_path = getenv("GROFF_PATH__");
  if (groff_path != 0 /* nullptr */) {
    string e = "PATH";
    e += '=';
    if (*groff_path)
      e += groff_path;
    e += '\0';
    if (putenv(strsave(e.contents())) != 0)
      fatal("cannot update process environment: %1", strerror(errno));
  }
  setlocale(LC_CTYPE, "");
  static const struct option long_options[] = {
    { "help", no_argument, 0 /* nullptr */, CHAR_MAX + 1 },
    { "version", no_argument, 0 /* nullptr */, 'v' },
    { 0, 0, 0, 0 }
  };
#if defined(DEBUGGING)
#define DEBUG_OPTION "D"
#else
#define DEBUG_OPTION ""
#endif
  while ((c = getopt_long(argc, argv,
			  ":abcCd:Ef:F:iI:m:M:n:o:qr:Rs:StT:Uvw:W:z"
			  DEBUG_OPTION,
			  long_options, 0 /* nullptr */))
	 != EOF)
    switch (c) {
    case 'v':
      {
	printf("GNU troff (groff) version %s\n", Version_string);
	exit(EXIT_SUCCESS);
	break;
      }
    case 'I':
      // Search path for .psbb files
      // and most other non-system input files.
      include_search_path.command_line_dir(optarg);
      break;
    case 'T':
      device = optarg;
      have_explicit_device_argument = true;
      is_writing_html = (strcmp(device, "html") == 0);
      break;
    case 'C':
      want_att_compat = true;
      // fall through
    case 'c':
      permit_color_output = false;
      break;
    case 'M':
      macro_path.command_line_dir(optarg);
      safer_macro_path.command_line_dir(optarg);
      config_macro_path.command_line_dir(optarg);
      break;
    case 'F':
      font::command_line_font_dir(optarg);
      break;
    case 'm':
      add_string(optarg, &macros);
      break;
    case 'E':
      want_errors_inhibited = true;
      break;
    case 'R':
      want_startup_macro_files_skipped = true;
      break;
    case 'w':
      enable_warning(optarg);
      break;
    case 'W':
      disable_warning(optarg);
      break;
    case 'i':
      want_stdin_read_last = true;
      break;
    case 'b':
      want_backtraces = true;
      break;
    case 'a':
      want_abstract_output = true;
      break;
    case 'z':
      want_output_suppressed = true;
      break;
    case 'n':
      if (sscanf(optarg, "%d", &next_page_number) == 1)
	have_explicit_first_page_number = true;
      else
	error("bad page number");
      break;
    case 'o':
      parse_output_page_list(optarg);
      break;
    case 'd':
      if (*optarg == '\0')
	error("'-d' requires non-empty argument");
      else if (*optarg == '=')
	error("malformed argument to '-d'; string name cannot be empty"
	      " or contain an equals sign");
      else
	add_string(optarg, &string_assignments);
      break;
    case 'r':
      if (*optarg == '\0')
	error("'-r' requires non-empty argument");
      else if (*optarg == '=')
	error("malformed argument to '-r'; register name cannot be"
	      " empty or contain an equals sign");
      else
	add_string(optarg, &register_assignments);
      break;
    case 'f':
      default_family = symbol(optarg);
      have_explicit_default_family = true;
      break;
    case 'q':
    case 's':
    case 't':
      // silently ignore these
      break;
    case 'S':
      want_unsafe_requests = false;
      is_safer_mode_locked = true;
      break;
    case 'U':
      if (is_safer_mode_locked)
	error("ignoring '-U' option; '-S' already specified");
      else
	want_unsafe_requests = true;
      break;
#if defined(DEBUGGING)
    case 'D':
      want_html_debugging = true;
      break;
#endif
    case CHAR_MAX + 1: // --help
      usage(stdout, argv[0]);
      exit(EXIT_SUCCESS);
      break;
    case '?':
      if (optopt != 0)
	error("unrecognized command-line option '%1'", char(optopt));
      else
	error("unrecognized command-line option '%1'",
	      argv[(optind - 1)]);
      usage(stderr, argv[0]);
      exit(2);
      break;		// never reached
    case ':':
      error("command-line option '%1' requires an argument",
	    char(optopt));
      usage(stderr, argv[0]);
      exit(2);
      break;		// never reached
    default:
      assert(0 == "unhandled case of command-line option");
    }
  if (want_unsafe_requests)
    mac_path = &macro_path;
  set_string(".T", device);
  // TODO: Kill this off in groff 1.24.0 release + 2 years.  See env.cpp.
  if ((strcmp("pdf", device) == 0) || strcmp("ps", device) == 0)
    is_device_ps_or_pdf = true;
  init_charset_table();
  init_hpf_code_table();
  if (0 /* nullptr */ == font::load_desc())
    fatal("cannot load 'DESC' description file for device '%1'",
	  device);
  units_per_inch = font::res;
  hresolution = font::hor;
  vresolution = font::vert;
  sizescale = font::sizescale;
  device_has_tcommand = font::has_tcommand;
  warn_scale = (double) units_per_inch;
  warn_scaling_unit = 'i';
  if (!have_explicit_default_family && (font::family != 0 /* nullptr */)
      && *font::family != '\0')
    default_family = symbol(font::family);
  font_size::init_size_list(font::sizes);
  int i;
  int j = 1;
  if (font::style_table)
    for (i = 0; font::style_table[i] != 0 /* nullptr */; i++)
      // Mounting a style can't actually fail due to a bad style name;
      // that's not determined until the full font name is resolved.
      // The DESC file also can't provoke a problem by requesting over a
      // thousand slots in the style table.
      if (!mount_style(j++, symbol(font::style_table[i])))
	warning(WARN_FONT, "cannot mount style '%1' directed by 'DESC'"
		" file for device '%2'", font::style_table[i], device);
  for (i = 0; font::font_name_table[i] != 0 /* nullptr */; i++, j++)
    // In the DESC file, a font name of 0 (zero) means "leave this
    // position empty".
    if (strcmp(font::font_name_table[i], "0") != 0)
      if (!mount_font(j, symbol(font::font_name_table[i])))
	warning(WARN_FONT, "cannot mount font '%1' directed by 'DESC'"
		" file for device '%2'", font::font_name_table[i],
		device);
  curdiv = topdiv = new top_level_diversion;
  if (have_explicit_first_page_number)
    topdiv->set_next_page_number(next_page_number);
  init_input_requests();
  init_env_requests();
  init_div_requests();
#ifdef COLUMN
  init_column_requests();
#endif /* COLUMN */
  init_node_requests();
  register_dictionary.define(".T",
      new readonly_boolean_register(&have_explicit_device_argument));
  init_registers();
  init_reg_requests();
  init_hyphenation_pattern_requests();
  init_environments();
  while (string_assignments != 0 /* nullptr */) {
    do_string_assignment(string_assignments->s);
    string_list *tem = string_assignments;
    string_assignments = string_assignments->next;
    delete tem;
  }
  while (register_assignments != 0 /* nullptr */) {
    do_register_assignment(register_assignments->s);
    string_list *tem = register_assignments;
    register_assignments = register_assignments->next;
    delete tem;
  }
  if (!want_startup_macro_files_skipped)
    process_startup_file(INITIAL_STARTUP_FILE);
  while (macros != 0 /* nullptr */) {
    process_macro_package_argument(macros->s);
    string_list *tem = macros;
    macros = macros->next;
    delete tem;
  }
  if (!want_startup_macro_files_skipped)
    process_startup_file(FINAL_STARTUP_FILE);
  for (i = optind; i < argc; i++)
    process_input_file(argv[i]);
  if (optind >= argc || want_stdin_read_last)
    process_input_file("-");
  exit_troff();
  return 0;			// not reached
}

void set_warning_mask_request()
{
  int n;
  if (has_arg() && read_integer(&n)) {
    if (n & ~WARN_MAX) {
      warning(WARN_RANGE, "warning mask must be in range 0..%1, got %2",
	      WARN_MAX, n);
      n &= WARN_MAX;
    }
    warning_mask = n;
  }
  else
    warning_mask = WARN_MAX;
  skip_line();
}

static void init_registers()
{
  struct tm *t = current_time();
  set_register("seconds", int(t->tm_sec));
  set_register("minutes", int(t->tm_min));
  set_register("hours", int(t->tm_hour));
  set_register("dw", int(t->tm_wday + 1));
  set_register("dy", int(t->tm_mday));
  set_register("mo", int(t->tm_mon + 1));
  set_register("year", int(1900 + t->tm_year));
  set_register("yr", int(t->tm_year));
  set_register("$$", getpid());
  register_dictionary.define(".A",
      new readonly_text_register(want_abstract_output));
}

/*
 *  registers associated with \O
 */

static int output_reg_minx_contents = -1;
static int output_reg_miny_contents = -1;
static int output_reg_maxx_contents = -1;
static int output_reg_maxy_contents = -1;

void check_output_limits(int x, int y)
{
  if ((output_reg_minx_contents == -1) || (x < output_reg_minx_contents))
    output_reg_minx_contents = x;
  if (x > output_reg_maxx_contents)
    output_reg_maxx_contents = x;
  if ((output_reg_miny_contents == -1) || (y < output_reg_miny_contents))
    output_reg_miny_contents = y;
  if (y > output_reg_maxy_contents)
    output_reg_maxy_contents = y;
}

void reset_output_registers()
{
  output_reg_minx_contents = -1;
  output_reg_miny_contents = -1;
  output_reg_maxx_contents = -1;
  output_reg_maxy_contents = -1;
}

void init_input_requests()
{
  init_request("ab", abort_request);
  init_request("als", alias_macro);
  init_request("am", append_macro);
  init_request("am1", append_nocomp_macro);
  init_request("ami", append_indirect_macro);
  init_request("ami1", append_indirect_nocomp_macro);
  init_request("as", append_string);
  init_request("as1", append_nocomp_string);
  init_request("asciify", asciify_request);
  init_request("backtrace", backtrace_request);
  init_request("blm", blank_line_macro);
  init_request("break", while_break_request);
  init_request("cc", assign_control_character_request);
  init_request("c2", assign_no_break_control_character_request);
  init_request("cf", unsafe_transparent_throughput_file_request);
  init_request("cflags", set_character_flags_request);
  init_request("char", define_character_request);
  init_request("chop", chop_macro);
  init_request("class", define_class_request);
  init_request("close", close_request);
  init_request("color", activate_color);
  init_request("composite", map_composite_character);
  init_request("continue", while_continue_request);
  init_request("cp", compatible);
  init_request("de", define_macro);
  init_request("de1", define_nocomp_macro);
  init_request("defcolor", define_color);
  init_request("dei", define_indirect_macro);
  init_request("dei1", define_indirect_nocomp_macro);
  init_request("device", device_request);
  init_request("devicem", device_macro_request);
  init_request("do", do_request);
  init_request("ds", define_string);
  init_request("ds1", define_nocomp_string);
  init_request("ec", assign_escape_character_request);
  init_request("ecr", restore_escape_char_request);
  init_request("ecs", save_escape_char_request);
  init_request("el", else_request);
  init_request("em", eoi_macro);
  init_request("eo", escape_off_request);
  init_request("ex", exit_request);
  init_request("fchar", define_fallback_character_request);
#ifdef WIDOW_CONTROL
  init_request("fpl", flush_pending_lines);
#endif /* WIDOW_CONTROL */
  init_request("hcode", set_hyphenation_codes);
  init_request("hpfcode", hyphenation_patterns_file_code);
  init_request("ie", if_else_request);
  init_request("if", if_request);
  init_request("ig", ignore);
  init_request("length", length_request);
  init_request("lf", line_file);
  init_request("lsm", leading_spaces_macro);
  init_request("mso", macro_source_request);
  init_request("msoquiet", macro_source_quietly_request);
  init_request("nop", nop_request);
  init_request("nroff", nroff_request);
  init_request("nx", next_file);
  init_request("open", open_request);
  init_request("opena", opena_request);
  init_request("output", output_request);
  init_request("pc", page_character_request);
  init_request("pchar", print_character_request);
  init_request("pcolor", print_color_request);
  init_request("pcomposite", print_composite_character_request);
  init_request("pi", pipe_output);
  init_request("pm", print_macro_request);
  init_request("psbb", ps_bbox_request);
  init_request("pso", pipe_source_request);
  init_request("pstream", print_stream_request);
  init_request("rchar", remove_character);
  init_request("rd", read_request);
  init_request("return", return_macro_request);
  init_request("rm", remove_macro);
  init_request("rn", rename_macro);
  init_request("schar", define_special_character_request);
  init_request("shift", shift);
  init_request("so", source_request);
  init_request("soquiet", source_quietly_request);
  init_request("spreadwarn", spreadwarn_request);
  init_request("stringdown", stringdown_request);
  init_request("stringup", stringup_request);
  init_request("substring", substring_request);
  init_request("sy", system_request);
  init_request("tag", tag);
  init_request("taga", taga);
  init_request("tm", terminal_message_request);
  init_request("tm1", terminal_message1_request);
  init_request("tmc", terminal_message_continuation_request);
  init_request("tr", translate);
  init_request("trf", transparent_throughput_file_request);
  init_request("trin", translate_input);
  init_request("trnt", translate_no_transparent);
  init_request("troff", troff_request);
  init_request("unformat", unformat_macro);
#ifdef COLUMN
  init_request("vj", vjustify);
#endif /* COLUMN */
  init_request("warn", set_warning_mask_request);
  init_request("warnscale", warnscale_request);
  init_request("while", while_request);
  init_request("write", stream_write_request);
  init_request("writec", stream_write_continuation_request);
  init_request("writem", stream_write_macro_request);
  register_dictionary.define(".$", new nargs_reg);
  register_dictionary.define(".br", new break_flag_reg);
  register_dictionary.define(".C", new readonly_boolean_register(&want_att_compat));
  register_dictionary.define(".cp", new enclosing_want_att_compat_reg);
  register_dictionary.define(".O", new variable_reg(&suppression_level));
  register_dictionary.define(".c", new lineno_reg);
  register_dictionary.define(".color", new readonly_boolean_register(&want_color_output));
  register_dictionary.define(".F", new filename_reg);
  register_dictionary.define(".g", new readonly_text_register(1));
  register_dictionary.define(".H", new readonly_register(&hresolution));
  register_dictionary.define(".R", new readonly_text_register(INT_MAX));
  register_dictionary.define(".U", new readonly_boolean_register(&want_unsafe_requests));
  register_dictionary.define(".V", new readonly_register(&vresolution));
  register_dictionary.define(".warn", new readonly_mask_register(&warning_mask));
  extern const char *major_version;
  register_dictionary.define(".x", new readonly_text_register(major_version));
  extern const char *revision;
  register_dictionary.define(".Y", new readonly_text_register(revision));
  extern const char *minor_version;
  register_dictionary.define(".y", new readonly_text_register(minor_version));
  register_dictionary.define("c.", new writable_lineno_reg);
  register_dictionary.define("llx", new variable_reg(&llx_reg_contents));
  register_dictionary.define("lly", new variable_reg(&lly_reg_contents));
  register_dictionary.define("lsn", new variable_reg(&leading_spaces_number));
  register_dictionary.define("lss", new variable_reg(&leading_spaces_space));
  register_dictionary.define("opmaxx",
			       new variable_reg(&output_reg_maxx_contents));
  register_dictionary.define("opmaxy",
			       new variable_reg(&output_reg_maxy_contents));
  register_dictionary.define("opminx",
			       new variable_reg(&output_reg_minx_contents));
  register_dictionary.define("opminy",
			       new variable_reg(&output_reg_miny_contents));
  register_dictionary.define("slimit",
			       new variable_reg(&input_stack::limit));
  register_dictionary.define("systat", new variable_reg(&system_status));
  register_dictionary.define("urx", new variable_reg(&urx_reg_contents));
  register_dictionary.define("ury", new variable_reg(&ury_reg_contents));
}

object_dictionary request_dictionary(501);

void init_request(const char *s, REQUEST_FUNCP f)
{
  request_dictionary.define(s, new request(f));
}

static request_or_macro *lookup_request(symbol nm)
{
  assert(!nm.is_null());
  request_or_macro *p
    = static_cast<request_or_macro *>(request_dictionary.lookup(nm));
  if (0 /* nullptr */ == p) {
    warning(WARN_MAC, "name '%1' not defined", nm.contents());
    p = new macro;
    request_dictionary.define(nm, p);
  }
  return p;
}

// XXX: move to node.cpp, its only call site?
node *charinfo_to_node_list(charinfo *ci, const environment *envp)
{
  // Don't interpret character definitions in AT&T compatibility mode.
  int old_want_att_compat = want_att_compat;
  want_att_compat = false;
  unsigned char previous_escape_char = escape_char;
  escape_char = '\\';
  macro *mac = ci->set_macro(0 /* nullptr */);
  assert(mac != 0 /* nullptr */);
  environment *oldenv = curenv;
  environment env(envp);
  curenv = &env;
  curenv->set_composite();
  token old_tok = tok;
  input_stack::add_boundary();
  string_iterator *si =
    new string_iterator(*mac, "special character", ci->nm);
  input_stack::push(si);
  // Don't use process_input_stack, because we don't want to recognize
  // requests.
  for (;;) {
    tok.next();
    if (tok.is_eof())
      break;
    if (tok.is_newline()) {
      error("a newline is not allowed in a composite character"
	    " escape sequence argument");
      while (!tok.is_eof())
	tok.next();
      break;
    }
    else
      tok.process();
  }
  node *n = curenv->extract_output_line();
  input_stack::remove_boundary();
  ci->set_macro(mac);
  tok = old_tok;
  curenv = oldenv;
  want_att_compat = old_want_att_compat;
  escape_char = previous_escape_char;
  have_formattable_input = false;
  return n;
}

static node *read_drawing_command() // \D
{
  token start_token;
  start_token.next();
  if (!want_att_compat && !start_token.is_usable_as_delimiter())
    warning(WARN_DELIM, "using %1 as an escape sequence delimiter"
			" is deprecated", tok.description());
  else if (want_att_compat
           && !start_token.is_usable_as_delimiter(false,
		  DELIMITER_ATT_STRING_EXPRESSION)) {
    warning(WARN_DELIM, "drawing command escape sequence"
	    " does not accept %1 as a delimiter",
	    start_token.description());
    return 0 /* nullptr */;
  }
  // TODO: groff 1.24.0 release + 2 years?
#if 0
  if (!start_token.is_usable_as_delimiter(true /* report error */))
    return 0 /* nullptr */;
#endif
  else {
    tok.next();
    if (tok == start_token)
      warning(WARN_MISSING, "missing arguments to drawing escape"
	      " sequence");
    else {
      int type = tok.ch(); // safely compares to char literals
      // TODO: grochar
      if (type == 'F') {
	read_drawing_command_color_arguments(start_token);
	return 0 /* nullptr */;
      }
      tok.next();
      int maxpoints = 10;
      hvpair *point = new hvpair[maxpoints];
      int npoints = 0;
      bool no_last_v = false;
      bool had_error = false;
      int i;
      for (i = 0; tok != start_token; i++) {
	if (i == maxpoints) {
	  hvpair *oldpoint = point;
	  point = new hvpair[maxpoints * 2];
	  for (int j = 0; j < maxpoints; j++)
	    point[j] = oldpoint[j];
	  maxpoints *= 2;
	  delete[] oldpoint;
	}
	if (tok.is_newline() || tok.is_eof()) {
	  // token::description() writes to static, class-wide storage,
	  // so we must allocate a copy of it before issuing the next
	  // diagnostic.
	  char *delimdesc = strdup(start_token.description());
	  warning(WARN_DELIM, "missing closing delimiter in drawing"
		  " escape sequence; expected %1, got %2", delimdesc,
		  tok.description());
	  free(delimdesc);
	  had_error = true;
	  break;
	}
	if (!read_hunits(&point[i].h,
			type == 'f' || type == 't' ? 'u' : 'm')) {
	  had_error = true;
	  break;
	}
	++npoints;
	tok.skip_spaces();
	point[i].v = V0;
	if (tok == start_token) {
	  no_last_v = true;
	  break;
	}
	if (!read_vunits(&point[i].v, 'v')) {
	  had_error = false;
	  break;
	}
	tok.skip_spaces();
      }
      while (tok != start_token && !tok.is_newline() && !tok.is_eof())
	tok.next();
      if (!had_error) {
	switch (type) {
	case 'l':
	  if (npoints != 1 || no_last_v) {
	    error("two arguments needed for line");
	    npoints = 1;
	  }
	  break;
	case 'c':
	  if (npoints != 1 || !no_last_v) {
	    error("one argument needed for circle");
	    npoints = 1;
	    point[0].v = V0;
	  }
	  break;
	case 'e':
	  if (npoints != 1 || no_last_v) {
	    error("two arguments needed for ellipse");
	    npoints = 1;
	  }
	  break;
	case 'a':
	  if (npoints != 2 || no_last_v) {
	    error("four arguments needed for arc");
	    npoints = 2;
	  }
	  break;
	case '~':
	  if (no_last_v)
	    error("even number of arguments needed for spline");
	  break;
	case 'f':
	  if (npoints != 1 || !no_last_v) {
	    error("one argument needed for gray shade");
	    npoints = 1;
	    point[0].v = V0;
	  }
	default:
	  // silently pass it through
	  break;
	}
	draw_node *dn = new draw_node(type, point, npoints,
				      curenv->get_font_size(),
				      curenv->get_stroke_color(),
				      curenv->get_fill_color());
	delete[] point;
	return dn;
      }
      else {
	delete[] point;
      }
    }
  }
  return 0 /* nullptr */;
}

static void read_drawing_command_color_arguments(token &start)
{
  tok.next();
  if (tok == start) {
    error("missing color scheme");
    return;
  }
  // safely compares to char literals; TODO: grochar
  int scheme = tok.ch();
  tok.next();
  color *col = 0 /* nullptr */;
  // TODO: grochar
  unsigned char end = start.ch();
  switch (scheme) {
  case 'c':
    col = read_cmy(end);
    break;
  case 'd':
    col = &default_color;
    break;
  case 'g':
    col = read_gray(end);
    break;
  case 'k':
    col = read_cmyk(end);
    break;
  case 'r':
    col = read_rgb(end);
    break;
  }
  if (col != 0 /* nullptr */)
    curenv->set_fill_color(col);
  while (tok != start) {
    if (!has_arg()) {
      // token::description() writes to static, class-wide storage, so
      // we must allocate a copy of it before issuing the next
      // diagnostic.
      char *delimdesc = strdup(start.description());
      warning(WARN_DELIM, "missing closing delimiter in color space"
	      " drawing escape sequence; expected %1, got %2",
	      delimdesc, tok.description());
      free(delimdesc);
      input_stack::push(make_temp_iterator("\n"));
      break;
    }
    tok.next();
  }
  have_formattable_input = true;
}

static struct warning_category {
  const char *name;
  unsigned int mask;
} warning_table[] = {
  { "char", WARN_CHAR },
  { "range", WARN_RANGE },
  { "break", WARN_BREAK },
  { "delim", WARN_DELIM },
  { "scale", WARN_SCALE },
  { "syntax", WARN_SYNTAX },
  { "tab", WARN_TAB },
  { "missing", WARN_MISSING },
  { "input", WARN_INPUT },
  { "escape", WARN_ESCAPE },
  { "space", WARN_SPACE },
  { "font", WARN_FONT },
  { "di", WARN_DI },
  { "mac", WARN_MAC },
  { "reg", WARN_REG },
  { "ig", WARN_IG },
  { "color", WARN_COLOR },
  { "file", WARN_FILE },
  { "all", WARN_MAX & ~(WARN_DI | WARN_MAC | WARN_REG) },
  { "w", WARN_MAX },
  { "default", DEFAULT_WARNING_MASK },
};

static unsigned int lookup_warning(const char *name)
{
  for (unsigned int i = 0U; i < countof(warning_table); i++)
    if (strcmp(name, warning_table[i].name) == 0)
      return warning_table[i].mask;
  return 0U;
}

static void enable_warning(const char *name)
{
  unsigned int mask = lookup_warning(name);
  if (mask != 0U)
    warning_mask |= mask;
  else
    error("unrecognized warning category '%1'", name);
}

static void disable_warning(const char *name)
{
  unsigned int mask = lookup_warning(name);
  if (mask != 0U)
    warning_mask &= ~mask;
  else
    error("unrecognized warning category '%1'", name);
}

static void copy_mode_error(const char *format,
			    const errarg &arg1,
			    const errarg &arg2,
			    const errarg &arg3)
{
  if (want_input_ignored) {
    static const char prefix[] = "(in ignored input) ";
    // ISO C++ does not permit VLAs on the stack.
    // C++03: new char[sizeof prefix + strlen(format)]();
    char *s = new char[sizeof prefix + strlen(format)];
    (void) memset(s, 0, (sizeof prefix + (strlen(format)
					  * sizeof(char))));
    strcpy(s, prefix);
    strcat(s, format);
    warning(WARN_IG, s, arg1, arg2, arg3);
    delete[] s;
  }
  else
    error(format, arg1, arg2, arg3);
}

enum error_type { DEBUG, WARNING, OUTPUT_WARNING, ERROR, FATAL };

static void do_error(error_type type,
		     const char *format,
		     const errarg &arg1,
		     const errarg &arg2,
		     const errarg &arg3)
{
  const char *filename;
  int lineno;
  if (want_errors_inhibited && (type < FATAL))
    return;
  if (want_backtraces)
    input_stack::backtrace();
  if (!get_file_line(&filename, &lineno))
    filename = 0 /* nullptr */;
  if (filename != 0 /* nullptr */) {
    if (program_name != 0 /* nullptr */)
      errprint("%1:", program_name);
    errprint("%1:%2: ", filename, lineno);
  }
  else if (program_name != 0 /* nullptr */)
    fprintf(stderr, "%s: ", program_name);
  switch (type) {
  case FATAL:
    fputs("fatal error: ", stderr);
    break;
  case ERROR:
    fputs("error: ", stderr);
    break;
  case WARNING:
    fputs("warning: ", stderr);
    break;
  case DEBUG:
    fputs("debug: ", stderr);
    break;
  case OUTPUT_WARNING:
    if (in_nroff_mode) {
      int fromtop = (topdiv->get_vertical_position().to_units()
		     / vresolution) + 1;
      fprintf(stderr, "warning [page %d, line %d",
	      topdiv->get_page_number(), fromtop);
      if (topdiv != curdiv) {
	int fromdivtop = (curdiv->get_vertical_position().to_units()
			  / vresolution) + 1;
	fprintf(stderr, ", diversion '%s', line %d",
		curdiv->get_diversion_name(), fromdivtop);
      }
      fprintf(stderr, "]: ");
    }
    else {
      double fromtop = topdiv->get_vertical_position().to_units()
		       / warn_scale;
      fprintf(stderr, "warning [page %d, %.1f%c",
	      topdiv->get_page_number(), fromtop, warn_scaling_unit);
      if (topdiv != curdiv) {
	double fromtop1 = curdiv->get_vertical_position().to_units()
			  / warn_scale;
	fprintf(stderr, " (diversion '%s', %.1f%c)",
		curdiv->get_diversion_name(), fromtop1,
		warn_scaling_unit);
      }
      fprintf(stderr, "]: ");
    }
    break;
  }
  errprint(format, arg1, arg2, arg3);
  fputc('\n', stderr);
  fflush(stderr);
  if (type == FATAL)
    write_any_trailer_and_exit(EXIT_FAILURE);
}

// This function should have no callers in production builds.
void debug(const char *format,
	   const errarg &arg1,
	   const errarg &arg2,
	   const errarg &arg3)
{
  do_error(DEBUG, format, arg1, arg2, arg3);
}

int warning(warning_type t,
	    const char *format,
	    const errarg &arg1,
	    const errarg &arg2,
	    const errarg &arg3)
{
  if ((t & warning_mask) != 0U) {
    do_error(WARNING, format, arg1, arg2, arg3);
    return 1;
  }
  else
    return 0;
}

int output_warning(warning_type t,
		   const char *format,
		   const errarg &arg1,
		   const errarg &arg2,
		   const errarg &arg3)
{
  if ((t & warning_mask) != 0U) {
    do_error(OUTPUT_WARNING, format, arg1, arg2, arg3);
    return 1;
  }
  else
    return 0;
}

void error(const char *format,
	   const errarg &arg1,
	   const errarg &arg2,
	   const errarg &arg3)
{
  do_error(ERROR, format, arg1, arg2, arg3);
}

void fatal(const char *format,
	   const errarg &arg1,
	   const errarg &arg2,
	   const errarg &arg3)
{
  do_error(FATAL, format, arg1, arg2, arg3);
}

void fatal_with_file_and_line(const char *filename, int lineno,
			      const char *format,
			      const errarg &arg1,
			      const errarg &arg2,
			      const errarg &arg3)
{
  if (program_name != 0 /* nullptr */)
    fprintf(stderr, "%s:", program_name);
  fprintf(stderr, "%s:", filename);
  if (lineno > 0)
    fprintf(stderr, "%d:", lineno);
  fputs(" fatal error: ", stderr);
  errprint(format, arg1, arg2, arg3);
  fputc('\n', stderr);
  fflush(stderr);
  write_any_trailer_and_exit(EXIT_FAILURE);
}

void error_with_file_and_line(const char *filename, int lineno,
			      const char *format,
			      const errarg &arg1,
			      const errarg &arg2,
			      const errarg &arg3)
{
  if (program_name != 0 /* nullptr */)
    fprintf(stderr, "%s:", program_name);
  fprintf(stderr, "%s:", filename);
  if (lineno > 0)
    fprintf(stderr, "%d:", lineno);
  fputs(" error: ", stderr);
  errprint(format, arg1, arg2, arg3);
  fputc('\n', stderr);
  fflush(stderr);
}

// This function should have no callers in production builds.
void debug_with_file_and_line(const char *filename,
			      int lineno,
			      const char *format,
			      const errarg &arg1,
			      const errarg &arg2,
			      const errarg &arg3)
{
  if (program_name != 0 /* nullptr */)
    fprintf(stderr, "%s:", program_name);
  fprintf(stderr, "%s:", filename);
  if (lineno > 0)
    fprintf(stderr, "%d:", lineno);
  fputs(" debug: ", stderr);
  errprint(format, arg1, arg2, arg3);
  fputc('\n', stderr);
  fflush(stderr);
}

dictionary charinfo_dictionary(501);

charinfo *lookup_charinfo(symbol nm, bool suppress_creation)
{
  void *p = charinfo_dictionary.lookup(nm);
  if (p != 0 /* nullptr */)
    return static_cast<charinfo *>(p);
  if (suppress_creation)
    return static_cast<charinfo *>(0 /* nullptr */);
  else {
    charinfo *cp = new charinfo(nm);
    (void) charinfo_dictionary.lookup(nm, cp);
    return cp;
  }
}

int charinfo::next_index = 0;

charinfo::charinfo(symbol s)
: translation(0 /* nullptr */), mac(0 /* nullptr */),
  special_translation(TRANSLATE_NONE), hyphenation_code(0U),
  flags(0U), ascii_code(0U), asciify_code(0U),
  is_not_found(false), is_transparently_translatable(true),
  translatable_as_input(false), mode(CHAR_NORMAL), nm(s)
{
  index = next_index++;
  number = -1;
  get_flags();
}

int charinfo::get_unicode_mapping()
{
  if (ascii_code != 0U)
    return ascii_code;
  return glyph_to_unicode(this);
}

void charinfo::set_hyphenation_code(unsigned char c)
{
  hyphenation_code = c;
}

void charinfo::set_translation(charinfo *ci, bool transparently,
			       bool as_input)
{
  translation = ci;
  if ((ci != 0 /* nullptr */) && as_input) {
    if (hyphenation_code != 0U)
      ci->set_hyphenation_code(hyphenation_code);
    if (asciify_code != 0U)
      ci->set_asciify_code(asciify_code);
    else if (ascii_code != 0U)
      ci->set_asciify_code(ascii_code);
    ci->make_translatable_as_input();
  }
  special_translation = TRANSLATE_NONE;
  is_transparently_translatable = transparently;
}

// Recompute flags for all entries in the charinfo dictionary.
void get_flags()
{
  dictionary_iterator iter(charinfo_dictionary);
  charinfo *ci;
  symbol s;
  // We must use the nuclear `reinterpret_cast` operator because GNU
  // troff's dictionary types use a pre-STL approach to containers.
  while (iter.get(&s, reinterpret_cast<void **>(&ci))) {
    assert(!s.is_null());
    ci->get_flags();
  }
  using_character_classes = false;
}

// Get the union of all flags affecting this charinfo.
void charinfo::get_flags()
{
  dictionary_iterator iter(char_class_dictionary);
  charinfo *ci;
  symbol s;
  // We must use the nuclear `reinterpret_cast` operator because GNU
  // troff's dictionary types use a pre-STL approach to containers.
  while (iter.get(&s, reinterpret_cast<void **>(&ci))) {
    assert(!s.is_null());
    if (ci->contains(get_unicode_mapping())) {
#if defined(DEBUGGING)
      if (want_html_debugging)
	fprintf(stderr, "charinfo::get_flags %p %s %d\n",
			static_cast<void *>(ci), ci->nm.contents(),
			ci->flags);
#endif
      flags |= ci->flags;
    }
  }
}

void charinfo::set_special_translation(int cc, bool transparently)
{
  special_translation = cc;
  translation = 0 /* nullptr */;
  is_transparently_translatable = transparently;
}

void charinfo::set_ascii_code(unsigned char c)
{
  ascii_code = c;
}

void charinfo::set_asciify_code(unsigned char c)
{
  asciify_code = c;
}

// Replace character definition with macro `m`, returning previous
// macro if any (if none, return a null pointer).
macro *charinfo::set_macro(macro *m)
{
  macro *tem = mac;
  mac = m;
  return tem;
}

// Replace character definition with macro `m` and update its character
// mode to `cm`, returning previous macro if any (if none, return a null
// pointer).
macro *charinfo::set_macro(macro *m, char_mode cm)
{
  macro *tem = mac;
  mac = m;
  mode = cm;
  return tem;
}

void charinfo::set_number(int n)
{
  assert(n >= 0);
  number = n;
}

int charinfo::get_number()
{
  assert(number >= 0);
  return number;
}

bool charinfo::contains(int c, bool already_called)
{
  if (already_called) {
    warning(WARN_SYNTAX, "nested class detected while processing"
	    " character code %1", c);
    return false;
  }
  std::vector<std::pair<int, int> >::const_iterator ranges_iter;
  ranges_iter = ranges.begin();
  while (ranges_iter != ranges.end()) {
    if (c >= ranges_iter->first && c <= ranges_iter->second) {
#if defined(DEBUGGING)
      if (want_html_debugging)
	fprintf(stderr, "charinfo::contains(%d)\n", c);
#endif
      return true;
    }
    ++ranges_iter;
  }

  // Nested classes don't work.  See Savannah #67770.
#if 0
  std::vector<charinfo *>::const_iterator nested_iter;
  nested_iter = nested_classes.begin();
  while (nested_iter != nested_classes.end()) {
    if ((*nested_iter)->contains(c, true))
      return true;
    ++nested_iter;
  }
#endif

  return false;
}

bool charinfo::contains(symbol s, bool already_called)
{
  if (already_called) {
    warning(WARN_SYNTAX, "nested class detected while processing symbol"
	    " %1", s.contents());
    return false;
  }
  const char *unicode = glyph_name_to_unicode(s.contents());
  if (unicode != 0 /* nullptr */ && strchr(unicode, '_') == 0) {
    char *ignore;
    int c = (int) strtol(unicode, &ignore, 16);
    return contains(c, true);
  }
  else
    return false;
}

bool charinfo::contains(charinfo *, bool)
{
  // Werner Lemberg marked this as "TODO" in 2010.
  assert(0 == "unimplemented member function");
  return false;
}

void charinfo::describe_flags()
{
  if (0U == flags)
    errprint("(none)\n");
  else {
    char none[] = { '\0' };
    char comma[] = { ',', ' ', '\0' };
    char *separator = none;
    errprint("(");
    if (flags & ENDS_SENTENCE) {
      errprint("%1ends sentence", separator);
      separator = comma;
    }
    if (flags & ALLOWS_BREAK_BEFORE) {
      errprint("%1allows break before", separator);
      separator = comma;
    }
    if (flags & ALLOWS_BREAK_AFTER) {
      errprint("%1allows break after", separator);
      separator = comma;
    }
    if (flags & OVERLAPS_HORIZONTALLY) {
      errprint("%1overlaps horizontally", separator);
      separator = comma;
    }
    if (flags & OVERLAPS_VERTICALLY) {
      errprint("%1overlaps vertically", separator);
      separator = comma;
    }
    if (flags & IS_TRANSPARENT_TO_END_OF_SENTENCE) {
      errprint("%1is transparent to end of sentence", separator);
      separator = comma;
    }
    if (flags & IGNORES_SURROUNDING_HYPHENATION_CODES) {
      errprint("%1ignores surrounding hyphenation codes", separator);
      separator = comma;
    }
    if (flags & PROHIBITS_BREAK_BEFORE) {
      errprint("%1prohibits break before", separator);
      separator = comma;
    }
    if (flags & PROHIBITS_BREAK_AFTER) {
      errprint("%1prohibits break after", separator);
      separator = comma;
    }
    if (flags & IS_INTERWORD_SPACE) {
      errprint("%1is interword space", separator);
      separator = comma;
    }
    errprint(")\n");
  }
}

void charinfo::dump_flags()
{
  errprint("  %1flags: %2 ", (is_class() ? "" : "inherent "), flags);
  describe_flags();
  if (!is_class()) {
    // Report influence of membership in character classes, if any.
    unsigned int saved_flags = flags;
    get_flags();
    if (flags != saved_flags) {
      errprint("  effective flags: %1 ", flags);
      describe_flags();
      flags = saved_flags;
    }
  }
}

void charinfo::dump()
{
  if (is_class()) {
    std::vector<std::pair<int, int> >::const_iterator ranges_iter;
    ranges_iter = ranges.begin();
    assert(mac != 0 /* nullptr */);
    errprint("  defined at: ");
    mac->dump();
    fflush(stderr);
    errprint("  contains code points: ");
    const size_t buflen = sizeof "U+10FFFF";
    int range_begin = 0;
    int range_end = 0;
    char beg_hexbuf[buflen];
    char end_hexbuf[buflen];
    (void) memset(beg_hexbuf, '\0', buflen);
    (void) memset(end_hexbuf, '\0', buflen);
    bool has_ranges = false;
    while (ranges_iter != ranges.end()) {
      has_ranges = true;
      range_begin = ranges_iter->first;
      range_end = ranges_iter->second;
      (void) snprintf(beg_hexbuf, buflen, "U+%.4X", range_begin);
      (void) snprintf(end_hexbuf, buflen, "U+%.4X", range_end);
      // TODO: comma-separate?  JSON list?
      if (range_begin == range_end)
	errprint("%1 ", beg_hexbuf);
      else
	errprint("%1-%2 ", beg_hexbuf, end_hexbuf);
      ++ranges_iter;
    }
    if (!has_ranges)
      errprint("(none)");
    errprint("\n");
#if 0
    // Nested classes don't work.  See Savannah #67770.
    errprint("  contains nested classes: ");
    std::vector<charinfo *>::const_iterator nested_iter;
    nested_iter = nested_classes.begin();
    bool has_nested_classes = false;
    while (nested_iter != nested_classes.end()) {
      has_nested_classes = true;
      // TODO: Here's where JSON would really pay off.
      (*nested_iter)->dump();
    }
    if (!has_nested_classes)
      errprint("(none)");
    errprint("\n");
#endif
    dump_flags();
  }
  else {
    if (translation != 0 /* nullptr */)
      errprint("  is translated\n");
    else
      errprint("  is not translated\n");
    if (mac != 0 /* nullptr */) {
      errprint("  has a macro: ");
      mac->json_dump();
      errprint("\n");
    }
    else
      errprint("  does not have a macro\n");
    errprint("  special translation: %1\n",
	     static_cast<int>(special_translation));
    errprint("  hyphenation code: %1\n",
	     static_cast<int>(hyphenation_code));
    dump_flags();
    errprint("  asciify code: %1\n", static_cast<int>(asciify_code));
    errprint("  ASCII code: %1\n", static_cast<int>(ascii_code));
    // Also see node.cpp::glyph_node::asciify().
    int mapping = get_unicode_mapping();
    if (mapping >= 0) {
      const size_t buflen = 6; // enough for five hex digits + '\0'
      char hexbuf[buflen];
      (void) memset(hexbuf, '\0', buflen);
      (void) snprintf(hexbuf, buflen, "%.4X", mapping);
      errprint("  Unicode mapping: U+%1\n", hexbuf);
    }
    else
      errprint("  Unicode mapping: none (%1)\n", mapping);
    errprint("  is%1 found\n", is_not_found ? " not" : "");
    errprint("  is%1 transparently translatable\n",
	     is_transparently_translatable ? "" : " not");
    errprint("  is%1 translatable as input\n",
	     translatable_as_input ? "" : " not");
    const char *modestr = character_mode_description(mode);
    if (strcmp(modestr, "") == 0)
      modestr =" normal";
    errprint("  mode:%1\n", modestr);
  }
  fflush(stderr);
}

symbol UNNAMED_SYMBOL("---");

// For indexed characters not between 0 and 255, we make a symbol out
// of the number and store them in this dictionary.

dictionary indexed_charinfo_dictionary(11);

static charinfo *get_charinfo_by_index(int n, bool suppress_creation)
{
  static charinfo *index_table[256];

  if (n >= 0 && n < 256) {
    charinfo *ci = index_table[n];
    if ((0 /*nullptr */ == ci) && !suppress_creation) {
      ci = new charinfo(UNNAMED_SYMBOL);
      ci->set_number(n);
      index_table[n] = ci;
    }
    return ci;
  }
  else {
    symbol ns(i_to_a(n));
    charinfo *ci =
      static_cast<charinfo *>(indexed_charinfo_dictionary.lookup(ns));
    if ((0 /*nullptr */ == ci) && !suppress_creation) {
      ci = new charinfo(UNNAMED_SYMBOL);
      ci->set_number(n);
      (void) indexed_charinfo_dictionary.lookup(ns, ci);
    }
    return ci;
  }
}

// This overrides the same function from libgroff; while reading font
// definition files it puts single-letter glyph names into
// 'charset_table' and converts glyph names of the form '\x' ('x' a
// single letter) into 'x'.  Consequently, symbol("x") refers to glyph
// name '\x', not 'x'.

glyph *name_to_glyph(const char *nm)
{
  charinfo *ci;
  if (nm[1] == 0)
    ci = charset_table[nm[0] & 0xff];
  else if (nm[0] == '\\' && nm[2] == 0)
    ci = lookup_charinfo(symbol(nm + 1));
  else
    ci = lookup_charinfo(symbol(nm));
  return ci->as_glyph();
}

glyph *number_to_glyph(int n)
{
  return get_charinfo_by_index(n)->as_glyph();
}

const char *glyph_to_name(glyph *g)
{
  // In both libgroff and troff, `charinfo` has `glyph` as a base class.
  // But in troff, `charinfo` stores much more information.
  charinfo *ci = reinterpret_cast<charinfo *>(g);
  return ((ci->nm != UNNAMED_SYMBOL) ? ci->nm.contents()
				     : 0 /* nullptr */);
}

// Local Variables:
// fill-column: 72
// mode: C++
// End:
// vim: set cindent noexpandtab shiftwidth=2 textwidth=72:

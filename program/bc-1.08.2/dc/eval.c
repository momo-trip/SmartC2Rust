/*
 * evaluate the dc language, from a FILE* or a string
 *
 * Copyright (C) 1994, 1997, 1998, 2000, 2003, 2005, 2006, 2008,
 * 2010, 2012-2017, 2022, 2024
 * Free Software Foundation, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/* This is the only module which knows about the dc input language */

#include "../config.h"

#include <stdio.h>
#include <stdlib.h> /* free */
#include <signal.h>
#include <string.h> /* memset */
#include <limits.h> /* UCHAR_MAX */
#ifdef HAVE_UNISTD_H
# include <unistd.h> /* isatty */
#endif
#include "dc.h"

typedef enum {
	DC_OKAY,	//no further intervention needed for this command
	DC_QUIT,	//quit out of program
	DC_EOS 		//end of current input stream
} dc_status;

/*a stack of input streams; each either a FILE* or a byte-string description:*/
typedef struct cmd_list_s {
	union {
		FILE *fp;		//used when cur == NULL
		dc_str base;	//container for string, used when cur != NULL
	} v; //named, to keep older (pre- iso9899:2011) compilers happy
	const char *cur;	//a cursor; if NULL, then use fp instead
	const char *first;	//invariant pointer to start of string
	const char *last;	//invariant pointer to one-past-end of string
	unsigned int tail_depth; //tail-recursion depth, for Q unwindings
	int pushback; //a single char of pushback, for dc_unget() calls
	struct cmd_list_s *backlink;
} cmd_list;

static cmd_list *cmd_stack = NULL;

static int dc_ibase=10;		/* input base, 2 <= dc_ibase <= DC_IBASE_MAX */
static int dc_obase=10;		/* output base, 2 <= dc_obase */
static int dc_scale=0;		/* scale (see user documentation) */

static unsigned int recursion_depth=0;

/* when stdin is a tty we enable some interactivity enhancements */
static int interactive_session = 0;

/* for handling SIGINT properly */
static volatile sig_atomic_t interrupt_seen=0;


/* step cursor past whitespace and comments */
static void
skip_to_non_whitespace(void)
{
	if (!cmd_stack)
		return;
	const char *cur = cmd_stack->cur;
	while (cur < cmd_stack->last)
		if (*cur==' ' || *cur=='\t' || *cur=='\n')
			++cur;
		else if (*cur == '#')
			while (++cur < cmd_stack->last  &&  *cur != '\n')
				continue;
		else
			break;
	cmd_stack->cur = cur;
}

static cmd_list *
new_cmd_item(void)
{
	cmd_list *new_item = dc_malloc(sizeof *new_item);
	memset(new_item, 0, sizeof *new_item); //paranoia
	new_item->pushback = EOF; //EOF signals that there is no pending pushback
	return new_item;
}

/* push current command context and start new one */
static void
push_cmd_stack(dc_data *datum)
{
	//special case #1: a number just gets pushed back on the evaluation stack
	if (datum->dc_type == DC_NUMBER) {
		dc_push(*datum);
		return;
	}
	if (datum->dc_type != DC_STRING)
		dc_garbage("in push_cmd_stack() datum", -1);

	//special case #2: is this tail-recursion of the currently active string?
	if (cmd_stack && cmd_stack->cur && cmd_stack->v.base == datum->v.string) {
		skip_to_non_whitespace();
		if (cmd_stack->cur == cmd_stack->last) {
			dc_free_str(&datum->v.string); //drop excess reference count
			cmd_stack->cur = cmd_stack->first;
			++cmd_stack->tail_depth;
			goto check_recursion_depth;
		}
	}

	//normal case: push a new command context
	cmd_list *new_top = new_cmd_item();
	new_top->v.base = datum->v.string;
	new_top->cur = new_top->first = dc_str2charp(datum->v.string);
	new_top->last = new_top->first + dc_strlen(datum->v.string);
	new_top->tail_depth = 1;
	new_top->backlink = cmd_stack;
	cmd_stack = new_top;

check_recursion_depth:
	++recursion_depth;
	if (max_recursion_depth && max_recursion_depth <= recursion_depth)
		interrupt_seen = 2;
}

/* pop top of cmd_stack, freeing its node */
static void
pop_cmd_stack(void)
{
	cmd_list *old = cmd_stack;
	cmd_stack = old->backlink;
	if (old->cur)
		dc_free_str(&old->v.base);
	free(old);
}

/* unwind out of recursion, taking care to note tail recursions */
static dc_status
cmd_unwind(unsigned int levels)
{
	recursion_depth -= levels < recursion_depth ? levels : recursion_depth;
	while (0 < levels) {
		if (!cmd_stack || !cmd_stack->cur) {
			//if not eval-ing a string, we can't unwind further
			return DC_QUIT;
		} else if (levels < cmd_stack->tail_depth) {
			//we have unwound into the middle of a virtual stack-of-tails
			cmd_stack->tail_depth -= levels;
			cmd_stack->cur = cmd_stack->last;
			break;
		} else {
			//all recursions using current top-of-cmd_stack have been exited
			levels -= cmd_stack->tail_depth;
			pop_cmd_stack();
		}
	}
	return DC_OKAY;
}

/* read the next input character from supplied stream, or EOF if appropriate;
 * if "peekonly" is true, then do not consume the character from the input stream
 */
static int
dc_get_next_chr(int peekonly)
{
	int c;
	while (cmd_stack && cmd_stack->pushback==EOF
			&& cmd_stack->cur && cmd_stack->cur == cmd_stack->last)
		pop_cmd_stack();
	if (!cmd_stack) {
		c = EOF;
	} else if (cmd_stack->pushback != EOF) {
		c = cmd_stack->pushback;
		if (!peekonly)
			cmd_stack->pushback = EOF;
	} else if (cmd_stack->cur) {
		c = *cmd_stack->cur;
		if (!peekonly)
			++cmd_stack->cur;
	} else if (cmd_stack->v.fp == stdin && interactive_session) {
		c = dc_getchar(peekonly);
	} else {
		c = getc(cmd_stack->v.fp);
		if (peekonly)
			ungetc(c, cmd_stack->v.fp);
	}
	return c;
}


/* read and consume the next input character from supplied stream, or EOF if appropriate */
static int
dc_nextchr(void)
{
	return dc_get_next_chr(0);
}

/* stash "c" back on the front of the cmd_stack stream; only one character
 * of pushback is guaranteed, and then only if "c" is the most recently read
 * character from "stream" */
static void
dc_unget(int c)
{
	if (!cmd_stack)
		return; //should be impossible: maybe should abort instead?
	cmd_stack->pushback = c; //only one-deep is supported
}

/* complain about a stream ending in the middle of a command pairing */
static int
eos_whimper(int c)
{
	fprintf(stderr, "%s: stream ended in the mid"
			"dle of processing a '%c' command\n", progname, c);
	return DC_EOS;
}

/* notice when an interrupt event happens */
static void
dc_trap_interrupt(int signo)
{
	signal(signo, dc_trap_interrupt);
	interrupt_seen = 1;
}


/* dc_nextcmd does the grunt work of figuring out what each input
 * character means; used by both dc_evalstr and dc_evalfile
 */
static dc_status
dc_nextcmd(void)
{
	int negcmp = 0;
	int c = dc_nextchr();
	int cmd = c;
	int tmpint;
	dc_data datum;

	switch (c){
	case EOF:
		return DC_EOS;
	case '_': case '.':
	case '0': case '1': case '2': case '3':
	case '4': case '5': case '6': case '7':
	case '8': case '9': case 'A': case 'B':
	case 'C': case 'D': case 'E': case 'F':
		dc_unget(c);
		dc_push(dc_getnum(dc_nextchr, dc_ibase, &tmpint));
		dc_unget(tmpint);
		break;
	case ' ': case '\t': case '\n':
		/* standard command separators */
		break;
	case '#':	/* comment; skip remainder of current line */
		do { c = dc_nextchr(); } while (c!=EOF && c!='\n');
		break;

	case '+':	/* add top two stack elements */
		dc_binop(dc_add, dc_scale);
		break;
	case '-':	/* subtract top two stack elements */
		dc_binop(dc_sub, dc_scale);
		break;
	case '*':	/* multiply top two stack elements */
		dc_binop(dc_mul, dc_scale);
		break;
	case '/':	/* divide top two stack elements */
		dc_binop(dc_div, dc_scale);
		break;
	case '%':
		/* take the remainder from division of the top two stack elements */
		dc_binop(dc_rem, dc_scale);
		break;
	case '~':
		/* Do division on the top two stack elements.
		 * Return the quotient as next-to-top of stack
		 * and the remainder as top-of-stack.
		 */
		dc_binop2(dc_divrem, dc_scale);
		break;
	case '^':	/* exponentiation of the top two stack elements */
		dc_binop(dc_exp, dc_scale);
		break;
	case '|':
		/* Consider the top three elements of the stack as (base, exp, mod),
		 * where mod is top-of-stack, exp is next-to-top, and base is
		 * just below that. Mod must be non-zero, exp must be non-negative,
		 * and all three must be integers. Push the result of raising
		 * base to the exp power, reduced modulo mod. If we had base in
		 * register b, exp in register e, and mod in register m then this
		 * is conceptually equivalent to "lble^lm%", but it is implemented
		 * in a more efficient manner, and can handle arbitrarily large
		 * values for exp.
		 */
		dc_triop(dc_modexp, dc_scale);
		break;
	case 'v':	/* replace top of stack with its square root */
		dc_monop(dc_sqrt, dc_scale);
		break;

	case 'c':	/* clear whole stack */
		dc_clear_stack();
		break;
	case 'd':	/* duplicate the datum on the top of stack */
		if (dc_top_of_stack(&datum) == DC_SUCCESS)
			dc_push(dc_dup(datum));
		break;
	case 'f':	/* print list of all stack items */
		dc_printall(dc_obase);
		break;
	case 'n':
		/* print the value popped off of top-of-stack;
		 * do not add a trailing newline
		 */
		if (dc_pop(&datum) == DC_SUCCESS)
			dc_print(datum, dc_obase, DC_NONL, DC_TOSS);
		break;
	case 'p':
		/* print the datum on the top of stack (without popping it),
		 * with a trailing newline
		 */
		if (dc_top_of_stack(&datum) == DC_SUCCESS)
			dc_print(datum, dc_obase, DC_WITHNL, DC_KEEP);
		break;
	case 'P':
		/* Pop the value off the top of a stack.  If it is
		 * a number, dump out the integer portion of its
		 * absolute value as a "base UCHAR_MAX+1" byte stream;
		 * if it is a string, just print it.
		 * In either case, do not append a trailing newline.
		 */
		if (dc_pop(&datum) == DC_SUCCESS){
			if (datum.dc_type == DC_NUMBER)
				dc_dump_num(datum.v.number);
			else if (datum.dc_type == DC_STRING)
				dc_out_str(datum.v.string, DC_TOSS);
			else
				dc_garbage("at top of stack", -1);
		}
		dc_flush_stdout(0);
		break;
	case 'r':	/* rotate (swap) the top two elements on the stack */
		dc_stack_rotate(2);
		break;
	case 'R':
		/* pop a value off of the evaluation stack;
		 * rotate the top remaining stack elements that many
		 * places forward (negative numbers mean rotate backward).
		 */
		if (dc_int_from_TOS(&tmpint, 0) != DC_SUCCESS) {
			dc_warn("rotation count must be numeric");
			break;
		}
		dc_discard_TOS();
		dc_stack_rotate(tmpint);
		break;

	case '[':	/* read to balancing ']' into a dc_str */
		//cheating here and using cmd_stack directly
		//(i.e., not through dc_nextchr())
		if (cmd_stack && cmd_stack->cur) {
			const char *start = cmd_stack->cur;
			size_t count = 1;
			for (; 0<count && cmd_stack->cur<cmd_stack->last; ++cmd_stack->cur)
				if (*cmd_stack->cur == ']') --count;
				else if (*cmd_stack->cur == '[') ++count;
			size_t len = cmd_stack->cur - start;
			dc_push(dc_makestring(start, (count==0 ? len-1 : len), NULL));
		} else {
			datum = dc_readstring(dc_nextchr, '[', ']', NULL);
			if (datum.dc_type == DC_STRING)
				dc_push(datum);
			else
				dc_warn("encountered EOF while looking for closing ]");
		}
		break;
	case ']':	/* common error, give gentler message*/
		dc_warn("unbalanced ']'");
		break;
	case 'a':	/* convert top of stack to an ascii character */
		if (dc_pop(&datum) == DC_SUCCESS){
			char tmpc = '?';
			static int want_warn = 1;
			if (want_warn) {
				dc_warn("warning: 'a' command is deprecated"
					"\n\t(contact <bug-dc@gnu.org> if you actually use it)");
				want_warn = 0;
			}
			if (datum.dc_type == DC_NUMBER){
				dc_push(datum);
				dc_push(dc_int2data(1+UCHAR_MAX));
				dc_binop(dc_rem, dc_scale);
				if (dc_pop(&datum) != DC_SUCCESS){
					/*impossible*/
				}
				tmpc = (char) dc_num2int(datum.v.number, '?');
				dc_free_num(&datum.v.number);
			}else if (datum.dc_type == DC_STRING){
				tmpc = *dc_str2charp(datum.v.string);
				dc_free_str(&datum.v.string);
			}else{
				dc_garbage("at top of stack", -1);
			}
			dc_push(dc_makestring(&tmpc, 1, NULL));
		}
		break;

	case 'i':	/* set input base to value on top of stack */
		if (dc_int_from_TOS(&tmpint, -1) != DC_SUCCESS
				||  tmpint < 2  ||  DC_IBASE_MAX < tmpint) {
			fprintf(stderr, "%s: input base must be a number "
					"between 2 and %d (inclusive)\n", progname, DC_IBASE_MAX);
			break;
		}
		dc_discard_TOS();
		dc_ibase = tmpint;
		break;
	case 'k':	/* set scale to value on top of stack */
		if (dc_int_from_TOS(&tmpint, -1) != DC_SUCCESS  ||  tmpint < 0) {
			dc_warn("scale must be a nonnegative number");
			break;
		}
		dc_discard_TOS();
		dc_scale = tmpint;
		break;
	case 'o':	/* set output base to value on top of stack */
		if (dc_int_from_TOS(&tmpint, -1) != DC_SUCCESS  ||  tmpint < 2) {
			dc_warn("output base must be a number greater than 1");
			break;
		}
		dc_discard_TOS();
		dc_obase = tmpint;
		break;
	case 'I':	/* push the current input base onto the stack */
		dc_push(dc_int2data(dc_ibase));
		break;
	case 'K':	/* push the current scale onto the stack */
		dc_push(dc_int2data(dc_scale));
		break;
	case 'O':	/* push the current output base onto the stack */
		dc_push(dc_int2data(dc_obase));
		break;
	case 'z':	/* push the current stack depth onto the top of stack */
		dc_push(dc_int2data(dc_tell_stackdepth()));
		break;
	case 'X':	/* replace the number on top-of-stack with its scale factor */
		if (dc_pop(&datum) == DC_SUCCESS){
			tmpint = 0;
			if (datum.dc_type == DC_NUMBER)
				tmpint = dc_tell_scale(datum.v.number);
			dc_push(dc_int2data(tmpint));
		}
		break;
	case 'Z':	/* replace the datum on the top-of-stack with its length */
		if (dc_pop(&datum) == DC_SUCCESS)
			dc_push(dc_int2data(dc_tell_length(datum)));
		break;

	case '!':	/* read to newline and call system() on resulting string */
		/* read to newline and call system() on resulting string,
		 * except if part of digraph !< != !>, in which case
		 * run a negated comparison
		 */
		c = dc_get_next_chr(1);
		if ( ! (c == '<' || c == '=' || c == '>') ){
			//cheating here and using cmd_stack directly
			//(i.e., not through dc_nextchr())
			if (cmd_stack && cmd_stack->cur) {
				cmd_stack->cur = dc_system(cmd_stack->cur);
			} else {
				datum = dc_readstring(dc_nextchr, '\n', '\n', NULL);
				if (datum.dc_type != DC_STRING || ferror(cmd_stack->v.fp))
					return DC_EOS;
				(void)dc_system(dc_str2charp(datum.v.string));
				dc_free_str(&datum.v.string);
			}
			break;
		}
		/*else, not "system" call after all, but a negated comparison:*/
		negcmp = !negcmp;
		c = dc_nextchr();
		/*@fallthrough@*/
	case '<':
	case '=':
	case '>':
		/* if relation holds for top two stack elements, which must
		 * both be numeric, then eval the register named by dc_nextchr()
		 */
		tmpint = ! dc_cmpop(c, !negcmp);
		if ((c = dc_nextchr()) == EOF)
			return eos_whimper(cmd);
		if (tmpint  ||  dc_register_get(c, &datum) != DC_SUCCESS)
			break;
		push_cmd_stack(&datum);
		break;
	case 'x':	/* eval the datum popped from top of stack */
		if (dc_pop(&datum) == DC_SUCCESS)
			push_cmd_stack(&datum);
		break;
	case '?':	/* read a line from standard-input and eval it */
		datum = dc_readline();
		if (datum.dc_type != DC_STRING)
			return DC_EOS;
		if (ferror(stdin)){
			dc_free_str(&datum.v.string);
			fprintf(stderr, "%s: ", progname);
			perror("error reading stdin");
			return DC_EOS;
		}
		push_cmd_stack(&datum);
		break;
	case 'q':	/* quit two levels of evaluation, possibly exiting program */
		return cmd_unwind(2);
	case 'Q':
		/* quit out of top-of-stack nested evals; pops value from stack;
		 * does not exit program (stops short if necessary)
		 */
		if (dc_int_from_TOS(&tmpint, -1) != DC_SUCCESS  ||  tmpint < 0) {
			dc_warn("Q command requires a number >= 0");
			break;
		}
		if (recursion_depth < tmpint)
			dc_warn("Q command argument exceeded string execution depth");
		dc_discard_TOS();
		(void) cmd_unwind(tmpint);
		break;

	case 'l':
		/* "load" -- push value on top of register stack named by dc_nextchr()
		 * onto top of evaluation stack; does not modify the register stack
		 */
		if ((c = dc_nextchr()) == EOF)
			return eos_whimper(cmd);
		if (dc_register_get(c, &datum) == DC_SUCCESS)
			dc_push(datum);
		break;
	case 's':
		/* "store" -- replace top of register stack named by dc_nextchr()
		 * with the value popped from the top of the evaluation stack
		 */
		if ((c = dc_nextchr()) == EOF)
			return eos_whimper(cmd);
		if (dc_pop(&datum) == DC_SUCCESS)
			dc_register_set(c, datum);
		break;
	case 'L':
		/* pop a value off of register stack named by dc_nextchr()
		 * and push it onto the evaluation stack
		 */
		if ((c = dc_nextchr()) == EOF)
			return eos_whimper(cmd);
		if (dc_register_pop(c, &datum) == DC_SUCCESS)
			dc_push(datum);
		break;
	case 'S':
		/* pop a value off of the evaluation stack and push it
		 * onto the register stack named by dc_nextchr()
		 */
		if ((c = dc_nextchr()) == EOF)
			return eos_whimper(cmd);
		if (dc_pop(&datum) == DC_SUCCESS)
			dc_register_push(c, datum);
		break;
	case ':':	/* store into array */
		if ((c = dc_nextchr()) == EOF)
			return eos_whimper(cmd);
		if (dc_int_from_TOS(&tmpint, -1) != DC_SUCCESS  ||  tmpint < 0) {
			dc_warn("array index must be a nonnegative integer");
		} else {
			dc_data value;
			dc_discard_TOS();
			if (dc_pop(&value) == DC_SUCCESS)
				dc_array_set(c, tmpint, value);
			else
				dc_push(dc_int2data(tmpint)); /*restore stack*/
		}
		break;
	case ';':	/* retrieve from array */
		if ((c = dc_nextchr()) == EOF)
			return eos_whimper(cmd);
		if (dc_int_from_TOS(&tmpint, -1) != DC_SUCCESS  ||  tmpint < 0) {
			dc_warn("array index must be a nonnegative integer");
			break;
		}
		dc_discard_TOS();
		dc_push(dc_array_get(c, tmpint));
		break;

	default:	/* What did that user mean? */
		fprintf(stderr, "%s: ", progname);
		dc_show_id(stderr, c, " unimplemented\n");
		break;
	}
	return DC_OKAY;
}


/* takes a string (from the command-line) and evals it */
int
dc_evalstr(const char *string)
{
	int retval = DC_FAIL;
	dc_data dcstring = dc_makestring(string, strlen(string), NULL);
	if (dcstring.dc_type != DC_STRING)
		dc_garbage("impossible command-line string", -1);
	push_cmd_stack(&dcstring); //dcstring gets freed when popped off cmdstack
	while (!interrupt_seen) {
		switch (dc_nextcmd()) {
		case DC_OKAY:
			continue;
		case DC_QUIT:
			retval = DC_EXIT_REQUEST;
			goto cleanup;
		default:
			retval = DC_SUCCESS;
			goto cleanup;
		}
	}
	fprintf(stderr, "\nmax_recursion_depth exceeded; "
			"evaluation interrupted!\n");
cleanup:
	while (cmd_stack) //unwind all pending evaluations
		pop_cmd_stack();
	return retval;
}

/* Set the "interactive_session".
 * An interactive session has some special case signal handling
 * (to allow for breaking out of long loops), and enables
 * readline (or libinput) editing when reading from stdin
 * (if the build enabled said feature).
 *
 * A positive "value" means to force the flag to be true;
 * a zero "value" forces the flag to be false;
 * a negative "value" means to attempt to auto-detect if
 * the session is attached to a terminal;
 */
void
dc_set_interactive_flag(int value)
{
	if (value >= 0)
		interactive_session = value;
#ifdef HAVE_UNISTD_H
	else
		interactive_session = isatty(fileno(stdin));
#endif
}

/* This is the main function of the whole DC program. */
int
dc_evalfile(FILE *fp)
{
	// Signals are awkward: we want to allow interactive users
	// to break out of long running macros, but otherwise we
	// prefer that SIGINT not be given any special treatment.
	// Sometimes "no special treatment" means to continue to
	// *ignore* the signal, but usually it means to kill the program.
	void (*sigint_default)(int) = signal(SIGINT, SIG_IGN);
	signal(SIGINT, sigint_default);

	//set an input context for fp as the top of the execution stack
	cmd_list *new_top = new_cmd_item();
	new_top->cur = NULL;
	new_top->v.fp = fp;
	new_top->backlink = cmd_stack;
	cmd_stack = new_top;

	if (fp == stdin)
		clearerr(stdin);
	interrupt_seen = 0;
	for (;;) {
		//only trap signal if we are interactive *and* are evaluating a string:
		if (interactive_session && cmd_stack && cmd_stack->cur)
			signal(SIGINT, dc_trap_interrupt);
		switch (dc_nextcmd()){
		case DC_OKAY:
			break;
		case DC_QUIT:
			goto reset_and_exit_success;
		case DC_EOS:
			if (ferror(fp))
				goto error_fail;
			goto reset_and_return_success;
		}

		if (ferror(fp))
			goto error_fail;
		if (interrupt_seen == 2) {
			fprintf(stderr, "\nmax_recursion_depth exceeded; "
					"evaluation interrupted!\n");
			goto reset_and_exit_fail;
		} else if (interrupt_seen) {
			interrupt_seen = 0;
			while (cmd_stack && cmd_stack->cur) //unwind all pending evaluations below us
				pop_cmd_stack();
			fprintf(stderr, "\nInterrupt!\n");
		}
		//reset trap signal if we are no longer evaluating a string:
		if (interactive_session && !(cmd_stack && cmd_stack->cur))
			signal(SIGINT, sigint_default);
	}

error_fail:
	signal(SIGINT, sigint_default);
	fprintf(stderr, "%s: ", progname);
	perror("error reading input");
	return DC_FAIL;
reset_and_exit_fail:
	signal(SIGINT, sigint_default);
	return DC_FAIL;
reset_and_exit_success:
	signal(SIGINT, sigint_default);
	return DC_EXIT_REQUEST;
reset_and_return_success:
	signal(SIGINT, sigint_default);
	pop_cmd_stack();
	return DC_SUCCESS;
}


/*
 * Local Variables:
 * mode: C
 * tab-width: 4
 * End:
 * vi: set ts=4 :
 */

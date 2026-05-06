/*
 * Handle reading from stdin, possibly using readline, libinput, or getc.
 *
 * Copyright (C) 2024
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

#include "../config.h"
#include <stdio.h>
#include <stdlib.h> /* free, getenv */
#include "dc.h"

static const char *PROMPT = NULL; //NULL => prior to first-readline

static void
set_prompt(void)
{
	const char *p = getenv("DC_PROMPT");
	PROMPT = p ? p : ""; // dc traditionally has no prompt
}


#if defined(READLINE)
# include <readline/readline.h>
# include <readline/history.h>

	dc_data
	dc_readline(void)
	{
		if (!PROMPT) {
			rl_bind_key('\t', rl_insert); // "tab completion" doesn't make sense for dc
			set_prompt();
		}
		char *line = readline(PROMPT);
		if (!line)
			return dc_makestring(NULL, 0, NULL);
		if (*line)
			add_history(line);
		// unfortunately, readline() strips the trailing newline,
		// but some parts of dc parsing (in particular, # comments)
		// depend on its presence
		dc_data result = dc_makestring(line, strlen(line), "\n");
		free(line);
		return result;
	}

	void
	input_cleanup(void)
	{
		// no-op
	}


#elif defined(LIBEDIT)
# include <limits.h>
# include <histedit.h>
	static const char *libedit_prompt(EditLine *x) { return PROMPT; }
	static EditLine *libedit_el = NULL;
	static History *libedit_hist = NULL;
	static HistEvent histev;

	dc_data
	dc_readline(void)
	{
		if (!libedit_el) {
			set_prompt();
			libedit_el = el_init("dc", stdin, stdout, stderr);
			libedit_hist = history_init();
			el_set(libedit_el, EL_EDITOR, "emacs");
			el_set(libedit_el, EL_HIST, history, libedit_hist);
			el_set(libedit_el, EL_PROMPT, libedit_prompt);
			el_source(libedit_el, NULL);
			history(libedit_hist, &histev, H_SETSIZE, INT_MAX);
		}
		int line_len_int;
		const char *line = el_gets(libedit_el, &line_len_int);
		if (line_len_int > 1 || (line_len_int == 1 && *line != '\n'))
			history(libedit_hist, &histev, H_ENTER, line); 
		return dc_makestring(line, (size_t)line_len_int, NULL);
	}

	void
	input_cleanup(void)
	{
		if (libedit_el) {
			history_end(libedit_hist);
			libedit_hist = NULL;
			el_end(libedit_el);
			libedit_el = NULL;
		}
	}


#else /* default to using stdio */

	dc_data
	dc_readline(void)
	{
		if (!PROMPT)
			set_prompt();
		if (*PROMPT) {
			fputs(PROMPT, stderr);
			fflush(stderr);
		}
#undef getchar /*just in case <stdio.h> defines a macro hiding the function*/
		return dc_readstring(getchar, '\n', '\n', "\n");
	}

	void
	input_cleanup(void)
	{
		// no-op
	}

#endif /* stdin handling variations */


int
dc_getchar(int peekonly)
{
	static dc_data linedata;
	static const char *line = NULL;
	static size_t  linelen = 0;
	static size_t  linecur = 0;
	for (;;) {
		if (line && linecur < linelen) {
			int c = line[linecur];
			if (!peekonly)
				++linecur;
			return c;
		}
		if (line) {
			dc_free_str(&linedata.v.string);
			line = NULL;
			linelen = 0;
		}
		linedata = dc_readline();
		if (linedata.dc_type != DC_STRING)
			return EOF;
		line = dc_str2charp(linedata.v.string);
		linelen = dc_strlen(linedata.v.string);
		linecur = 0;
	}
}


/*
 * Local Variables:
 * mode: C
 * tab-width: 4
 * End:
 * vi: set ts=4 :
 */

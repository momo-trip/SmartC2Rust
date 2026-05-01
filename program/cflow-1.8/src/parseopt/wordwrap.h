/*
 * parseopt - generic option parser library
 * Copyright (C) 2023-2025 Sergey Poznyakoff
 *
 * Parseopt is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * Parseopt is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with parseopt. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _WORDWRAP_H
# define _WORDWRAP_H

#include <sys/types.h>
#include <stdarg.h>

typedef struct wordwrap_file *WORDWRAP_FILE;
WORDWRAP_FILE wordwrap_fdopen (int fd);
WORDWRAP_FILE wordwrap_open (int fd,
			     ssize_t (*writer) (void *, const char *, size_t),
			     void *data);
int wordwrap_close (WORDWRAP_FILE wf);
int wordwrap_flush (WORDWRAP_FILE wf);
int wordwrap_error (WORDWRAP_FILE wf);
int wordwrap_set_left_margin (WORDWRAP_FILE wf, unsigned left);
int wordwrap_next_left_margin (WORDWRAP_FILE wf, unsigned left);
int wordwrap_set_right_margin (WORDWRAP_FILE wf, unsigned right);
int wordwrap_write (WORDWRAP_FILE wf, char const *str, size_t len);
int wordwrap_puts (WORDWRAP_FILE wf, char const *str);
int wordwrap_putc (WORDWRAP_FILE wf, int c);
int wordwrap_para (WORDWRAP_FILE wf);
int wordwrap_vprintf (WORDWRAP_FILE wf, char const *fmt, va_list ap);
int wordwrap_printf (WORDWRAP_FILE wf, char const *fmt, ...);
int wordwrap_at_bol (WORDWRAP_FILE wf);
int wordwrap_at_eol (WORDWRAP_FILE wf);
void wordwrap_word_start (WORDWRAP_FILE wf);
void wordwrap_word_end (WORDWRAP_FILE wf);

#endif

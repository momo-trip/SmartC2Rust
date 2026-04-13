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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <wctype.h>
#include <wchar.h>
#include <errno.h>
#include <limits.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include "wordwrap.h"

#define UNSET ((unsigned)-1)
#define ISSET(c) (c != UNSET)
#define DEFAULT_RIGHT_MARGIN 80

struct position
{
  unsigned off;
  unsigned col;
};

#define POSITION_INITIALIZER { 0, 0 }

static inline void
position_init (struct position *pos, unsigned n)
{
  pos->off = pos->col = n;
}

static inline void
position_incr (struct position *pos, int nbytes)
{
  pos->off += nbytes;
  pos->col++;
}

static inline void
position_add (struct position *a, struct position *b)
{
  a->off += b->off;
  a->col += b->col;
}

static inline int
position_eq (struct position *a, struct position *b)
{
  return a->col == b->col;
}

struct wordwrap_file
{
  int fd;                /* Output file descriptor. */
  ssize_t (*writer) (void *, char const *, size_t);
  void *stream;
  unsigned left_margin;  /* Left margin. */
  unsigned right_margin; /* Right margin. */
  char *buffer;          /* Output buffer. */
  size_t bufsize;        /* Size of buffer in bytes. */
  struct position cur;   /* Current position in buffer. */
  struct position last_ws; /* Position of the beginning of the last whitespace
			      sequence written to the buffer. */
  struct position ws_run; /* Number of bytes/columns in the last
			     whitespace sequence. */

  unsigned word_start;   /* Start of a sequence that should be treated as a
			    single word. */
  unsigned next_left_margin; /* Left margin to be set after next flush. */

  int indent;            /* If 1, reindent next line. */
  int unibyte;           /* 0: Normal operation.
			    1: multibyte functions disabled for this line. */
  int err;               /* Last errno value associated with this file. */
};

/*
 * Reset the file for the next input line.
 */
static void
wordwrap_line_init (WORDWRAP_FILE wf, int clrws)
{
  position_init (&wf->cur, wf->left_margin);
  wf->unibyte = 0;
  if (clrws)
    {
      position_init (&wf->ws_run, 0);
    }
}

/*
 * Detect the value of the right margin.  Use TIOCGWINSZ ioctl, the COLUMNS
 * environment variable, or the default value, in that order.
 */
static unsigned
detect_right_margin (WORDWRAP_FILE wf)
{
  struct winsize ws;
  unsigned r = 0;

  ws.ws_col = ws.ws_row = 0;
  if ((ioctl (wf->fd, TIOCGWINSZ, (char *) &ws) < 0) || ws.ws_col == 0)
    {
      char *p = getenv ("COLUMNS");
      if (p)
	{
	  unsigned long n;
	  char *ep;
	  errno = 0;
	  n = strtoul (p, &ep, 10);
	  if (!(errno || *ep || n > UINT_MAX))
	    r = n;
	}
      else
	r = DEFAULT_RIGHT_MARGIN;
    }
  else
    r = ws.ws_col;
  return r;
}

static ssize_t
_ww_fd_writer (void *data, const char *str, size_t n)
{
  WORDWRAP_FILE wf = data;
  return write (wf->fd, str, n);
}

/*
 * Create a wordwrap file operating on file descriptor FD.
 * In the contrast to the libc fdopen, the descriptor is dup'ed.
 * Left margin is set to 0, right margin is auto detected.
 */
WORDWRAP_FILE
wordwrap_open (int fd, ssize_t (*writer) (void *, const char *, size_t),
	       void *data)
{
  struct wordwrap_file *wf;
  int ec;

  if ((wf = calloc (1, sizeof (*wf))) == NULL)
    return NULL;
  if ((wf->fd = dup (fd)) == -1)
    {
      ec = errno;
      free (wf);
      errno = ec;
      return NULL;
    }
  wf->writer = writer;
  wf->stream = data;

  wf->word_start = UNSET;
  wf->next_left_margin = UNSET;

  wordwrap_set_right_margin (wf, 0);

  return wf;
}

WORDWRAP_FILE
wordwrap_fdopen (int fd)
{
  WORDWRAP_FILE wf = wordwrap_open (fd, _ww_fd_writer, NULL);
  wf->stream = wf;
  return wf;
}
/*
 * Close the descriptor associated with the wordwrap file, and deallocate
 * the memory.
 */
int
wordwrap_close (WORDWRAP_FILE wf)
{
  int rc;

  rc = wordwrap_flush (wf);
  close (wf->fd);
  free (wf->buffer);
  free (wf);

  return rc;
}

/*
 * Return true if wordwrap file is at the beginning of line.
 */
int
wordwrap_at_bol (WORDWRAP_FILE wf)
{
  return wf->cur.col == wf->left_margin;
}

/*
 * Return true if wordwrap file is at the end of line.
 */
int
wordwrap_at_eol (WORDWRAP_FILE wf)
{
  return wf->cur.col == wf->right_margin;
}

/*
 * Write SIZE bytes from the buffer to the file.
 * Return the number of bytes written.
 * Set the file error indicator on error.
 */
static ssize_t
full_write (WORDWRAP_FILE wf, size_t size)
{
  ssize_t total = 0;

  while (total < size)
    {
      ssize_t n = wf->writer (wf->stream, wf->buffer + total, size - total);
      if (n == -1)
	{
	  wf->err = errno;
	  break;
	}
      if (n == 0)
	{
	  wf->err = ENOSPC;
	  break;
	}
      total += n;
    }
  return total;
}

/*
 * A fail-safe version of mbrtowc.  If the call to mbrtowc, fails,
 * switches the stream to the unibyte mode.
 */
static inline size_t
safe_mbrtowc (WORDWRAP_FILE wf, wchar_t *wc, const char *s, mbstate_t *ps)
{
  if (!wf->unibyte)
    {
      size_t n = mbrtowc (wc, s, MB_CUR_MAX, ps);
      if (n == (size_t) -1 || n == (size_t) -2)
	wf->unibyte = 1;
      else
	return n;
    }
  *wc = *(unsigned char *)s;
  return 1;
}

/*
 * Return length of the whitespace prefix in STR.
 */
static size_t
wsprefix (WORDWRAP_FILE wf, char const *str, size_t size)
{
  size_t i;
  mbstate_t mbs;
  wchar_t wc;

  memset (&mbs, 0, sizeof (mbs));
  for (i = 0; i < size; )
    {
      size_t n = safe_mbrtowc (wf, &wc, &str[i], &mbs);

      if (!iswblank (wc))
	break;

      i += n;
    }

  return i;
}

/*
 * Rescan SIZE bytes from the current buffer from the current offset.
 * Update offset, column, and whitespace segment counters.
 */
static void
wordwrap_rescan (WORDWRAP_FILE wf, size_t size)
{
  mbstate_t mbs;
  wchar_t wc;

  wordwrap_line_init (wf, 0);

  memset (&mbs, 0, sizeof (mbs));
  while (wf->cur.off < size)
    {
      size_t n = safe_mbrtowc (wf, &wc, &wf->buffer[wf->cur.off], &mbs);

      if (iswblank (wc))
	{
	  if (!(wf->ws_run.col > 0 &&
		wf->last_ws.col + wf->ws_run.col == wf->cur.col))
	    {
	      wf->last_ws = wf->cur;
	      position_init (&wf->ws_run, 0);
	    }
	  position_incr (&wf->ws_run, n);
	}

      position_incr(&wf->cur, n);
    }
}

static struct position
wordwrap_last_ws (WORDWRAP_FILE wf, size_t size, struct position *last_ws)
{
  mbstate_t mbs;
  wchar_t wc;
  struct position cur = POSITION_INITIALIZER;
  struct position ws_run = POSITION_INITIALIZER;

  memset (&mbs, 0, sizeof (mbs));
  last_ws->off = last_ws->col = UNSET;
  while (cur.off < size)
    {
      size_t n = safe_mbrtowc (wf, &wc, &wf->buffer[cur.off], &mbs);
      if (iswblank (wc))
	{
	  if (!(ws_run.col > 0 && last_ws->col + ws_run.col == cur.col))
	    {
	      *last_ws = cur;
	      position_init (&ws_run, 0);
	    }
	  position_incr (&ws_run, n);
	}
      else
	{
	  position_init (last_ws, UNSET);
	  position_init (&ws_run, 0);
	}
      position_incr (&cur, n);
    }
  return cur;
}

/*
 * Flush SIZE bytes from the current buffer to the FD.
 * Reinitialize WF for the next line.
 */
static int
flush_line (WORDWRAP_FILE wf, size_t size)
{
  ssize_t n;
  struct position pos, last_ws;

  if (wf->ws_run.off > 0 && size == wf->last_ws.off + wf->ws_run.off)
    {
      pos = last_ws = wf->last_ws;
    }
  else
    {
      pos = wordwrap_last_ws (wf, size, &last_ws);
    }

  if ((pos.col >= wf->left_margin && wf->cur.col > wf->left_margin) ||
      size == wf->cur.off)
    {
      if (last_ws.off != UNSET)
	pos = last_ws;

      n = full_write (wf, pos.off);
      if (n == -1)
	return -1;

      if (n < pos.off)
	{
	  //FIXME
	  abort ();
	}
    }

  wf->writer (wf->stream, "\n", 1);

  if (ISSET (wf->next_left_margin))
    {
      wf->left_margin = wf->next_left_margin;
      wf->next_left_margin = UNSET;
    }

  n = wf->cur.off - size;
  if (n > 0)
    {
      size_t wsn;

      wsn = wsprefix (wf, wf->buffer + size, n);

      size += wsn;
      n -= wsn;

      if (n)
	{
	  memmove (wf->buffer + wf->left_margin, wf->buffer + size, n);
	  wf->cur.off = wf->left_margin + n;
	  position_init (&wf->ws_run, 0);
	}
    }

  if (wf->indent)
    {
      memset (wf->buffer, ' ', wf->left_margin);
      wf->indent = 0;
      position_init (&wf->last_ws, 0);
      position_init (&wf->ws_run, wf->left_margin);
    }

  wordwrap_rescan (wf, wf->left_margin + n);

  return 0;
}

/*
 * Flush the wordwrap file buffer.
 */
int
wordwrap_flush (WORDWRAP_FILE wf)
{
  if (wf->cur.col > wf->left_margin)
    return flush_line (wf, wf->cur.off);
  return 0;
}

/*
 * Return error indicator (last errno value).
 */
int
wordwrap_error (WORDWRAP_FILE wf)
{
  return wf->err;
}

/*
 * Set left margin value.
 */
int
wordwrap_set_left_margin (WORDWRAP_FILE wf, unsigned left)
{
  int bol;

  if (left == wf->left_margin)
    return 0;
  else if (left >= wf->right_margin)
    {
      wf->err = errno = EINVAL;
      return -1;
    }

  bol = wordwrap_at_bol (wf);
  wf->left_margin = left;
  wf->indent = 1;
  if (left < wf->cur.col ||
      (left == wf->cur.col && (wf->ws_run.col == 0 ||
			      wf->cur.col > wf->last_ws.col + wf->ws_run.col)))
    {
      if (!bol)
	flush_line (wf, wf->cur.off);//FIXME: remove trailing ws
      else
	wordwrap_line_init (wf, 1);
    }
  else if (left > wf->cur.col)
    {
      size_t n = wf->left_margin - wf->cur.col;
      if (n > 0)
	{
	  memset (wf->buffer + wf->cur.off, ' ', n);
	  wf->last_ws = wf->cur;
	  position_init (&wf->ws_run, n);
	  position_add (&wf->cur, &wf->ws_run);
	  wf->unibyte = 0;
	}
      else
	wordwrap_line_init (wf, 1);
    }
  return 0;
}

/*
 * Set delayed left margin value.  The new value will take effect after the
 * current line is flushed.
 */
int
wordwrap_next_left_margin (WORDWRAP_FILE wf, unsigned left)
{
  if (left == wf->left_margin)
    return 0;
  else if (left >= wf->right_margin)
    {
      wf->err = errno = EINVAL;
      return -1;
    }
  wf->next_left_margin = left;
  wf->indent = 1;
  return 0;
}

/*
 * Set right margin for the file.
 */
int
wordwrap_set_right_margin (WORDWRAP_FILE wf, unsigned right)
{
  if (right == 0)
    right = detect_right_margin (wf);

  if (right == wf->right_margin)
    return 0;
  else if (right <= wf->left_margin)
    {
      wf->err = errno = EINVAL;
      return -1;
    }
  else
    {
      char *p;
      size_t size;

      if (right < wf->cur.off)
	{
	  if (wordwrap_flush (wf))
	    return -1;
	}

      size = MB_CUR_MAX * (right + 1);
      p = realloc (wf->buffer, size);
      if (!p)
	{
	  wf->err = errno;
	  return -1;
	}

      wf->buffer = p;
      wf->bufsize = size;
      wf->right_margin = right;
    }

  return 0;
}

/*
 * Mark current output position as the word start.  The normal whitespace
 * splitting is disabled, until wordwrap_word_end is called or the current
 * buffer is flushed, whichever happens first.
 * The functions wordwrap_word_start () / wordwrap_word_end () mark the
 * sequence of characters that should not be split on whitespace, such as,
 * e.g. option name with argument in help output ("-f FILE").
 */
void
wordwrap_word_start (WORDWRAP_FILE wf)
{
  wf->word_start = wf->cur.off;
}

/*
 * Disable word marker.
 */
void
wordwrap_word_end (WORDWRAP_FILE wf)
{
  wf->word_start = UNSET;
}

/*
 * Write LEN bytes from the string STR to the wordwrap file.
 */
int
wordwrap_write (WORDWRAP_FILE wf, char const *str, size_t len)
{
  size_t i;
  wchar_t wc;
  mbstate_t mbs;

  memset (&mbs, 0, sizeof (mbs));
  for (i = 0; i < len; )
    {
      size_t n = safe_mbrtowc (wf, &wc, &str[i], &mbs);

      if (wf->cur.col + 1 == wf->right_margin || wc == '\n')
	{
	  size_t len;

	  if (ISSET (wf->word_start))
	    {
	      len = wf->word_start;
	      wf->word_start = UNSET;
	    }
	  else if (!iswspace (wc) && wf->ws_run.off > 0 && wf->last_ws.off > 0)
	    len = wf->last_ws.off;
	  else
	    len = wf->cur.off;

	  flush_line (wf, len);
	  if (wc == '\n')
	    {
	      i += n;
	      continue;
	    }
	}

      if (iswblank (wc))
	{
	  if (wf->cur.col == wf->left_margin)
	    {
	      /* Skip leading whitespace */
	      i += n;
	      continue;
	    }
	  else if (!(wf->ws_run.col > 0 &&
		     wf->last_ws.col + wf->ws_run.col == wf->cur.col))
	    {
	      wf->last_ws = wf->cur;
	      position_init (&wf->ws_run, 0);
	    }
	  position_incr (&wf->ws_run, n);
	}

      memcpy (wf->buffer + wf->cur.off, str + i, n);

      position_incr (&wf->cur, n);
      i += n;
    }
  return 0;
}

/*
 * Write a nul-terminated string STR to the file (terminating \0 not
 * included).
 */
int
wordwrap_puts (WORDWRAP_FILE wf, char const *str)
{
  return wordwrap_write (wf, str, strlen (str));
}

/*
 * Write a single character to the file.
 */
int
wordwrap_putc (WORDWRAP_FILE wf, int c)
{
  char ch = c;
  return wordwrap_write (wf, &ch, 1);
}

/*
 * Insert a paragraph (empty line).
 */
int
wordwrap_para (WORDWRAP_FILE wf)
{
  if (wordwrap_at_bol (wf))
    return wordwrap_write (wf, "\n", 1);
  else
    return wordwrap_write (wf, "\n\n", 2);
}

/*
 * Format AP according to FMT and write the formatted output to file.
 */
int
wordwrap_vprintf (WORDWRAP_FILE wf, char const *fmt, va_list ap)
{
  size_t buflen = 64;
  char *buf;
  ssize_t n;
  int rc;

  buf = malloc (buflen);
  if (!buf)
    {
      wf->err = errno;
      return -1;
    }

  for (;;)
    {
      va_list aq;

      va_copy (aq, ap);
      n = vsnprintf (buf, buflen, fmt, aq);
      va_end (aq);

      if (n < 0 || n >= buflen || !memchr(buf, '\0', n + 1))
	{
	  char *p;

	  if ((size_t) -1 / 3 * 2  <= buflen)
	    {
	      wf->err = ENOMEM;
	      free (buf);
	      return -1;
	    }

	  buflen += (buflen + 1) / 2;
	  p = realloc (buf, buflen);
	  if (!p)
	    {
	      wf->err = errno;
	      free (buf);
	      return -1;
	    }
	  buf = p;
	}
      else
	break;
    }

  rc = wordwrap_write (wf, buf, n);
  free (buf);
  return rc;
}

/*
 * Format argument list according to FMT and write the formatted output
 * to file.
 */
int
wordwrap_printf (WORDWRAP_FILE wf, char const *fmt, ...)
{
  va_list ap;
  int rc;

  va_start (ap, fmt);
  rc = wordwrap_vprintf (wf, fmt, ap);
  va_end (ap);
  return rc;
}

/* Copyright 1992-2024 Free Software Foundation, Inc.
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

// needed for _PC_NAME_MAX, pathconf()
#include "posix.h"
#include "nonposix.h"

#include "lib.h"

/* file_name_max(dir) does the same as pathconf(dir, _PC_NAME_MAX) */

#ifdef _POSIX_VERSION

size_t file_name_max(const char *fname)
{
  return pathconf(fname, _PC_NAME_MAX);
}

#else /* not _POSIX_VERSION */

#ifdef HAVE_DIRENT_H
#include <dirent.h>
#else /* not HAVE_DIRENT_H */
#ifdef HAVE_SYS_DIR_H
#include <sys/dir.h>
#endif /* HAVE_SYS_DIR_H */
#endif /* not HAVE_DIRENT_H */

#ifndef NAME_MAX
#ifdef MAXNAMLEN
#define NAME_MAX MAXNAMLEN
#endif
#endif

#ifndef NAME_MAX
#ifdef MAXNAMELEN
#define NAME_MAX MAXNAMELEN
#endif
#endif

#ifndef NAME_MAX
#include <stdio.h>
#ifdef FILENAME_MAX
#define NAME_MAX FILENAME_MAX
#endif
#endif

#ifndef NAME_MAX
#define NAME_MAX 14
#endif

size_t file_name_max(const char *)
{
  return NAME_MAX;
}

#endif /* not _POSIX_VERSION */

// Local Variables:
// fill-column: 72
// mode: C++
// End:
// vim: set cindent noexpandtab shiftwidth=2 textwidth=72:

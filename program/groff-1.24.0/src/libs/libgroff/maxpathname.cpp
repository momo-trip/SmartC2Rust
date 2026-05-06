/* Copyright 2005-2024 Free Software Foundation, Inc.
     Written by Werner Lemberg (wl@gnu.org)

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

#include "lib.h"

// needed for pathconf()
#include "posix.h"
#include "nonposix.h"

/* path_name_max(dir) does the same as pathconf(dir, _PC_PATH_MAX) */

#ifdef _POSIX_VERSION

size_t path_name_max()
{
  return pathconf("/", _PC_PATH_MAX) < 1 ? 1024 : pathconf("/",_PC_PATH_MAX);
}

#else /* not _POSIX_VERSION */

#include <stdlib.h>

#ifdef HAVE_DIRENT_H
# include <dirent.h>
#else /* not HAVE_DIRENT_H */
# ifdef HAVE_SYS_DIR_H
#  include <sys/dir.h>
# endif /* HAVE_SYS_DIR_H */
#endif /* not HAVE_DIRENT_H */

#ifndef PATH_MAX
# ifdef MAXPATHLEN
#  define PATH_MAX MAXPATHLEN
# else /* !MAXPATHLEN */
#  ifdef MAX_PATH
#   define PATH_MAX MAX_PATH
#  else /* !MAX_PATH */
#   ifdef _MAX_PATH
#    define PATH_MAX _MAX_PATH
#   else /* !_MAX_PATH */
#    define PATH_MAX 255
#   endif /* !_MAX_PATH */
#  endif /* !MAX_PATH */
# endif /* !MAXPATHLEN */
#endif /* !PATH_MAX */

size_t path_name_max()
{
  return PATH_MAX;
}

#endif /* not _POSIX_VERSION */

// Local Variables:
// fill-column: 72
// mode: C++
// End:
// vim: set cindent noexpandtab shiftwidth=2 textwidth=72:

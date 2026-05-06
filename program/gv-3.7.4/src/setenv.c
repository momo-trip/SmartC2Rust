/*
    setenv and unsetenv emulation code
    Copyright (C) 1992,1995-1999,2000-2003,2005-2008 Free Software Foundation,
    Copyright (C) 2008  Markus Steinborn

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "ac_config.h"
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

extern char **environ;

int gnu_gv_setenv (const char *name, const char *value, int overwrite)
{
   if (!overwrite && getenv(name))
      return 0;
      
   char *tmp = malloc(strlen(name)+strlen(value)+2);
   strcpy(tmp, name);
   strcat(tmp, "=");
   strcat(tmp, value);
   return putenv(tmp);
   /* The string pointed to by string becomes part of
      the environment, so altering the string changes the environment.
      so we cannot free it. */
}

int gnu_gv_unsetenv (const char *name) /* Taken from GNULIB, slightly modified */
{
  size_t len;
  char **ep;

  if (name == NULL || *name == '\0' || strchr (name, '=') != NULL)
    {
      errno = EINVAL;
      return -1;
    }

  len = strlen (name);

  ep = environ;
  while (*ep != NULL)
    if (!strncmp (*ep, name, len) && (*ep)[len] == '=')
      {
	/* Found it.  Remove this pointer by moving later ones back.  */
	char **dp = ep;

	do
	  dp[0] = dp[1];
	while (*dp++);
	/* Continue the loop in case NAME appears again.  */
      }
    else
      ++ep;

  return 0;
}

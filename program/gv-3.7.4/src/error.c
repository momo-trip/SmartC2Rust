/*
**
** error.c
**
** Copyright (C) 1995, 1996, 1997 Johannes Plass
** Copyright (C) 2004 Jose E. Marchesi
** 
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
** 
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with GNU gv; see the file COPYING.  If not, write to
** the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
** Boston, MA 02111-1307, USA.
** 
** Author:   Johannes Plass (plass@thep.physik.uni-mainz.de)
**           Department of Physics
**           Johannes Gutenberg-University
**           Mainz, Germany
**
**           Jose E. Marchesi (jemarch@gnu.org)
**           GNU Project
**
*/
#include "ac_config.h"


/*
#define MESSAGES
*/
#include "message.h"

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "paths.h"
#include INC_X11(Xos.h)
#include INC_X11(Intrinsic.h)

#include "types.h"
#include "error.h"
#include "main_resources.h"
#include "main_globals.h"

char* 
open_fail_error(errornumber,error_str,file_name,show)
  int errornumber;
  char *error_str;
  char *file_name;
  int show;
{
  char *m;

  if (!(m = strerror(errornumber))) m = "Unknown error";

  if (show) {
    fprintf(stderr,"%s: %s %s (%s)\n",gv_name,error_str,file_name,m);
    m = NULL;
  } else {
    char buf[1024];
    sprintf(buf,"%s %s: %s",error_str,file_name,m);
    m = XtNewString(buf);
  }
  ENDMESSAGE(open_fail_error_message)
  return(m);
}


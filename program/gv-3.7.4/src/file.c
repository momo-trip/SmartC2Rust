/*
**
** file.c
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
** Changes submitted by Maurizio Loreti distributed on the public
** domain:
**
**       - Code for handle bzip2 compressed files.
**
*/
#include "ac_config.h"

/*
#define MESSAGES
*/
#include "message.h"

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "paths.h"
#include INC_X11(Intrinsic.h)

#include "types.h"
#include "config.h"
#include "file.h"
#include "main_resources.h"
#include "main_globals.h"

#ifndef ENODATA
   #define ENODATA ENOMSG
#endif

/*############################################################*/
/* file_getDirOfPath */
/*############################################################*/

char *
file_getDirOfPath(char *path)
{
   char *dir=NULL;
   char *pos;

   BEGINMESSAGE(file_getDirOfPath)

   if (path) {
      dir=XtNewString(path);
      pos = strrchr(dir,'/');
      if (pos) { pos++; *pos='\0'; }
   }

#ifdef MESSAGES
   if (!dir) { INFMESSAGE (### Warning: returning NULL) }
   else      { INFSMESSAGE(returning, dir) }
#endif
   ENDMESSAGE(file_getDirOfPath)
   return(dir);
}

/*############################################################*/
/* file_locateFilename */
/*############################################################*/

char *
file_locateFilename(char *path)
{
   char *tmp=NULL;
   BEGINMESSAGE(file_locateFileName)
   if (path) {

         tmp = strrchr(path,'/');

      if (!tmp) tmp=path;
      else tmp++;
      INFSMESSAGE(found,tmp)
   }
   ENDMESSAGE(file_locateFilename)
   return(tmp);
}

/*############################################################*/
/* file_getTmpFilename */
/* provide some temporary file name */
/*############################################################*/

char *
file_getTmpFilename(const char *baseDirectory, const char *baseFilename, int *filed)
{
   char tempFilename[256];
   char *tempFilenameP;
   char tmpNameBuf[256];
   char *tmpName;
   char *tmpExt;
   char *pos;
   int len;

   BEGINMESSAGE(file_getTmpFilename)

   if (baseDirectory)
	pos = strrchr(baseDirectory, '/');
   else
	pos = NULL;
   if (pos) {
	len = pos - baseDirectory;
   } else {
	baseDirectory = app_res.scratch_dir;
	len = strlen(baseDirectory);
	if (len > 0 && baseDirectory[len-1] == '/')
		len--;
   }

   if (!baseFilename) baseFilename= ".";
   strcpy(tmpNameBuf,baseFilename);
   pos = file_locateFilename(tmpNameBuf);
   if (pos) tmpName = pos;
   else     tmpName = tmpNameBuf;

   pos = strrchr(tmpName,'.');
   if (pos) { *pos='\0'; tmpExt = pos+1; }
   else tmpExt = "";

   /* Limit filename to 39 characters (excluding dir and .tmp).
   This is required for VMS, but is also reasonable for Unix. */
   if (strlen(tmpName)+strlen(tmpExt)>23) {
      if (strlen(tmpExt)>11) tmpExt[11] = '\0';     /* allow .ps_page_nnn */
      if (strlen(tmpName)+strlen(tmpExt)>23) tmpName[23-strlen(tmpExt)] = '\0';
   }
   {
      int done = 0;
      int i=1;
      do {
         int fd, l;
	 mode_t oldumask;
         l = snprintf(tempFilename, sizeof(tempFilename),
			 "%.*s/gv_%s_%s.XXXXXX",
			 len, baseDirectory, tmpName, tmpExt);
	 if (l < 0 || l >= (int)sizeof(tempFilename) )
		 break;
         file_translateTildeInPath(tempFilename, sizeof(tempFilename));
	 oldumask = umask(0077);
	 fd = mkstemp(tempFilename);
	 umask(oldumask);
	 if (fd < 0)
		 break;
	 if (fd >= 0) {
	 	if (filed)
			 *filed = fd;
		 else
			 close(fd);
         	done = 1;
	 }
         i++;
      } while (!done && i <= 10000);
      if (!done) {
          ENDMESSAGE(file_getTmpFilename)
          return NULL;
      }
   }
   SMESSAGE(tempFilename)
   tempFilenameP = XtNewString(tempFilename);
   ENDMESSAGE(file_getTmpFilename)
   return(tempFilenameP);
}

/*############################################################*/
/* file_translateTildeInPath */
/* Replaces tilde in string by user's home directory. */
/*############################################################*/

void file_translateTildeInPath(char *path, size_t s)
{

   BEGINMESSAGE(file_translateTildeInPath)
   if (path[0] == '~' && (path[1] == '/' || path[1] == '\0')) {
      char *home = getenv("HOME");

      if (home != NULL) {
         size_t pl = strlen(path);
         size_t hl = strlen(home);
         if (pl + hl - 1 < s - 1) {
             memmove(path + hl, path + 1, pl - 1 + 1);
             memcpy(path, home, hl);
         }
      }
   }
   ENDMESSAGE(file_translateTildeInPath)
}

/*############################################################*/
/* file_fileIsDir */
/*############################################################*/

int file_fileIsDir(char *fn)
{
  struct stat s;
  int r=0;
  char *c;
  BEGINMESSAGE(file_fileIsNotUseful)
  if (fn) {
     c = strrchr(fn,'/');
     if (c && (!*(c+1) || isspace(*(c+1)))) r = 1;

     if (!r && !stat(fn,&s)  && (S_ISDIR(s.st_mode))) r=1;
  }
  IMESSAGE(r)
  ENDMESSAGE(file_fileIsDir)
  return(r);
}

/*############################################################*/
/* file_fileIsNotUseful */
/*############################################################*/

int
file_fileIsNotUseful(char *fn)
{
  struct stat s;
  int r=0;
  BEGINMESSAGE(file_fileIsNotUseful)
  if (!fn || stat(fn, &s))
    r = 1;
  else if (S_ISDIR(s.st_mode)) {
    r = 1;
    errno = EISDIR;
  } else if (s.st_size == 0) {
    r = 1;
#ifdef EFTYPE
    errno = EFTYPE;
#else
    errno = ENODATA;
#endif
  }
  IMESSAGE(r)
  ENDMESSAGE(file_fileIsNotUseful)
  return(r);
}

/*############################################################*/
/* file_pdfname2psname */
/* If the file ends in .pdf, change this to .ps.*/
/* Return pointer to temp copy if changed, else to input string. */
/*############################################################*/

char *
file_pdfname2psname(char *name)
{
  char *e;

  if (!name) 
    {
      return(name);
    }

  e = name+strlen(name);

  if ((e-name)-4 >= 0)
    {
      e -= 4;
      if (!strcasecmp(e,".pdf"))
	{
	  strcpy(e,".ps");
	}
    }

  return(name);
}


/*############################################################*/
/* file_getUsefulName */
/*############################################################*/

char *
file_getUsefulName(char *name)
{
  char *e,*c;
  String mext,ext;

  BEGINMESSAGE(file_getUsefulName)
  if (!name) {
    ENDMESSAGE(file_getUsefulName)
    return(name);
  }
  INFSMESSAGE(in,name)

  c = e = strrchr(name,'.');
  if (!e) {
     ENDMESSAGE(file_getUsefulName)
     return(name);
  }
  mext = ext = XtNewString(e);
  while (*ext) { *ext = tolower(*ext); ext++; }
  if      (!strncmp(mext,".gz",3))  ext = "";
  else if (!strncmp(mext,".bz2",4)) ext = "";
  else if (!strncmp(mext,".z",2))   ext = "";
  else if (!strncmp(mext,".ps",3))  ext = ".ps";
  else if (!strncmp(mext,".pdf",4)) ext = ".pdf";
  else                              ext = e;
  if (e == name && !(*ext)) {
    ENDMESSAGE(file_getUsefulName)
    return(name);
  }
  strcpy(mext,ext);
  ext=mext;
  while (*ext && *e) *ext++=*e++;
  strcpy(c,mext);
  XtFree(mext);
  INFSMESSAGE(out,name)
  ENDMESSAGE(file_getUsefulName)
  return(name);
}


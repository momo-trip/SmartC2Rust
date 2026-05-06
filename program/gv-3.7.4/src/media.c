/*
**
** media.c
**
** Copyright (C) 1997 Johannes Plass
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

#include <stdio.h>
#include <math.h>

#include "paths.h"
#include INC_X11(Intrinsic.h)
#include INC_XMU(SysUtil.h)

#include "types.h"
#include "config.h"
#include "types.h"
#include "options.h"
#include "media.h"

/*##################################################
  media_freeMedias
##################################################*/

void media_freeMedias(medias)
  Media *medias;
{
  int i=0;
  BEGINMESSAGE(media_freeMedias)
  while (medias[i]) {
    XtFree(medias[i]->name);
    XtFree((XtPointer)medias[i]);
    i++;
  }
  XtFree((XtPointer)medias);
  ENDMESSAGE(media_freeMedias)
}

/*##################################################
  media_parseMedias
##################################################*/

static Media media_mallocMedia(void)
{
  Media media;
  media = (Media) XtMalloc(sizeof(MediaStruct));
  memset((void*)media ,0,sizeof(MediaStruct));
  return media;
}

Media *media_parseMedias(s)
  char *s;
{
  char *c,*nl;
  Media *medias,*mmedias,media;
  int i,n,have_media=0,have_used_media=0,used,w,h;
  char name[100];

  BEGINMESSAGE(media_parseMedias)
  if (!s) s = "";
  s =options_squeezeMultiline(s);
  for (n=1,c=s; (c = strchr(c,'\n')); n++, c++);
  INFIMESSAGE(number of medias,n)
  mmedias = medias = (Media*) XtMalloc((n+3)*sizeof(Media));
  media = media_mallocMedia();
  media->name = XtNewString("BBox");
  media->width= 0;
  media->height= 0;
  media->used= 1;
  *medias++ = media;
  c=s;
  if (*s) while (n>0) {
    nl = strchr(c,'\n'); 
    if (nl) *nl='\0';
    name[0]='\0';
    used = 1;
    while (*c && (*c == '#' || *c == '!')) { used=0; c++; }
    i=sscanf(c," %[^,] , %d %d ",name,&w,&h);
    if (i==3 && w>0 && h>0) {
      media = media_mallocMedia();
      media->name = XtNewString(name);
      media->width= w;
      media->height= h;
      media->used= used;
      if (used) have_used_media=1;
      have_media = 1;
      INFSMESSAGE(found media,media->name)
      IIMESSAGE(media->width,media->height)
      *medias++ = media;
    }
    n--;
    if (!nl) break;
    c=++nl;
  }
  if (!have_media) {
    media = media_mallocMedia();
    media->name = XtNewString("A4");
    media->width= 595;
    media->height= 842;
    media->used= 1;
    *medias++ = media;
  }
  if (!have_used_media) mmedias[1]->used=1;
  *medias = (Media) NULL;

  XtFree(s);
  ENDMESSAGE(media_parseMedias)
  return(mmedias);
}

/*##################################################
  media_numMedias
##################################################*/

int media_numMedias(medias)
  Media *medias;
{
  int i;
  for (i=1; medias[i]; i++);
  return i;
}



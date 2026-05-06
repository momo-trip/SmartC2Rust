/*
**
** doc_misc.c
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
*/
#include "ac_config.h"

/*
#define MESSAGES
*/
#include "message.h"

#include <stdlib.h>
#include <stdio.h>
#include <gv_signal.h>

#include "paths.h"
#include INC_X11(Intrinsic.h)
#include INC_XMU(CharSet.h)
#include "Ghostview.h"
#include "main_resources.h"

#include "types.h"
extern Media* gv_medias;
#include "ps.h"
#include "doc_misc.h"

#include "resource.h"

/*##################################################################*/
/* doc_scanFile */
/*##################################################################*/

int
doc_scanFile(fPP,docP,filename,filename_raw,filename_dscP,cmd_scan_pdf,filename_uncP,cmd_uncompress,scanstyle,gv_gs_safeDir)
   FILE ** fPP;
   Document *docP;
   String filename;
   String filename_raw;
   String *filename_dscP;
   String cmd_scan_pdf;
   String *filename_uncP;
   String cmd_uncompress;
   int scanstyle;
   int gv_gs_safeDir;
{
   Document d;
   int i,ret;

   BEGINMESSAGE(doc_scanFile)
   d = (Document)NULL;
   ret = 0;
   if (*fPP && filename) 
     d = psscan(fPP,filename,filename_raw,filename_dscP,cmd_scan_pdf,filename_uncP,cmd_uncompress,scanstyle,gv_gs_safeDir);
   if (d) {
      d->labels_useful=0;
      d->structured   =0;
      if ((!d->epsf && d->numpages>0) || (d->epsf && d->numpages>1)) {
         ret = d->structured = 1;
         if (d->numpages == 1) d->labels_useful=1;
         else for (i = 1; i < d->numpages; i++)
            if (strcmp(d->pages[i-1].label,d->pages[i].label)) { d->labels_useful = 1; break; }
      }
   }
   *docP = d;

#  ifdef MESSAGES
     if (!d) { INFMESSAGE(unsuccessful) }
     else {
         INFMESSAGE(successful)
         if (d->structured)    { INFMESSAGE(structured)    } else { INFMESSAGE(not structured)    }
         if (d->labels_useful) { INFMESSAGE(labels useful) } else { INFMESSAGE(labels not useful) }
     }
#  endif
   ENDMESSAGE(doc_scanFile)
   return ret;
}

/*##################################################################*/
/* doc_putPageInRange */
/*##################################################################*/

int
doc_putPageInRange(d,pagenumber)
   Document d;
   int pagenumber;
{
   BEGINMESSAGE(doc_putPageInRange)
   if (d && d->structured) {
      if (pagenumber >= d->numpages) pagenumber = d->numpages-1;
      if (pagenumber < 0) pagenumber = 0;
   } else pagenumber = 0;
   ENDMESSAGE(doc_putPageInRange)
   return pagenumber;
}    

/*############################################################*/
/* doc_mediaIsOk */
/*############################################################*/

int 
doc_mediaIsOk (d,pagenumber,m)
  Document d;
  int pagenumber;
  int m;
{
  int r=1;
  int bbox = d ? d->nummedia : 0;
  BEGINMESSAGE(doc_mediaIsOk)
  if (m==bbox) {
    int l;
    if (doc_boundingBoxOfPage(d,pagenumber,&l,&l,&l,&l) == MEDIA_ID_INVALID)
      r = 0;
  } else if (m==MEDIA_ID_INVALID)
      r = 0;
  ENDMESSAGE(doc_mediaIsOk)
  return(r);
}

/*############################################################*/
/* doc_boundingBoxOfPage */
/*############################################################*/

int
doc_boundingBoxOfPage(d,pagenumber,llxP,llyP,urxP,uryP)
  Document d;
  int pagenumber;
  int *llxP,*llyP,*urxP,*uryP;
{
  int *dbb = NULL;
  int retry;

  BEGINMESSAGE(doc_boundingBoxOfPage)

  if (!d) {
    INFMESSAGE(no document) ENDMESSAGE(doc_preferredMediaOfPage)
    return(MEDIA_ID_INVALID);
  }

  retry=1;
# define BB_VALID (dbb[URX]>dbb[LLX] && dbb[URY]>dbb[LLY])
  if (d->structured && 0<=pagenumber && pagenumber<d->numpages)
    { dbb = d->pages[pagenumber].boundingbox;  if BB_VALID retry=0; }
  if (retry && d->structured)
    { dbb = d->default_page_boundingbox; if BB_VALID retry=0; }
  if (retry)
    { dbb = d->boundingbox;              if BB_VALID retry=0; }
# undef BB_VALID
  if (!retry) {
    *llxP=dbb[LLX]; *llyP=dbb[LLY]; *urxP=dbb[URX]; *uryP=dbb[URY];
    INFMESSAGE(bounding box)
    ENDMESSAGE(doc_boundingBoxOfPage)
    return(d->nummedia);
  }
  INFMESSAGE(MEDIA_ID_INVALID)
  ENDMESSAGE(doc_boundingBoxOfPage)
  return(MEDIA_ID_INVALID);
}

/*############################################################*/
/* doc_preferredMediaOfPage */
/*############################################################*/

int
doc_preferredMediaOfPage(d,pagenumber,llxP,llyP,urxP,uryP)
  Document d;
  int pagenumber;
  int *llxP,*llyP,*urxP,*uryP;
{
# define BB_VALID (dbb[URX]>dbb[LLX] && dbb[URY]>dbb[LLY])
  int *dbb = NULL;
  Media media = NULL,dm;
  int m=MEDIA_ID_INVALID;
  int found,i,j;

  BEGINMESSAGE(doc_preferredMediaOfPage)

  if (!d) {
    INFMESSAGE(no document) ENDMESSAGE(doc_preferredMediaOfPage)
    return(MEDIA_ID_INVALID);
  }

  found=0;
  if (!found && d->structured && 0<=pagenumber && pagenumber<d->numpages)
    { media = (Media) d->pages[pagenumber].media;  if (media) found=1; }
  if (!found)
    { media = (Media) d->default_page_media; if (media) found=1; }
  if (d->structured && 0<=pagenumber && pagenumber<d->numpages)
    { dbb = d->pages[pagenumber].boundingbox;  if BB_VALID found=-1; }
  if (!found && d->structured)
    { dbb = d->default_page_boundingbox; if BB_VALID found=-1; }
  if (!found)
    { dbb = d->boundingbox;              if BB_VALID found=-1; }
# undef BB_VALID

  /* try to map bounding box to standard or document pagemedia */
  if (found==-1 && dbb[LLX]==0 && dbb[LLY]==0) {
    for (dm=(Media) d->media,i=0; !media && i<d->nummedia; i++,dm++)
      if (dm->used &&
          ((dm->width==dbb[URX] || dm->width+1==dbb[URX]) &&
           (dm->height==dbb[URY] || dm->height+1==dbb[URY]))) media=dm;
    for (j=0; gv_medias[j] && !media ; j++) {
       dm = gv_medias[j];
       if (dm->used==1 &&
           ((dm->width==dbb[URX] || dm->width+1==dbb[URX]) &&
            (dm->height==dbb[URY] || dm->height+1==dbb[URY]))) media=dm;
    }
    if (media) found=1;
  }

  if (found==-1) {
    *llxP=dbb[LLX]; *llyP=dbb[LLY]; *urxP=dbb[URX]; *uryP=dbb[URY];
    INFMESSAGE(bounding box)
    m=d->nummedia;
  } else if (found==1) {
    for (dm=(Media)d->media,i=0; i<d->nummedia; i++,dm++)
      if (media==dm) { m = i; break; }
    if (m==MEDIA_ID_INVALID) for (i=0; gv_medias[i] ; i++)
      if (media==gv_medias[i]) { m = i+d->nummedia; break; }
    if (m!=MEDIA_ID_INVALID) {
      INFIMESSAGE(doc prefers media,m)
      *llxP=*llyP=0; *urxP=media->width-1; *uryP=media->height-1;
    }
  }
  INFIMESSAGE(returning media number,m)
  ENDMESSAGE(doc_preferredMediaOfPage)
  return(m);
}

/*##################################################################*/
/* doc_preferredOrientationOfPage */
/*##################################################################*/

int
doc_preferredOrientationOfPage(d,page)
   Document d;
   int page;
{
   int o;

   BEGINMESSAGE(doc_preferredOrientationOfPage)
   o  = O_UNSPECIFIED;
   if (d) {
      if (d->structured && 0<=page && page<d->numpages && d->pages[page].orientation != O_UNSPECIFIED)
         o = d->pages[page].orientation;
      else if (d->default_page_orientation != O_UNSPECIFIED)
         o = d->default_page_orientation;
      else if (d->orientation != O_UNSPECIFIED)
         o = d->orientation;
   }
   ENDMESSAGE(doc_preferredOrientationOfPage)
   return(o);
}

/*############################################################*/
/* doc_convStringToPage */
/*############################################################*/

int
doc_convStringToPage(d,pageLabel)
   Document d;
   String pageLabel;
{
   int i,j;
   int page;

   BEGINMESSAGE(doc_convStringToPage)
   page=-1;
   if (pageLabel && d && d->labels_useful) for (i=0; i<d->numpages; i++) {
      if (d->pageorder == DESCEND) j = d->numpages-1-i;
      else                         j = i;
      if (!strcmp(pageLabel,d->pages[j].label)) { page=i; break; }
   }
      INFMESSAGE(1)
   if (page<0 && pageLabel) page=atoi(pageLabel)-1;
   if (page<0) page=0;
   IMESSAGE(page)
   ENDMESSAGE(doc_convStringToPage)
   return(page);
}

/*############################################################*/
/* doc_convDocOrientToXtOrient */
/*############################################################*/

XtPageOrientation
doc_convDocOrientToXtOrient(orientation,swapLandscape)
   int orientation;
   int swapLandscape;
{
   XtPageOrientation ret;

   BEGINMESSAGE(doc_convDocOrientToXtOrient)

   switch (orientation) {
   case O_PORTRAIT:
      INFMESSAGE(portrait)
      ret= XtPageOrientationPortrait;
      break;
   case O_UPSIDEDOWN:
      INFMESSAGE(upsidedown)
      ret = XtPageOrientationUpsideDown;
      break;
   case O_LANDSCAPE:
     if (swapLandscape) {INFMESSAGE(seascape)  ret = XtPageOrientationSeascape; }
     else               {INFMESSAGE(landscape) ret = XtPageOrientationLandscape;}
      break;
   case O_SEASCAPE:
     if (swapLandscape) {INFMESSAGE(landscape)ret = XtPageOrientationLandscape;}
      else              {INFMESSAGE(seascape)ret = XtPageOrientationSeascape;}
      break;
   case O_UNSPECIFIED:
      INFMESSAGE(unspecified)
      ret = XtPageOrientationUnspecified;
      break;
   default:
      INFMESSAGE(doc_convDocOrientToXtOrient: unknown orientation)
      ret = XtPageOrientationUnspecified;
      break;
   }
   ENDMESSAGE(doc_convDocOrientToXtOrient)

   return ret;
}

/*############################################################*/
/* doc_convStringToDocOrient */
/*############################################################*/

int
doc_convStringToDocOrient(ostr)
   String ostr;
{
   int o=O_PORTRAIT;
   BEGINMESSAGE(doc_convStringToDocOrient)
   if (ostr) {
SMESSAGE(ostr)
#     define IS_STR(aaa,bbb) (!XmuCompareISOLatin1(aaa,bbb))
      if      IS_STR(ostr,"Automatic")      { INFMESSAGE(is O_AUTOMATIC)      o = O_AUTOMATIC;  }
      else if IS_STR(ostr,"Portrait")       { INFMESSAGE(is O_PORTRAIT)       o = O_PORTRAIT;   }
      else if IS_STR(ostr,"Landscape")      { INFMESSAGE(is O_LANDSCAPE)      o = O_LANDSCAPE;  }
      else if IS_STR(ostr,"Seascape")       { INFMESSAGE(is O_SEASCAPE)       o = O_SEASCAPE;   }
      else if IS_STR(ostr,"Upside-Down")    { INFMESSAGE(is O_UPSIDEDOWN)     o = O_UPSIDEDOWN; }
      else if IS_STR(ostr,"Swap-Landscape") { INFMESSAGE(is O_SWAP_LANDSCAPE) o = O_SWAP_LANDSCAPE; }
#     undef IS_STR
   }
   ENDMESSAGE(doc_convStringToDocOrient)
   return o;
}

/*############################################################*/
/* doc_convStringToPageMedia */
/*############################################################*/

int
doc_convStringToPageMedia(d,mstr)
   Document d;
   String mstr;
{
   int media = MEDIA_ID_INVALID;
   int i;

   BEGINMESSAGE(doc_convStringToPageMedia)
   /*### automatic pagemedia evaluation */
   if (!XmuCompareISOLatin1(mstr,"Automatic")) media = MEDIA_ID_AUTO;
   /*### document pagemedia */
   if (media==MEDIA_ID_INVALID  && d && d->nummedia) {
      for (i = 0; i<d->nummedia; i++) 
          if (!XmuCompareISOLatin1(mstr,d->media[i].name)) media = i;
   }
   /*### standard pagemedia */
   if (media==MEDIA_ID_INVALID) {
      int num_doc_papersizes=0;
      if (d) num_doc_papersizes = d->nummedia;
      for (i=0; gv_medias[i]; i++)
	  if (gv_medias[i]->used && !XmuCompareISOLatin1(mstr,gv_medias[i]->name)) media = num_doc_papersizes+i;
   }
   IMESSAGE(media)
   ENDMESSAGE(doc_convStringToPageMedia)
   return media;
}

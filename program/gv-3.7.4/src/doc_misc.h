/*
**
** doc_misc.h
**
** Copyright (C) 1996-1997 Johannes Plass
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
    
#ifndef _GV_DOC_MISC_H_
#define _GV_DOC_MISC_H_

extern int doc_scanFile (
   FILE **, Document*, String, String, String*, String, String*, String,
   int,     /* scanstyle */
   int     /* gv_gs_safeDir */
);
extern int doc_putPageInRange (Document, int);
extern int doc_mediaIsOk (Document, int, int);
extern int doc_boundingBoxOfPage (Document, int, int*, int*, int*, int*);
extern int doc_preferredMediaOfPage (Document, int, int*, int*, int*, int*);
extern int doc_preferredOrientationOfPage (Document, int);
extern int doc_convStringToPage (Document, String);
extern XtPageOrientation doc_convDocOrientToXtOrient (int, int);
extern int doc_convStringToDocOrient (String);
extern int doc_convStringToPageMedia (Document, String);

#endif /* _GV_DOC_MISC_H_ */

/*
**
** resource.h
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

#ifndef _GV_RESOURCE_H_
#define _GV_RESOURCE_H_

extern void          resource_freeData (
void
);

extern XrmDatabase resource_buildDatabase (
  Display *,
  char *,
  char *,
  int *,
  char **
);

extern void resource_putResource (XrmDatabase *,
				  char *,
				  char *,
				  char *);

extern char *        resource_getResource (
   XrmDatabase,
   char *,
   char *,
   char *,
   char *
);

extern int           resource_checkGeometryResource (
  XrmDatabase *,
  char *,
  char *
);

extern int           resource_checkResources (
  char *,
  char *,
  char *
);

extern char *        resource_userDefaultsFile (
void
);

extern String orientations[5];
extern String popupVerb[5];
extern String automaticLabel;
extern String confirm_quit_styles[4];
extern String title_styles[4];
extern String saveCurrentPageLabel;
extern String saveMarkedPagesLabel;
extern String saveDocumentLabel;
extern String saveAsPDFLabel;
extern String openFileLabel;
extern String passwordPromptLabel;
extern String passwordRequiredLabel;
extern String quitConfirmLabel;
extern String putTexCommandLabel, texCommandLabel;
extern String stillInProgressLabel;
extern String execOfFailedLabel;
extern String copyrightTranslationLabel;

#endif /* _GV_RESOURCE_H_ */

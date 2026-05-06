/*
**
** versionp.h
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
**           Department of Physicss
**           Johannes Gutenberg-University
**           Mainz, Germany
**
**           Jose E. Marchesi (jemarch@gnu.org)
**           GNU Project
**
*/

#ifndef _GV_VERSIONP_H_
#define _GV_VERSIONP_H_

#include "version.h"

#define  VERSION_SHELL_TITLE	"gv Copyright Information"
#define  VERSION_TOPLEVEL	toplevel       			/* the Application Shell   */
#define  VERSION_APPLIC_CONTEXT	app_con  			/* the Application Context */

char *versionIdentification[] = {
        "gv 3.7.4",
        "March 2013",
	0
};

/*
   declare it writable due to problems with sscanf() in main_versionIsCompatible() 
   on HP/UX 9.07
   [suggested by Christian Illinger (illinger@lepsi.in2p3.fr)]
*/
char versionResource[] = "gv 3.6.7.90";
char versionCompatibility[] = "gv 3.6.7.90";
                                            
char * copyright = "\
 gv -- An X11 user interface for Ghostscript. \n\
 \n\
 Copyright (C) 1992-1997  Johannes Plass, Timothy O. Theisen  \n\
 Copyright (C) 2004,2005,2006,2007 Jose E. Marchesi \n\
 Copyright (C) 2007,2008,2009,2010 Markus Steinborn \n\
 \n\
 This program is free software; you can redistribute it and/or modify \n\
 it under the terms of the GNU General Public License as published by \n\
 the Free Software Foundation; either version 3 of the License, or \n\
 (at your option) any later version. \n\
 \n\
 This program is distributed in the hope that it will be useful, \n\
 but WITHOUT ANY WARRANTY; without even the implied warranty of \n\
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the \n\
 GNU General Public License for more details. \n\
 \n\
 You should have received a copy of the GNU General Public License\n\
 along with GNU gv; see the file COPYING.  If not, see\n\
 <http://www.gnu.org/licenses/>.";

char *author[] = {
"\
   GNU Maintainer:      Markus Steinborn (gnugv_maintainer@yahoo.de)\n\
   Original Author:     Johannes Plass\n\
\n\
   %s\n\
\n\
   See the `AUTHORS' file for a complete contributors list\
",
"\
 Please send error reports to:  bug-gv@gnu.org \n\
",
0 
};

#endif /* _GV_VERSIONP_H_ */

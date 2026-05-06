/*
**
** types.h
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

#ifndef _GV_TYPES_H_
#define _GV_TYPES_H_


#include <sys/types.h>


/*##############################################
  options
##############################################*/

typedef void (*OptionPopupCreateProc)(void);

typedef void (*OptionPopupUpdateProc)(void);

typedef struct OptionPopupStruct_ {
  Widget popup;
  OptionPopupCreateProc create;
  OptionPopupUpdateProc update;
  Boolean visible;
} OptionPopupStruct, *OptionPopup;

/*##############################################
  miscmenu
##############################################*/

typedef struct MiscMenuEntryStruct_ {
  char  *name;
  XtCallbackProc cb;
  XtPointer client_data;
  int sensitivity_type;
} MiscMenuEntryStruct, *MiscMenuEntry;

/*##############################################
  magmenu
##############################################*/

typedef struct MagMenuEntryStruct_ {
  char  *name;
  float scale;
  int   is_base;
  int   is_center;
} MagMenuEntryStruct, *MagMenuEntry;

/*##############################################
  media
##############################################*/

typedef struct documentmedia {
  char *name;
  int  width;
  int  height;
  int  used;
} MediaStruct, *Media;

/*##############################################
  scale
##############################################*/

typedef struct ScaleStruct_ {
  char  *name;
  float scale;
  int   is_base;
  int   is_center;
} ScaleStruct, *Scale;

#define SCALE_IS_REAL_BASED   1
#define SCALE_IS_PIXEL_BASED  2

/*##############################################
  process
##############################################*/

typedef void (*ProcessNotifyProc)(XtPointer /* data */, int /* type */);

#define PROCESS_NONE	0
#define PROCESS_NOTIFY	1
#define PROCESS_KILL	2

typedef struct ProcessDataStruct_ *ProcessData;

typedef struct ProcessDataStruct_ {
   int               type;
   pid_t             pid;
   XtIntervalId      timer;
   ProcessNotifyProc notify_proc;
   XtPointer         data;
   ProcessData       next;
   Widget            menuentry;
   String            name;
} ProcessDataStruct;

/*##############################################
  Definitions
##############################################*/

#define REQUEST_OPTION_CHANGE		-10
#define REQUEST_TOGGLE_RESIZE		-9
#define REQUEST_REDISPLAY		-8
#define REQUEST_SETUP			-7
#define REQUEST_NEW_SCALE		-6
#define REQUEST_NEW_PAGEMEDIA		-5
#define REQUEST_NEW_ORIENTATION		-4
#define REQUEST_REOPEN			-3
#define REQUEST_NEW_FILE		-2
#define NO_CURRENT_PAGE			-1 /* NO_CURRENT_PAGE must be -1 */

#define EYEGUIDE_DRAW   (1<<0)
#define EYEGUIDE_REMOVE (1<<1)
#define EYEGUIDE_RESET  (1<<2)

#define SPM_ALL                         (1<<0)
#define SPM_EVEN                        (1<<1)
#define SPM_ODD                         (1<<2)
#define SPM_CURRENT			(1<<3)
#define SPM_TOGGLE			(1<<4)
#define SPM_MARK			(1<<5)
#define SPM_UNMARK			(1<<6)

#define PAGE_MODE_INVALID		0
#define PAGE_MODE_ALL			(1<<0)
#define PAGE_MODE_MARKED		(1<<1)
#define PAGE_MODE_CURRENT		(1<<2)

#define PAGE_NONE			0
#define PAGE_LEFT			(1<<0)
#define PAGE_RIGHT			(1<<1)
#define PAGE_UP				(1<<2)
#define PAGE_DOWN			(1<<3)
#define PAGE_TOP			(1<<4)
#define PAGE_BOTTOM			(1<<5)
#define PAGE_LEFTEDGE			(1<<6)
#define PAGE_RIGHTEDGE			(1<<7)
#define PAGE_CENTER			(1<<8)
#define PAGE_REDISPLAY			(1<<9)
#define PAGE_PAGE			(1<<10)
#define PAGE_H				(1<<11)
#define PAGE_V				(1<<12)
#define PAGE_NOT       			(1<<13)

#define CHECK_FILE_DATE			(1<<0)
#define CHECK_FILE_VERSION		(1<<1)

#define SCROLL_MODE_NONE                0
#define SCROLL_MODE_PANNER              (1<<0)
#define SCROLL_MODE_GHOSTVIEW           (1<<1)
#define SCROLL_MODE_SCROLLBAR           (1<<2)

#define FILE_TYPE_PS			0
#define FILE_TYPE_PDF			1

#define SAVE_MODE_NONE                  0
#define SAVE_MODE_FILE                  1
#define SAVE_MODE_PRINTER               2

#define PROCESS_NONE   0
#define PROCESS_NOTIFY 1
#define PROCESS_KILL   2

#define PROCESS_MENU_NONE       0
#define PROCESS_MENU_HIDE       1
#define PROCESS_MENU_SHOW       2
#define PROCESS_MENU_ADD_ENTRY  3
#define PROCESS_MENU_DEL_ENTRY  4
#define PROCESS_MENU_PROGRESS   5

/* orientations below match the definitions in ps.h */
#define O_INVALID         -1000
#define O_UNSPECIFIED     0
#define O_PORTRAIT        1
#define O_LANDSCAPE       2
#define O_SEASCAPE        3
#define O_UPSIDEDOWN      4
#define O_AUTOMATIC       8
#define O_SWAP_LANDSCAPE  1000

#define MEDIA_ID_INVALID  -3
#define MEDIA_ID_AUTO     -2

#define SCALE_REL         (1<<8)
#define SCALE_ABS         (1<<9)
#define SCALE_BAS         (1<<10)
#define SCALE_MIN         (1<<11)
#define SCALE_VAL         (~(SCALE_REL|SCALE_ABS|SCALE_BAS|SCALE_MIN))

#endif /* _GV_TYPES_H_ */


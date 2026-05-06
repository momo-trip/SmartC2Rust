/*
**
** FileSel.c
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

#include <stdlib.h> /* for malloc etc.*/
#include <ctype.h>  /* for toupper    */
#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

#include <inttypes.h>
    /* Damn it, don't ever use getwd with stupid linux ###jp### */
#define getwd(aaa) getcwd((aaa),(FS_MAXNAMLEN-2))    

#include "paths.h"
#include INC_X11(Xlib.h)
#include INC_X11(Xos.h)
#include INC_X11(IntrinsicP.h)
#include INC_X11(StringDefs.h)
#include INC_XMU(Misc.h)
#include INC_XMU(CharSet.h)		/* for XmuCompareISOLatin1() */
#include INC_XAW(XawInit.h)
#include INC_XAW(AsciiText.h)
#include INC_XAW(Scrollbar.h)
#include INC_XAW(SimpleMenu.h)
#include INC_XAW(SmeBSB.h)
#include "Button.h"
#include "Clip.h"
#include "Frame.h"
#include "FileSelP.h"
#include "MButton.h"
#include "Vlist.h"
#include "VlistP.h"

/*####################################################################
   OS dependant Definitions
####################################################################*/


#define FS_MAXNAMLEN 255
#define DIR_SEPARATOR_STRING "/"
#define DIR_SPECIFICATION_START_STRING ""
#define DIR_SPECIFICATION_END_STRING ""

#define CHANGE_TO_HEAD_OF_DIR_SPEC(path) {	\
              char*p=path;			\
              *p='/'; p++; *p='\0';		\
           }

#define ONE_STEP_UP ".."		/* chdir argument to make one step up in the directory tree. */
#define HOME getenv("HOME")	        /* chdir argument to go home (the login directory) */


/*####################################################################
   Initializations
####################################################################*/

static String unknownList[] = {"<cannot enter directory>",NULL};
static String cannotopenList[] = {"<cannot read directory>",NULL};

static void FS_textfieldFocusAction(Widget,XEvent*,String*,Cardinal*);
static void FS_textfieldBackSpaceAction(Widget,XEvent*,String*,Cardinal*);
static void FS_textfieldDeleteAction(Widget,XEvent*,String*,Cardinal*);
static void FS_listAction(Widget,XEvent*,String*,Cardinal*);
static void FS_preferButtonAction(Widget,XEvent*,String*,Cardinal*);

static XtActionsRec file_selectionActionsTable[] = {
       { "FS_textfieldFocusAction",  FS_textfieldFocusAction },
       { "FS_textfieldBackSpaceAction", FS_textfieldBackSpaceAction },
       { "FS_textfieldDeleteAction", FS_textfieldDeleteAction },
       { "FS_preferButton",          FS_preferButtonAction },
       { "List",      FS_listAction }
};

static String list_translations =
"#replace\n\
<EnterWindow>:		List(highlight)\n\
<LeaveWindow>:		List(unhighlight)\n\
~Button2 <Btn1Down>:	List(set) List(scrollon)\n\
~Button2 <Btn1Motion>:	List(scroll)\n\
~Button2 <Btn1Up>:	List(reset) List(notify) List(scrolloff) List(highlight)\n\
~Button1 <Btn2Down>:	List(scrollon)\n\
	 <Btn2Motion>:	List(scroll,0,2.0)\n\
~Button1 <Btn2Up>:	List(page) List(scrolloff) List(highlight)\n\
~Button1 <Btn3Down>:	List(scrollon)\n\
	 <Btn3Motion>:	List(scroll,0,2.0)\n\
~Button1 <Btn3Up>:	List(page) List(scrolloff) List(highlight)\n\
<MotionNotify>:		List(highlight)\
";

static String curlist_translations =
"#replace\n\
<EnterWindow>:		List(highlight)\n\
<LeaveWindow>:		List(unhighlight)\n\
~Button2 <Btn1Down>:	List(scrollon) List(set)\n\
~Button2 <Btn1Motion>:	List(scroll)\n\
~Button2 <Btn1Up>:	List(reset) List(notify) List(scrolloff) List(highlight)\n\
~Button1 <Btn2Down>:	List(scrollon)\n\
	 <Btn2Motion>:	List(scroll,0,2.0)\n\
~Button1 <Btn2Up>:	List(page) List(scrolloff) List(highlight)\n\
~Button1 <Btn3Down>:	List(scrollon)\n\
	 <Btn3Motion>:	List(scroll,0,2.0)\n\
~Button1 <Btn3Up>:	List(page) List(scrolloff) List(highlight)\n\
<MotionNotify>:		List(highlight)\
";

static String TextField_translations =
"#override\n\
<Key>Down:	no-op()\n\
<Key>Up:	no-op()\n\
<Key>Linefeed: 	no-op()\n\
Ctrl<Key>J: 	no-op()\n\
Ctrl<Key>M: 	no-op()\n\
Ctrl<Key>N: 	no-op()\n\
Ctrl<Key>O: 	no-op()\n\
Ctrl<Key>P: 	no-op()\n\
Ctrl<Key>R: 	no-op()\n\
Ctrl<Key>S: 	no-op()\n\
Ctrl<Key>V: 	no-op()\n\
Ctrl<Key>Z: 	no-op()\n\
Meta<Key>V: 	no-op()\n\
Meta<Key>Z: 	no-op()\n\
<Key>BackSpace: FS_textfieldBackSpaceAction()\n\
<Key>Delete: 	FS_textfieldDeleteAction()\n\
<Key>Right: 	forward-character()\n\
<Key>Left: 	backward-character()\n\
<Key>Return: 	no-op()\n\
<Key>Tab: 	FS_preferButton(next)\n\
<Btn1Down>:	FS_textfieldFocusAction() select-start()\n\
<Btn3Down>:	FS_textfieldFocusAction() extend-start()\
"; 

#if 0
#define FILE_SELECTION_LAYOUT \
"\
"
#endif

/*####################################################################
   Macros and Definitions
####################################################################*/

/* general Xt Macros */

#ifdef MIN
#   undef MIN
#endif
#define MIN(_a_,_b_) (_a_)<(_b_)?(_a_):(_b_)
#ifdef MAX
#   undef MAX
#endif
#define MAX(_a_,_b_) (_a_)>(_b_)?(_a_):(_b_)

#define USE_Arg(num)  Arg args[num]; Cardinal argn = 0

#define ADD_Callback(widget,proc) XtAddCallback((Widget)(widget),XtNcallback,(proc),NULL)
#define ADD_Callback_Data(widget,proc,data) XtAddCallback((Widget)(widget),XtNcallback,(proc),(data))
#define ADD_Widget(name,class,parent) XtCreateManagedWidget((name),class,(Widget)(parent),NULL,(Cardinal)0)  
#define ADD_PopupShell(name,class,parent) XtCreatePopupShell((name),class,(Widget)(parent),NULL,(Cardinal)0)

#define ADD_Widget_Arg(name,class,parent) XtCreateManagedWidget((name),class,(parent),args,argn)
#define RESET_Arg argn=0
#define GOT_Arg   (argn)
#define SET_Arg(name,value)		\
	   XtSetArg(args[argn],(name),(value));	argn++

#define SET_Values(widget)   XtSetValues((widget),args,argn)
#define SET_Value(widget,name,value)	\
           RESET_Arg;			\
           SET_Arg((name),(value));	\
           SET_Values((widget))

#define GET_Values(widget)   XtGetValues((Widget)(widget),args,argn)
#define GET_Value(widget,name,value)	\
           RESET_Arg;			\
           SET_Arg((name),(value));	\
           GET_Values((widget))

#define streq(a,b) (strcmp((a),(b))==0)
#define resource(name) (appResources->name)

#define SCROLL_SCROLLPROC 1
#define SCROLL_JUMPPROC 2
#define SCROLL_CLIPREPORT 3

/* FileSelection specific Macros */

#define FS_WIDGET		FileSelectionWidget fs = (FileSelectionWidget)
#define FS_FILE_SELECTION       fs

#define FS_RESCANBUTTON 	fs->file_selection.rescanbuttonFS
#define FS_FILTERSBUTTON	fs->file_selection.filtersbuttonFS
#define FS_FILTERSMENU		fs->file_selection.filtersmenuFS
#define FS_DIRSBUTTON		fs->file_selection.dirsbuttonFS
#define FS_DIRSMENU		fs->file_selection.dirsmenuFS
#define FS_BUTTON1		fs->file_selection.button1FS
#define FS_BUTTON2		fs->file_selection.button2FS
#define FS_BUTTON3		fs->file_selection.button3FS
#define FS_BUTTON4		fs->file_selection.button4FS

#define FS_PATHFRAME		fs->file_selection.pathframeFS
#define FS_PATH			fs->file_selection.pathFS
#define FS_FILTERFRAME		fs->file_selection.filterframeFS
#define FS_FILTER		fs->file_selection.filterFS
#define FS_OLD_TEXTFIELD	fs->file_selection.old_textfieldFS
#define FS_TOPSCROLL		fs->file_selection.topscrollFS
#define FS_TOPFRAME		fs->file_selection.topframeFS
#define FS_TOPCLIP		fs->file_selection.topclipFS
#define FS_TOPAAA		fs->file_selection.topaaaFS
#define FS_TOPLIST		fs->file_selection.toplistFS
#define FS_CURSCROLL		fs->file_selection.curscrollFS
#define FS_CURFRAME		fs->file_selection.curframeFS
#define FS_CURCLIP		fs->file_selection.curclipFS
#define FS_CURAAA		fs->file_selection.curaaaFS
#define FS_CURLIST		fs->file_selection.curlistFS
#define FS_SUBSCROLL		fs->file_selection.subscrollFS
#define FS_SUBFRAME		fs->file_selection.subframeFS
#define FS_SUBCLIP		fs->file_selection.subclipFS
#define FS_SUBAAA		fs->file_selection.subaaaFS
#define FS_SUBLIST		fs->file_selection.sublistFS

#define TOPDIR			fs->file_selection.topdir
#define TOPDIR_ALLOC		fs->file_selection.topdir_alloc
#define TOPDIR_ENTRIES  	fs->file_selection.topdir.num_of_entries
#define TOPDIR_ENTRY(num) 	fs->file_selection.topdir.entry[(num)]
#define TOPDIR_LIST		fs->file_selection.topdir.entry

#define CURDIR			fs->file_selection.curdir
#define CURDIR_ALLOC		fs->file_selection.curdir_alloc
#define CURDIR_ENTRIES  	fs->file_selection.curdir.num_of_entries
#define CURDIR_ENTRY(num) 	fs->file_selection.curdir.entry[(num)]
#define CURDIR_LIST		fs->file_selection.curdir.entry

#define SUBDIR			fs->file_selection.subdir
#define SUBDIR_ALLOC		fs->file_selection.subdir_alloc
#define SUBDIR_ENTRIES  	fs->file_selection.subdir.num_of_entries
#define SUBDIR_ENTRY(num) 	fs->file_selection.subdir.entry[(num)]
#define SUBDIR_LIST		fs->file_selection.subdir.entry

#define PATH_RESOURCE		fs->file_selection.path
#define FILTER_RESOURCE		fs->file_selection.filter
#define TMP_DIR_RESOURCE	fs->file_selection.tmp_dir
#define HIGHLIGHT		fs->file_selection.highlight_pixel
#define OLD_HIGHLIGHT		fs->file_selection.old_highlight_pixel

#define FILTERS			fs->file_selection.filters
#define DIRS			fs->file_selection.dirs
#define PATH			fs->file_selection.path_field_value
#define FILTER			fs->file_selection.filter_field_value
#define APP_DIR			fs->file_selection.app_dir
#define CURRENT_PATH            (fs->file_selection.current_path)
#define CURRENT_DIR             (fs->file_selection.current_dir)
#define CURRENT_FILE            (fs->file_selection.current_file)

#define BUTTONS            	fs->file_selection.internal_buttons
#define BUTTONS_RESOURCE        fs->file_selection.buttons
#define PREFERRED_BUTTON	fs->file_selection.preferred_button

#define REVERSE_SCROLLING	fs->file_selection.reverse_scrolling

#define REALLOC_MORE_IF_NEEDED(list,needed,current) 					\
    if (needed >= current) {								\
       current *= 2;									\
       list = (String *) XtRealloc((char *) list,(unsigned)(current*sizeof(String)));\
    }
#define ALLOC_LIST(list,needed) 							\
    list = (String *) XtMalloc((unsigned)(needed* sizeof(String)))

#define POSITION(pos) ((pos==1)+2*(pos==2)+4*(pos==3)+8*(pos==4)) 
#define IS_BUTTON(pos) (POSITION(pos) & BUTTONS) 
#define POSITION_TO_BUTTON_NAME(pos,name) sprintf((name),"button%d",(int)(pos))

#define MULTICLICK_INTERVAL ((unsigned long) 400)
#define DISABLED        ((XtIntervalId) 0)
#define MULTICLICK      fs->file_selection.multiclick
#define ENABLE_MULTICLICK                                   		\
    MULTICLICK = XtAppAddTimeOut(                       		\
                    XtWidgetToApplicationContext((Widget)FS_FILE_SELECTION),\
                    MULTICLICK_INTERVAL,                		\
                    MulticlickNotify,					\
                    ((XtPointer)FS_FILE_SELECTION)			\
                 )

#define DESTROY_MULTICLICK		\
    if (MULTICLICK) {			\
       XtRemoveTimeOut(MULTICLICK);	\
       MULTICLICK = DISABLED;		\
    }

#define offset(field) XtOffsetOf(FileSelectionRec, file_selection.field)
#define lay_offset(field) XtOffsetOf(FileSelectionRec, aaa.field)

static XtResource resources[] = {
    {XtNpath,XtCPath,XtRString,sizeof(String),offset(path),XtRImmediate,(XtPointer)""},
    {XtNtmpDir,XtCTmpDir,XtRString,sizeof(String),offset(tmp_dir),XtRImmediate,(XtPointer)""},
    {XtNfilter,XtCFilter,XtRString,sizeof(String),offset(filter),XtRImmediate,(XtPointer)""},
    {XtNfilters,XtCFilters,XtRString,sizeof(String),offset(filters),XtRImmediate,(XtPointer)"None\nno .*"},
    {XtNdirs,XtCDirs,XtRString,sizeof(String),offset(dirs),XtRImmediate,(XtPointer)"Home\nTmp"},
    {XtNbuttons,XtCButtons,XtRInt,sizeof(int),offset(buttons),XtRImmediate,(XtPointer)0},
    {XtNpreferredButton,XtCPreferredButton,XtRInt,sizeof(int),offset(preferred_button),XtRImmediate,(XtPointer)0},
    {XtNhighlightPixel, XtCHighlightPixel, XtRPixel, sizeof(Pixel),offset(highlight_pixel), XtRImmediate, (XtPointer)NULL}, 
    {XtNreverseScrolling, XtCReverseScrolling, XtRBoolean, sizeof(Boolean),offset(reverse_scrolling), XtRImmediate, (XtPointer)False},
#if 0
    {XtNlayout, XtCLayout, XtRLayout, sizeof(BoxPtr),lay_offset(layout),XtRString,FILE_SELECTION_LAYOUT},
#endif
    {XtNlayout, XtCLayout, XtRLayout, sizeof(BoxPtr),lay_offset(layout),XtRLayout,NULL},
    {XtNresizeWidth,  XtCBoolean, XtRBoolean, sizeof(Boolean),lay_offset(resize_width),XtRImmediate,(XtPointer)True},
    {XtNresizeHeight, XtCBoolean, XtRBoolean, sizeof(Boolean),lay_offset(resize_height),XtRImmediate,(XtPointer)False},
};
#undef offset
#undef lay_offset

static Boolean SetValues(Widget,Widget,Widget,ArgList,Cardinal*);
static void ClassInitialize(void);
static void Initialize(Widget,Widget,ArgList,Cardinal*);
static void Realize(Widget,XtValueMask*,XSetWindowAttributes*);
static void Destroy(Widget);
static void filtersProc(Widget,XtPointer,XtPointer);
static void dirsProc(Widget,XtPointer,XtPointer);
static void rescanProc(Widget,XtPointer,XtPointer);
static void TopDirSelectionProc(Widget,XtPointer,XtPointer);
static void CurDirSelectionProc(Widget,XtPointer,XtPointer);
static void SubDirSelectionProc(Widget,XtPointer,XtPointer);
static void changeLists(FileSelectionWidget);
static Widget BuildMenu(Widget,String,String,XtCallbackProc);
static void SetDirectoryView(FileSelectionWidget,char*);
static void CreateTextField(Widget*,Widget*,char*,XtTranslations,String,Widget);
static void SetPreferredButton(Widget,int,int);
static void CreateList(Widget*,Widget*,Widget*,Widget*,Widget*,XtTranslations,String,Widget);

FileSelectionClassRec file_selectionClassRec = {
  {
/* core class fields */
    /* superclass         */   (WidgetClass) (&aaaClassRec),
    /* class name         */   "FileSelection",
    /* size               */   sizeof(FileSelectionRec),
    /* class_initialize   */   ClassInitialize,
    /* class_part init    */   NULL,
    /* class_inited       */   FALSE,
    /* initialize         */   Initialize,
    /* initialize_hook    */   NULL,
    /* realize            */   Realize,
    /* actions            */   file_selectionActionsTable,
    /* num_actions        */   XtNumber(file_selectionActionsTable),
    /* resources          */   resources,
    /* resource_count     */   XtNumber(resources),
    /* xrm_class          */   NULLQUARK,
    /* compress_motion    */   0,
    /* compress_exposure  */   0,
    /* compress_enterleave*/   0,
    /* visible_interest   */   FALSE,
    /* destroy            */   Destroy,
    /* resize             */   XtInheritResize,
    /* expose             */   NULL,
    /* set_values         */   SetValues,
    /* set_values_hook    */   NULL,
    /* set_values_almost  */   XtInheritSetValuesAlmost,
    /* get_values_hook    */   NULL,
    /* accept_focus       */   NULL,
    /* version            */   XtVersion,
    /* callback_private   */   NULL,
    /* tm_table           */   NULL,
    /* query_geometry     */   XtInheritQueryGeometry,
    /* display_accelerator*/   XtInheritDisplayAccelerator,
    /* extension          */   NULL
   }, 
   {
/* composite class fields */
    /* geometry_manager   */   XtInheritGeometryManager,
    /* change_managed     */   XtInheritChangeManaged,
    /* insert_child       */   XtInheritInsertChild,
    /* delete_child       */   XtInheritDeleteChild,
    /* extension          */   NULL
   }, 
   {
/* constraint class fields */
    /* subresources       */   NULL,
    /* subresource_count  */   0,
    /* constraint_size    */   sizeof(FileSelectionConstraintsRec),
    /* initialize         */   NULL,
    /* destroy            */   NULL,
    /* set_values         */   NULL,
    /* extension          */   NULL
   },
  { 
/* aaa class fields */
    /* foo                */   0
  },
  { 
/* file selection class fields */
    /* empty              */   0
  }  
};

WidgetClass file_selectionWidgetClass = (WidgetClass) &file_selectionClassRec;

/*-------------------------------------------------------------------------------
   ClassInitialize
-------------------------------------------------------------------------------*/

static void 
ClassInitialize(void)
{
   BEGINMESSAGE(ClassInitialize)
   XawInitializeWidgetSet();
   ENDMESSAGE(ClassInitialize)
}

/*-------------------------------------------------------------------------------
   Initialize
-------------------------------------------------------------------------------*/

static void Initialize(Widget request _GL_UNUSED, Widget new, ArgList argl _GL_UNUSED, Cardinal *num_argl _GL_UNUSED)
{
   FS_WIDGET 	new;

   BEGINMESSAGE(Initialize)

   { 
       char app_dir[FS_MAXNAMLEN];
       getwd(app_dir);
       APP_DIR = XtNewString(app_dir);
   }  

   FS_RESCANBUTTON  = ADD_Widget("rescan",buttonWidgetClass,new);
                ADD_Callback(FS_RESCANBUTTON,rescanProc);

   FILTERS=XtNewString(FILTERS);
   FS_FILTERSBUTTON = ADD_Widget("filters",mbuttonWidgetClass,new);
   FS_FILTERSMENU = BuildMenu(FS_FILTERSBUTTON,FILTERS,"None",filtersProc);

   DIRS=XtNewString(DIRS);
   FS_DIRSBUTTON = ADD_Widget("dirs",mbuttonWidgetClass,new);
   FS_DIRSMENU = BuildMenu(FS_DIRSBUTTON,DIRS,"Home\nTmp",dirsProc);

   BUTTONS = 0;
   IMESSAGE(BUTTONS_RESOURCE)
   if ((BUTTONS_RESOURCE > 0) && (BUTTONS_RESOURCE<5)) {
      if (BUTTONS_RESOURCE > 0) {
         FS_BUTTON1 = ADD_Widget("button1",buttonWidgetClass,new);
         BUTTONS += 1;
      }
      if (BUTTONS_RESOURCE > 1) {
         FS_BUTTON2 = ADD_Widget("button2",buttonWidgetClass,new);
         BUTTONS += 2;
      }
      if (BUTTONS_RESOURCE > 2) {
         FS_BUTTON3 = ADD_Widget("button3",buttonWidgetClass,new);
         BUTTONS += 4;
      }
      if (BUTTONS_RESOURCE > 3) {
         FS_BUTTON4 = ADD_Widget("button4",buttonWidgetClass,new);
         BUTTONS += 8;
      }
      IMESSAGE(BUTTONS)
   }

   {
      XtTranslations text_trans;
      String value;
      text_trans=XtParseTranslationTable(TextField_translations);
      value=PATH_RESOURCE ? PATH_RESOURCE : APP_DIR;
      CreateTextField(&FS_PATHFRAME,  &FS_PATH,  value,text_trans,"path",  new);
      value=FILTER_RESOURCE;
      CreateTextField(&FS_FILTERFRAME,&FS_FILTER,value,text_trans,"filter",new);
   }

   if (BUTTONS_RESOURCE > 0 && BUTTONS_RESOURCE < 5 ) {
	XtInstallAccelerators(FS_PATH,FS_BUTTON1);
	XtInstallAccelerators(FS_FILTER,FS_BUTTON1);
      if (BUTTONS_RESOURCE > 1) {
	 XtInstallAccelerators(FS_PATH,FS_BUTTON2);
	 XtInstallAccelerators(FS_FILTER,FS_BUTTON2);
      }
      if (BUTTONS_RESOURCE > 2) {
	 XtInstallAccelerators(FS_PATH,FS_BUTTON3);
	 XtInstallAccelerators(FS_FILTER,FS_BUTTON3);
      }
      if (BUTTONS_RESOURCE > 3) {
	 XtInstallAccelerators(FS_PATH,FS_BUTTON4);
	 XtInstallAccelerators(FS_FILTER,FS_BUTTON4);
      }
   }
   SetPreferredButton(new,PREFERRED_BUTTON,TRUE);

   {
      XtTranslations list_trans;
      list_trans=XtParseTranslationTable(list_translations);
      CreateList(NULL,         &FS_TOPFRAME,&FS_TOPCLIP,&FS_TOPAAA,&FS_TOPLIST,list_trans,"top",new);
      CreateList(&FS_SUBSCROLL,&FS_SUBFRAME,&FS_SUBCLIP,&FS_SUBAAA,&FS_SUBLIST,list_trans,"sub",new);
      list_trans=XtParseTranslationTable(curlist_translations);
      CreateList(&FS_CURSCROLL,&FS_CURFRAME,&FS_CURCLIP,&FS_CURAAA,&FS_CURLIST,list_trans,"cur",new);
   }

   TOPDIR_ALLOC    = 20;  TOPDIR_ENTRIES = 0; ALLOC_LIST(TOPDIR_LIST,TOPDIR_ALLOC);
   TOPDIR_ENTRY(0) = NULL;
   CURDIR_ALLOC    = 100; CURDIR_ENTRIES = 0; ALLOC_LIST(CURDIR_LIST,CURDIR_ALLOC);
   CURDIR_ENTRY(0) = NULL;
   SUBDIR_ALLOC    = 20;  SUBDIR_ENTRIES = 0; ALLOC_LIST(SUBDIR_LIST,SUBDIR_ALLOC);
   SUBDIR_ENTRY(0) = NULL;

   FS_OLD_TEXTFIELD = (Widget)NULL;

   CURRENT_PATH = XtMalloc(FS_MAXNAMLEN*sizeof(char));
   CURRENT_DIR  = XtMalloc(FS_MAXNAMLEN*sizeof(char));
   CURRENT_FILE = XtMalloc(FS_MAXNAMLEN*sizeof(char));
   PATH         = XtNewString("");
   FILTER       = XtNewString("");
   MULTICLICK = DISABLED;

   XtCallActionProc(FS_PATH,"FS_textfieldFocusAction",(XEvent *)NULL,(String *)NULL,(Cardinal)0);
   XtCallActionProc(FS_FILTER,"FS_textfieldFocusAction",(XEvent *)NULL,(String *)NULL,(Cardinal)0);

   ENDMESSAGE(Initialize)
}

/*-------------------------------------------------------------------------------
   Realize
-------------------------------------------------------------------------------*/

static void Realize (Widget w, XtValueMask *valueMask, XSetWindowAttributes *attrs)
{
   FS_WIDGET w;
   BEGINMESSAGE(Realize)
   (*file_selectionWidgetClass->core_class.superclass->core_class.realize)(w, valueMask, attrs);
   changeLists((FileSelectionWidget)w);
   FS_textfieldFocusAction(FS_PATH, NULL, NULL, NULL);
   ENDMESSAGE(Realize)
}

/*-------------------------------------------------------------------------------
   SetValues
-------------------------------------------------------------------------------*/

static Boolean SetValues(Widget current, Widget request _GL_UNUSED, Widget new, ArgList in_args _GL_UNUSED, Cardinal *in_num_args _GL_UNUSED)
{
  FS_WIDGET	new;
  FileSelectionWidget cfs = (FileSelectionWidget)current;
  USE_Arg(5);

  BEGINMESSAGE(SetValues)

  if (PATH_RESOURCE != cfs->file_selection.path) {
    if (PATH_RESOURCE) { SET_Value(FS_PATH,XtNstring,PATH_RESOURCE); }
    else { SET_Value(FS_PATH,XtNstring,APP_DIR); }
    INFMESSAGE(changing Path Selection Field)
  }
  
  if (FILTER_RESOURCE != cfs->file_selection.filter) {
    SET_Value(FS_FILTER,XtNstring,FILTER_RESOURCE);
    INFMESSAGE(changing Filter Field)
  }

  if (PREFERRED_BUTTON != cfs->file_selection.preferred_button) {
    SetPreferredButton(new,cfs->file_selection.preferred_button,FALSE);
    SetPreferredButton(new,PREFERRED_BUTTON,TRUE);
    INFMESSAGE(switched Accelerators)
  }

  if (FILTERS != cfs->file_selection.filters) {
    XtFree(cfs->file_selection.filters);
    FILTERS=XtNewString(FILTERS);
    XtDestroyWidget(FS_FILTERSMENU);
    FS_FILTERSMENU=BuildMenu(FS_FILTERSBUTTON,FILTERS,"None",filtersProc);
    INFMESSAGE(switched available filters)
  }

  if (DIRS != cfs->file_selection.dirs) {
    XtFree(cfs->file_selection.dirs);
    DIRS=XtNewString(DIRS);
    XtDestroyWidget(FS_DIRSMENU);
    FS_DIRSMENU=BuildMenu(FS_DIRSBUTTON,DIRS,"Home\nTmp",dirsProc);
    INFMESSAGE(switched available dirs)
  }

  ENDMESSAGE(SetValues)
  return False;
}

/*-------------------------------------------------------------------------------
   Destroy
-------------------------------------------------------------------------------*/

static void
Destroy(Widget w)
{
   FS_WIDGET w;

   BEGINMESSAGE(Destroy)
   while ((--TOPDIR_ENTRIES) >=0 ) XtFree(TOPDIR_ENTRY(TOPDIR_ENTRIES)); ++TOPDIR_ENTRIES;
   while ((--CURDIR_ENTRIES) >=0 ) XtFree(CURDIR_ENTRY(CURDIR_ENTRIES)); ++CURDIR_ENTRIES;
   while ((--SUBDIR_ENTRIES) >=0 ) XtFree(SUBDIR_ENTRY(SUBDIR_ENTRIES)); ++SUBDIR_ENTRIES;

   XtFree((XtPointer)TOPDIR_LIST);
   XtFree((XtPointer)CURDIR_LIST);
   XtFree((XtPointer)SUBDIR_LIST);
   XtFree(PATH);
   XtFree(FILTER);
   XtFree(CURRENT_PATH);
   XtFree(CURRENT_DIR);
   XtFree(CURRENT_FILE);
   XtFree(APP_DIR);
   XtFree(FILTERS);
   XtFree(DIRS);
   DESTROY_MULTICLICK;

   ENDMESSAGE(Destroy)
}

/*-------------------------------------------------------------------------------
   strwild
-------------------------------------------------------------------------------*/

static Boolean strwild(char *string, char *wild)
{
  char  *cwild;
  int 	 nwild;
  int   wildlen;
  char  *colon,*stringori;
  Boolean match,match_total=FALSE,exclude=FALSE;
   
  INFMESSAGE1(executing strwild)

  if (!string) return(FALSE);
  if (!wild)   return(TRUE);
  stringori=string;
  while (wild) {
    match = TRUE;
    nwild=0;
    colon=strchr(wild,',');
    if (colon) *colon = '\0';
    if (*wild == '!') { exclude=TRUE; wild++; }
    if (*wild=='\0') {
      match=True;
    } else {
      strcpy((cwild=malloc(strlen(wild)+1)),wild);
      wild = cwild; while ((wild=strchr(wild,'*'))) { ++nwild;  *wild++ = '\0'; }
      wild=cwild;
      wildlen=strlen(wild);
      
      if ((wildlen) && ((strncmp(string,wild,wildlen)) || ((!nwild) && (*(string += wildlen))))) {
	match=FALSE;
      } else {
	wild += (wildlen+1);
	while (nwild) {
	  wildlen=strlen(wild);
	  if ((wildlen) && ((!(string = strstr(string,wild))) || ((nwild==1) && (*(string += wildlen))))) { 
	    match=FALSE; 
	    break;
	  } else {
	    wild += (wildlen+1);
	    --nwild;
	  }
	}
      }
      free(cwild);
    }
    if (colon) { wild = colon +1; *colon = ','; }
    else       wild = NULL;
    string=stringori;
    if (match==TRUE) {
      if (!exclude) match_total=TRUE;
      else return(FALSE);
    }
  }
  return(match_total);
}

/*-------------------------------------------------------------------------------
   strreplace
-------------------------------------------------------------------------------*/

static void
strreplace(char *out, char *find, char *replace, char *in)
{
   int locat = 0;
   int findlength;
   char *intemp;
   char *temp;

   INFMESSAGE1(executing strreplace)

   findlength = strlen(find);
   if (!(*in) || !(*find)) return;

   intemp = XtMalloc(strlen(in)+1);
   strcpy(intemp,in);

   temp=intemp;
   while ((temp=strstr(temp,find))) { *temp='\0'; temp += findlength; ++locat; }

   temp=intemp; *out = '\0';
   while ((locat--) > 0) {
      strcat(out,temp); strcat(out,replace);
      temp = strchr(temp,'\0') + findlength;
   }
   strcat(out,temp);
   XtFree(intemp);
}  

/*----------------------------------------------------------------------
   appendDirEndSeparator
----------------------------------------------------------------------*/

static void
appendDirEndSeparator(char *path)
{
   size_t l=0;
   BEGINMESSAGE(appendDirEndSeparator)
   INFSMESSAGE(old:,path)
   if (path) l=strlen(path);
   if (l && l<=(FS_MAXNAMLEN-2) && path[l-1] != '/') strcat(path,"/");   
   INFSMESSAGE(new:,path);
   ENDMESSAGE(appendDirEndSeparator)
}

/*----------------------------------------------------------------------
   savestrcpy
----------------------------------------------------------------------*/

static void
savestrcpy(char *dest, char *source)
{
  BEGINMESSAGE(savestrcpy)
  strncpy(dest,source,FS_MAXNAMLEN-1);
  dest[FS_MAXNAMLEN-1]='\0';
  ENDMESSAGE(savestrcpy)
}

/*----------------------------------------------------------------------
   setText
----------------------------------------------------------------------*/

static void
setText(Widget w, String line)
{
  USE_Arg(1);

  BEGINMESSAGE(setText)
  if (!line) line="";
  SET_Value(w,XtNstring,line);
  SET_Value(w,XtNinsertPosition,strlen(line));
  ENDMESSAGE(setText)
}

/*----------------------------------------------------------------------
   savestrcat
----------------------------------------------------------------------*/

static void
savestrcat(char *dest, char *source)
{
  size_t ld,ls;
  char *s;

  BEGINMESSAGE(savestrcat)
  ld = strlen(dest);
  ls = strlen(source);
  s = dest + ld; 
  strncpy(s,source,FS_MAXNAMLEN-1-ld);
  dest[FS_MAXNAMLEN-1]='\0';
  ENDMESSAGE(savestrcat)
}

/*-------------------------------------------------------------------------------
   translateTildeInPath
-------------------------------------------------------------------------------*/

static void translateTildeInPath(char *path)
{
  char *pos;

  BEGINMESSAGE(translateTildeInPath)
  INFSMESSAGE(old,path)
  if (path && (pos=strchr(path,'~'))) {
    char *home;
    char tmp[FS_MAXNAMLEN];
    home=getenv("HOME");
    if (home) {
      *pos='\0'; pos++;
      savestrcpy(tmp,path);
      savestrcat(tmp,home);
      savestrcat(tmp,pos);
      savestrcpy(path,tmp);
    }
  }
  INFSMESSAGE(new,path)
  ENDMESSAGE(translateTildeInPath)
}

/*-------------------------------------------------------------------------------
   FScompareEntries
-------------------------------------------------------------------------------*/

static int FScompareEntries(const void *a, const void *b)
{
  String *p = (String*) a;
  String *q = (String*) b;

  return strcmp(*p,*q);

}

/*-------------------------------------------------------------------------------
   SetPreferredButton
-------------------------------------------------------------------------------*/

static void preferButton(Widget w, int prefer)
{
   BEGINMESSAGE(preferButton)
   if (w) {
     if (prefer) ButtonHighlight(w,NULL,NULL,NULL);
     else        ButtonReset(w,NULL,NULL,NULL);
   }
   ENDMESSAGE(preferButton)
}

static void SetPreferredButton(Widget w, int position, int install)
{
   FS_WIDGET 	w;
   char 	name[10];
   Widget 	button;

   BEGINMESSAGE(SetPreferredButton)

   IMESSAGE(position)
   if (IS_BUTTON(position)) {
      POSITION_TO_BUTTON_NAME(position,name);
      button = XtNameToWidget((Widget)FS_FILE_SELECTION,name);
      if (!install) { 
         preferButton(button,0);
         if (PREFERRED_BUTTON==position) PREFERRED_BUTTON=0;
      } else {
         XtInstallAccelerators(FS_PATH,button);
         XtInstallAccelerators(FS_FILTER,button);
         preferButton(button,1);
         PREFERRED_BUTTON=position;
      }
      IMESSAGE(PREFERRED_BUTTON)
   }
   ENDMESSAGE(SetPreferredButton)
}

/*-------------------------------------------------------------------------------
    FS_preferButtonAction
-------------------------------------------------------------------------------*/

static void FS_preferButtonAction(Widget w, XEvent *event _GL_UNUSED, String *params, Cardinal *nparams)
{
   FileSelectionWidget FS_FILE_SELECTION;

   BEGINMESSAGE(FS_preferButtonAction)

   if (!w || !params || !nparams || !(*nparams)) {
      INFMESSAGE(illegal call)
      ENDMESSAGE(FS_preferButtonAction)
      return;
   }

   while (w && XtClass(w) != file_selectionWidgetClass) w = XtParent(w);
   if (!w) {
      INFMESSAGE(could not find file selection widget)
      ENDMESSAGE(FS_preferButtonAction)
      return;
   }
   FS_FILE_SELECTION = (FileSelectionWidget)w;

   if (streq(params[0],"next")) {
      int old,new;
      INFMESSAGE(next)
      new=old=PREFERRED_BUTTON;
      INFIMESSAGE(old preferred button:,old)
      do {
         ++new; if (new>4) new=1;
      } while (new!=old && !IS_BUTTON(new));
      if (new != old) {
         SetPreferredButton(w,old,FALSE);
         SetPreferredButton(w,new,TRUE);
      }
      INFIMESSAGE(new preferred button:,new)
   }
   ENDMESSAGE(FS_preferButtonAction)
}


/*-------------------------------------------------------------------------------
    FS_listAction
-------------------------------------------------------------------------------*/

static void FS_listAction(Widget w, XEvent *event, String *params, Cardinal *nparams)
{
#  define HISTORY_POINTS 10
#  define DECAY_TIME 200
   static int firstposx,posx[HISTORY_POINTS+1],posix;
   static int firstposy,posy[HISTORY_POINTS+1],posiy;
   static int childx,childy,childw,childh,clipw,cliph,childyp;
   static int moving=0,scrolling=0;
   static int startvisible=0;
   static Time to;
   Widget child;
   Widget clip;
   Widget list;
   int entry;
   FileSelectionWidget FS_FILE_SELECTION;

   BEGINMESSAGE(FS_listAction)

   if (!w || !event || !params || !nparams || !(*nparams)) {
      INFMESSAGE(illegal call)
      ENDMESSAGE(FS_listAction)
      return;
   }
   if (XtClass(w) != vlistWidgetClass) {
      INFMESSAGE(caller is not a list widget)
      ENDMESSAGE(FS_listAction)
      return;
   }

   list   = w;
   child  = XtParent(list);
   clip   = XtParent(child);
   FS_FILE_SELECTION = (FileSelectionWidget) XtParent(XtParent(clip));

   if (streq(params[0],"set")) {
      INFMESSAGE(set)
      if (!scrolling) {
	entry = VlistEntryOfPosition(list,(int)event->xbutton.y);
	if (entry >=0 && entry < VlistEntries(list))
	  VlistChangeSelected(list,entry,XawVlistSet);
      }
   }
   else if (streq(params[0],"unset")) {
     entry = VlistSelected(list);
     INFMESSAGE(unset)
     VlistChangeSelected(list,entry,XawVlistUnset);
   }
  else if (!strcmp(params[0],"highlight")) {
#if 0
    {
      Window root, child;
      int dummyx, dummyy;
      unsigned int dummymask;
      int winx,winy;
      INFMESSAGE(highlight)
      XQueryPointer(XtDisplay(list), XtWindow(list), &root, &child, 
		    &dummyx, &dummyy,
		    &winx, &winy, &dummymask);
      entry = VlistEntryOfPosition(list,winy);
      if (VlistSelected(list) == -1 && entry != VlistHighlighted(list))
	VlistChangeHighlighted(list,entry,XawVlistSet);
    }
#endif
    entry = VlistEntryOfPosition(list,(int)event->xbutton.y);
    if (VlistSelected(list) == -1 && entry != VlistHighlighted(list))
      VlistChangeHighlighted(list,entry,XawVlistSet);
  }
  else if (!strcmp(params[0],"unhighlight")) {
    entry = VlistHighlighted(list);
    INFMESSAGE(unhighlight)
    VlistChangeHighlighted(list,entry,XawVlistUnset);
  }
  else if (!strcmp(params[0],"reset")) {
    entry = VlistHighlighted(list);
    INFMESSAGE(reset)
    VlistChangeHighlighted(list,entry,XawVlistUnset);
    entry = VlistSelected(list);
    VlistChangeSelected(list,entry,XawVlistUnset);
  }
  else if (streq(params[0],"notify")) {
    INFMESSAGE(notify)
    if (!scrolling) {
      int e = VlistEntryOfPosition(list,(int)event->xbutton.y);
      if (e >=0 && e < VlistEntries(list)) {
	if      (list == FS_CURLIST) CurDirSelectionProc(list,NULL,(XtPointer)(intptr_t)e);
	else if (list == FS_SUBLIST) SubDirSelectionProc(list,NULL,(XtPointer)(intptr_t)e);
	else if (list == FS_TOPLIST) TopDirSelectionProc(list,NULL,(XtPointer)(intptr_t)e);
      }
    }
  }
  else if (streq(params[0],"scrollon")) {
    INFMESSAGE(start-move)
    if (event->type != ButtonPress) goto break_scrolling;
    moving = 1;
    scrolling = 0;
    posix=posiy=0;
    firstposx = posx[0] = (int) event->xbutton.x_root;
    firstposy = posy[0] = (int) event->xbutton.y_root;
    to = ((XMotionEvent*)event)->time;
    startvisible = VlistGetFirstVisible(list);
    childyp = 0;
  }
  else if (streq(params[0],"scroll")) {
    if (event->type != MotionNotify) goto break_scrolling;
    childx = (int) child->core.x;
    childy = (int) child->core.y;
    childw = (int) child->core.width;
    childh = (int) child->core.height;
    clipw  = (int) clip->core.width;
    cliph  = (int) clip->core.height;
    if ((abs((int) event->xbutton.x_root-firstposx)>1) ||
	(abs((int) event->xbutton.y_root-firstposy)>1)) scrolling = 1;
    IIMESSAGE1(childx,childy)
    IIMESSAGE1(childw,childh)
    IIMESSAGE1(clipw,cliph)
    INFMESSAGE(move)
    if (moving && clipw && cliph) {
      int x,y;
      int dx,dy;
      double relfactor=1.0;  /* some default value */
      double absfactor=0.0;  /* some default value */
      x = (int) event->xbutton.x_root;
      y = (int) event->xbutton.y_root;             
      
      if ((((XMotionEvent*)event)->time - to) > DECAY_TIME) {
	if (posix>0) { posx[0]=posx[posix]; posix=0; }
	if (posiy>0) { posy[0]=posy[posiy]; posiy=0; }
      }
      if (posix>0 && (x-posx[posix])*(posx[posix]-posx[posix-1]) < 0) {
	posx[0]=posx[posix]; posix=0;
      }
      if (posiy>0 && (y-posy[posiy])*(posy[posiy]-posy[posiy-1]) < 0) {
	posy[0]=posy[posiy]; posiy=0;
      }
      to = ((XMotionEvent*)event)->time;
      ++posix;
      ++posiy;
      
      if (posix>HISTORY_POINTS) {
	posix=1;
	while (posix<=HISTORY_POINTS) { posx[posix-1]=posx[posix]; posix++; }
	posix=HISTORY_POINTS;
      }
      posx[posix] = x;
      if (posiy>HISTORY_POINTS) {
	posiy=1;
	while (posiy<=HISTORY_POINTS) { posy[posiy-1]=posy[posiy]; posiy++; }
	posiy=HISTORY_POINTS;
      }
      posy[posiy] = y;
      
      dx = (x - posx[0])/(posix);
      dy = (y - posy[0])/(posiy);
#if 0
      printf("time=%d x=%d y=%d dx=%d dy=%d\n",(int)to,x,y,dx,dy);
      printf("posix=%d posx[posix]=%d posx[0]=%d\n",posix,posx[posix],posx[0]);
      printf("posiy=%d posy[posiy]=%d posy[0]=%d\n",posiy,posy[posiy],posy[0]);
#endif
      if (dx || dy) {
	if (*nparams>=2) relfactor = atof((char*)(params[1]));
	relfactor = relfactor >= 0 ? (relfactor<=100 ? relfactor : 100) : 0;
	if (*nparams>=3) absfactor = atof((char*)(params[2]));
	absfactor = absfactor >= 0 ? (absfactor<=200 ? absfactor : 200) : 0;
	DDMESSAGE1(absfactor,relfactor)
        if (REVERSE_SCROLLING) { dx = -dx; } else { dy = -dy; }
	childx = (int) (childx-(dx*absfactor)-(relfactor*childw*dx)/clipw);
	childy = (int) (childy-(dy*absfactor)-(relfactor*childh*dy)/cliph);
	ClipWidgetSetCoordinates(clip,childx,0);
	if (childy != childyp)
	{
	   childyp=childy;
           startvisible = VlistGetFirstVisible(list);
	   VlistMoveFirstVisible(list, startvisible, childy);
	}
	childx = (int) child->core.x;
	childy = (int) child->core.y;
	childw = (int) child->core.width;
	childh = (int) child->core.height;
	clipw  = (int) clip->core.width;
	cliph  = (int) clip->core.height;
      }
    }
  }
  else if (streq(params[0],"scrolloff")) {
break_scrolling:
    INFMESSAGE(stop-move)
    moving = 0;
    scrolling = 0;
  }
  else if (streq(params[0],"page") && !scrolling) {
    int x,y,sx,sy,pw,ph;
    Position midx,midy;
    INFMESSAGE(page)
    x = (int) event->xbutton.x_root;
    y = (int) event->xbutton.y_root;
    sx = (int) child->core.x;
    sy = (int) child->core.y;
    pw  = (int) clip->core.width;
    ph  = (int) clip->core.height;
    XtTranslateCoords(clip,0,((Position)ph/2),&midx,&midy);
    if (y<midy) sy = (int) (sy + abs(ph-20));
    else  sy = (int) (sy - abs(ph-20));
    ClipWidgetSetCoordinates(clip,sx,0);

    startvisible = VlistGetFirstVisible(list);
    VlistMoveFirstVisible(list, startvisible, sy);
  }
  ENDMESSAGE(FS_listAction)
}

/*-------------------------------------------------------------------------------
   FS_textfieldBackSpaceAction 
-------------------------------------------------------------------------------*/

static void FS_textfieldBackSpaceAction(Widget w, XEvent *event _GL_UNUSED, String *parms _GL_UNUSED, Cardinal *nparms _GL_UNUSED)
{
   BEGINMESSAGE(FS_textfieldBackSpaceAction)

   if (XtIsSubclass(w,asciiTextWidgetClass)) {
      XawTextPosition begin_sel,end_sel;

      XawTextGetSelectionPos(w,&begin_sel,&end_sel);
      if (begin_sel != end_sel) 
         XtCallActionProc(w,"kill-selection",(XEvent *)NULL,(String *)NULL,(Cardinal)0);
      else 
         XtCallActionProc(w,"delete-previous-character",(XEvent *)NULL,(String *)NULL,(Cardinal)0);
   }       

   ENDMESSAGE(FS_textfieldBackSpaceAction)
}

/*-------------------------------------------------------------------------------
   FS_textfieldDeleteAction 
-------------------------------------------------------------------------------*/

static void FS_textfieldDeleteAction(Widget w, XEvent *event _GL_UNUSED, String *parms _GL_UNUSED, Cardinal *nparms _GL_UNUSED)
{
   BEGINMESSAGE(FS_textfieldDeleteAction)

   if (XtIsSubclass(w,asciiTextWidgetClass)) {
      XawTextPosition begin_sel,end_sel;

      XawTextGetSelectionPos(w,&begin_sel,&end_sel);
      if (begin_sel != end_sel) 
         XtCallActionProc(w,"kill-selection",(XEvent *)NULL,(String *)NULL,(Cardinal)0);
      else 
         XtCallActionProc(w,"delete-next-character",(XEvent *)NULL,(String *)NULL,(Cardinal)0);
   }       

   ENDMESSAGE(FS_textfieldDeleteAction)
}

/*-------------------------------------------------------------------------------
   FS_textfieldFocusAction 
-------------------------------------------------------------------------------*/

static void FS_textfieldFocusAction(Widget w, XEvent *event _GL_UNUSED, String *parms _GL_UNUSED, Cardinal *nparms _GL_UNUSED)
{
   USE_Arg(5);

   BEGINMESSAGE(FS_textfieldFocusAction)

   if (XtIsSubclass(w,asciiTextWidgetClass)) {
      FS_WIDGET XtParent(XtParent(w));

      if ((FS_OLD_TEXTFIELD) && (w != FS_OLD_TEXTFIELD)) {
         RESET_Arg;
         if (HIGHLIGHT) { SET_Arg(XtNbackground,OLD_HIGHLIGHT); }     
         SET_Arg(XtNdisplayCaret,     False);
         SET_Values(FS_OLD_TEXTFIELD);
      } 

      if ((!FS_OLD_TEXTFIELD) || (FS_OLD_TEXTFIELD != w)) {
         XtSetKeyboardFocus((Widget)FS_FILE_SELECTION, w);
         if (HIGHLIGHT) { GET_Value(w,XtNbackground,&(OLD_HIGHLIGHT)); } 
         RESET_Arg;
         if (HIGHLIGHT) { SET_Arg(XtNbackground,HIGHLIGHT); }     
         SET_Arg(XtNdisplayCaret,      True);
         SET_Values(w);
      }
      FS_OLD_TEXTFIELD = w;
   }
   ENDMESSAGE(FS_textfieldFocusAction)
}  

/*-------------------------------------------------------------------------------
    MulticlickNotify
-------------------------------------------------------------------------------*/

static void MulticlickNotify(XtPointer client_data, XtIntervalId *idp _GL_UNUSED)
{
    FS_WIDGET client_data;
   
    BEGINMESSAGE(MulticlickNotify) 
    MULTICLICK = DISABLED;
    ENDMESSAGE(MulticlickNotify)
}                               

/*-------------------------------------------------------------------------------
   changeList
-------------------------------------------------------------------------------*/

static void changeList(Widget w, String *list, int entries)
{
  int i,e,l;
  USE_Arg(2);
  char *d,*s,*p;

  BEGINMESSAGE(changeList)
  i=e=l=0;
  while (i<entries) {
     l = l +strlen(list[i]) + 1;
     e++;
     i++;
  }
  l++;
  e++;
  s = XtMalloc(l*sizeof(char));
  d = XtMalloc(e*sizeof(char));
  s[0]='\0';
  i=0;
  p = s;
  while (i<entries) {
     size_t len = strlen(list[i]);

     memcpy(p, list[i], len);
     p += len;
     *(p++) = '\n';
     d[i]=' ';
     i++;
  }
  *p = '\0';
  d[i]='\0';

  RESET_Arg;
  SET_Arg(XtNlabel, s);
  SET_Arg(XtNvlist, d);
  SET_Values(w);
  XtFree(s);
  XtFree(d);
  ENDMESSAGE(changeList)
} 

/*-------------------------------------------------------------------------------
   changeLists
-------------------------------------------------------------------------------*/

static void changeLists(FileSelectionWidget FS_FILE_SELECTION)
{
   BEGINMESSAGE(changeLists)
   IMESSAGE(TOPDIR_ENTRIES)
   IMESSAGE(CURDIR_ENTRIES)
   IMESSAGE(SUBDIR_ENTRIES)
   changeList(FS_TOPLIST, TOPDIR_LIST, TOPDIR_ENTRIES);
   changeList(FS_CURLIST, CURDIR_LIST, CURDIR_ENTRIES);
   changeList(FS_SUBLIST, SUBDIR_LIST, SUBDIR_ENTRIES);
   ENDMESSAGE(changeLists)
}

/*----------------------------------------------------------------------
   SetIncompleteDirectoryView
----------------------------------------------------------------------*/

static void
SetIncompleteDirectoryView(FileSelectionWidget fs, String *list)
{
   BEGINMESSAGE(SetIncompleteDirectoryView)
   changeList(FS_CURLIST, list, 1);
   chdir(APP_DIR);
   ENDMESSAGE(SetIncompleteDirectoryView)
}  

/*----------------------------------------------------------------------
   SplitPath
----------------------------------------------------------------------*/

static void
SplitPath(String path, char *dir, char *file)
{
  char tmp[FS_MAXNAMLEN];
  char *s;

  BEGINMESSAGE(SplitPath)
  SMESSAGE(path)
  savestrcpy(tmp,path);

  if ((s=strrchr(tmp,'/'))) {
    s++;
    savestrcpy(file,s);
    *s='\0';
    savestrcpy(dir,tmp);

  } else {
    savestrcpy(dir,tmp);
    file[0]='\0';
  } 
  SMESSAGE(dir)
  SMESSAGE(file)
  ENDMESSAGE(SplitPath)
}  

/*----------------------------------------------------------------------
   CombineToPath
----------------------------------------------------------------------*/

static void
CombineToPath(String path, char *dir, char *file)
{
  char tmp[FS_MAXNAMLEN];

  BEGINMESSAGE(CombineToPath)
  SMESSAGE(dir)
  SMESSAGE(file)
  savestrcpy(tmp,dir);
  savestrcat(tmp,file);
  savestrcpy(path,tmp);
  SMESSAGE(path)
  ENDMESSAGE(CombineToPath)
}  

/*----------------------------------------------------------------------
   SetDirectoryView
----------------------------------------------------------------------*/

static void SetDirectoryView(FileSelectionWidget fs, char *dir)
{
   DIR			*dirp=NULL;
   struct dirent	*dp;
   String  		str;
   char 		*temp;
   Boolean 		accepted;
   char 		path[FS_MAXNAMLEN];
   char                 *filter,*malloced_filter;
   int			viewmode;
   int error;
   struct stat          sbuf;

   USE_Arg(5);
   
   BEGINMESSAGE(SetDirectoryView)

   if (CURRENT_DIR) chdir(CURRENT_DIR);

   if (dir) setText(FS_PATH,dir);
   GET_Value(FS_PATH,XtNstring,&dir);
   if (!dir || *dir=='\0') dir=CURRENT_DIR;
   if (!dir || *dir=='\0') dir=APP_DIR;
   if (!dir || *dir=='\0') {
     INFMESSAGE(cannot find valid dir)
     ENDMESSAGE(SetDirectoryView)
     return;
   }
   savestrcpy(path,dir);
   if (strchr(path,'~')) translateTildeInPath(path);
   dir = path;
   INFSMESSAGE(trying to chdir to,dir)
   error=chdir(dir);
   if (error) {
     char tmpdir[FS_MAXNAMLEN],tempfile[FS_MAXNAMLEN];
     INFMESSAGE(failed)
     SplitPath(dir,tmpdir,tempfile);
     appendDirEndSeparator(tmpdir);
     INFSMESSAGE(trying to chdir to,tmpdir)
     error=chdir(tmpdir);
     if (!error) savestrcpy(CURRENT_FILE,tempfile);
#ifdef MESSAGES
     else { INFMESSAGE(failed) }
#endif
   } else {
     CURRENT_FILE[0]='\0';
   }
   getwd(CURRENT_DIR);
   appendDirEndSeparator(CURRENT_DIR);
   CombineToPath(path,CURRENT_DIR,CURRENT_FILE);
   savestrcpy(CURRENT_PATH,path);
   SMESSAGE(CURRENT_PATH)
   while ((--CURDIR_ENTRIES) >=0 ) XtFree(CURDIR_ENTRY(CURDIR_ENTRIES)); ++CURDIR_ENTRIES;
   CURDIR_ENTRY(CURDIR_ENTRIES) = (String) NULL;

   if (error) error = 1;
   else if (!(dirp=opendir("."))) error=2;
   if (error) {
     INFIMESSAGE(cannot analyze directory,error)
     changeLists(FS_FILE_SELECTION);
     if (error==1) SetIncompleteDirectoryView(fs,unknownList); 
     else          SetIncompleteDirectoryView(fs,cannotopenList);
     chdir(APP_DIR);
     ENDMESSAGE(SetDirectoryView)
     return;
   }

   while ((--TOPDIR_ENTRIES) >=0 ) XtFree(TOPDIR_ENTRY(TOPDIR_ENTRIES)); ++TOPDIR_ENTRIES;
   TOPDIR_ENTRY(TOPDIR_ENTRIES) = (String) NULL;
   while ((--SUBDIR_ENTRIES) >=0 ) XtFree(SUBDIR_ENTRY(SUBDIR_ENTRIES)); ++SUBDIR_ENTRIES;
   SUBDIR_ENTRY(SUBDIR_ENTRIES) = (String) NULL;
   setText(FS_PATH,CURRENT_PATH);
   GET_Value(FS_FILTER,XtNstring,&filter);
   {
     char *tmp,*f;
     f = filter;
     malloced_filter = tmp = (char*) XtMalloc((strlen(filter)+10)*sizeof(char));
     if (*f=='n') *tmp++=' ';
     while (*f) {
       if (isspace(*f)) {
	 *tmp++ = ' ';
	 f++;
	 while (isspace(*f)) f++;
       } else { *tmp++ = *f; f++; }
     }
     *tmp = '\0';
     tmp = malloced_filter;
     strreplace(tmp," no ",",!",tmp);
     strreplace(tmp," ",",",tmp);
     filter = malloced_filter;
  }
  SMESSAGE(filter)

   if (streq(filter,"")) viewmode = XawFileSelectionRescan;
   else                  viewmode = XawFileSelectionFilter;


   accepted = TRUE;
   while ((dp = readdir(dirp))) {
      str = dp->d_name;
         if (strcmp(str,".")) {
            if (!stat(str,&sbuf) && S_ISDIR(sbuf.st_mode) && (strncmp(str, ".", 1) || !strcmp(str, ".."))) {
               REALLOC_MORE_IF_NEEDED(SUBDIR_LIST,SUBDIR_ENTRIES+1,SUBDIR_ALLOC);
               SUBDIR_ENTRY(SUBDIR_ENTRIES) = XtNewString(str);
               SMESSAGE(SUBDIR_ENTRY(SUBDIR_ENTRIES))
               SUBDIR_ENTRIES++;
            } else {
               if (viewmode==XawFileSelectionFilter) accepted=strwild(str,filter);
               if (accepted) {
                  REALLOC_MORE_IF_NEEDED(CURDIR_LIST,CURDIR_ENTRIES+1,CURDIR_ALLOC);
                  CURDIR_ENTRY(CURDIR_ENTRIES) = XtNewString(str);
                  SMESSAGE(CURDIR_ENTRY(CURDIR_ENTRIES))
                  CURDIR_ENTRIES++;
               }
#              ifdef MESSAGES
               else {
                  INFMESSAGE(list entry not accepted by viewmode)
               }
#              endif
            }
         }
   }

   IMESSAGE(CURDIR_ENTRIES)
   if (!CURDIR_ENTRIES) {
      CURDIR_ENTRY(CURDIR_ENTRIES) = (String) NULL;
      INFMESSAGE(no entries in this direcory)
   }
   else qsort( CURDIR_LIST, CURDIR_ENTRIES, sizeof(char *), FScompareEntries);

   IMESSAGE(SUBDIR_ENTRIES)
   if (!SUBDIR_ENTRIES) {
      SUBDIR_ENTRY(SUBDIR_ENTRIES) = (String) NULL;
      INFMESSAGE(no subdirectories)
   }
   else qsort( SUBDIR_LIST, SUBDIR_ENTRIES, sizeof(char *), FScompareEntries);

   closedir(dirp);

   {
     {
       char *p=path;
       savestrcpy(path,CURRENT_DIR);
       REALLOC_MORE_IF_NEEDED(TOPDIR_LIST,TOPDIR_ENTRIES+1,TOPDIR_ALLOC);
	TOPDIR_ENTRY(TOPDIR_ENTRIES) = XtNewString("/");
	SMESSAGE(TOPDIR_ENTRY(TOPDIR_ENTRIES))
	TOPDIR_ENTRIES++;
	SMESSAGE(p)
        savestrcat(path,"/");
	if (*p=='/') p++;
	temp=p;
	while ((*temp != '/') && (p=strchr(p,'/'))) { 
	  *p = '\0';
	  p++;
	  REALLOC_MORE_IF_NEEDED(TOPDIR_LIST,TOPDIR_ENTRIES+1,TOPDIR_ALLOC);
	  TOPDIR_ENTRY(TOPDIR_ENTRIES) = XtNewString(temp);
	  SMESSAGE(TOPDIR_ENTRY(TOPDIR_ENTRIES))
	  TOPDIR_ENTRIES++;
	  temp=p;
	}
      }

      IMESSAGE1(TOPDIR_ENTRIES)
      if (!TOPDIR_ENTRIES) {
         TOPDIR_ENTRY(TOPDIR_ENTRIES) = (String) NULL;
         INFMESSAGE1(no topdirectories)
      }
   }

   XtFree(malloced_filter);
   changeLists(FS_FILE_SELECTION);
   chdir(APP_DIR);

   ENDMESSAGE(SetDirectoryView)
   return;
}

/*-------------------------------------------------------------------------------
   rescanProc
   callback for the rescan button
-------------------------------------------------------------------------------*/

static void
rescanProc(Widget w, XtPointer client_data _GL_UNUSED, XtPointer call_data _GL_UNUSED)
{
   FS_WIDGET XtParent(w);

   BEGINMESSAGE(rescanProc)
   SetDirectoryView(FS_FILE_SELECTION,NULL);
   ENDMESSAGE(rescanProc)
}

/*-------------------------------------------------------------------------------
   filtersProc
   callback for the filter menu entries
-------------------------------------------------------------------------------*/

static void
filtersProc(Widget w, XtPointer client_data _GL_UNUSED, XtPointer call_data _GL_UNUSED)
{
   FS_WIDGET XtParent(XtParent(XtParent(w)));
   Arg args[2];
   Cardinal n;
   String label;

   BEGINMESSAGE(filtersProc)
                                             n=0;
   XtSetArg(args[n], XtNlabel, &label);      n++;
   XtGetValues(w, args, n);
   if (!strcmp(label,"None")) label="";
                                             n=0;
   XtSetArg(args[n], XtNstring, label);      n++;
   XtSetValues(FS_FILTER, args, n);
   SetDirectoryView(FS_FILE_SELECTION,NULL);
   ENDMESSAGE(filtersProc)
}

/*-------------------------------------------------------------------------------
   dirsProc
   callback for the dirs menu entries
-------------------------------------------------------------------------------*/

static void
dirsProc(Widget w, XtPointer client_data _GL_UNUSED, XtPointer call_data _GL_UNUSED)
{
  FS_WIDGET XtParent(XtParent(XtParent(w)));
  USE_Arg(2);
  String newpath;

  BEGINMESSAGE(dirsProc)
  GET_Value(w,XtNlabel,&newpath);
  if (!strcmp(newpath,"Home")) {
    newpath = HOME;
  }
  else if (!strcmp(newpath,"Tmp")) {
    if (TMP_DIR_RESOURCE) newpath = TMP_DIR_RESOURCE;
    else newpath = HOME;
  }
  SetDirectoryView(FS_FILE_SELECTION,newpath);
  ClipWidgetSetCoordinates(FS_CURCLIP, 0, 0);
  ENDMESSAGE(dirsProc)
}

/*----------------------------------------------------------------------
   TopDirSelectionProc
   callback for topdirectory list
----------------------------------------------------------------------*/

static void
TopDirSelectionProc(Widget w, XtPointer client_data _GL_UNUSED, XtPointer call_data)
{
  FS_WIDGET XtParent(XtParent(XtParent(XtParent(w))));
  int item = (int)(intptr_t)call_data;
  char newpath[FS_MAXNAMLEN];
   
  BEGINMESSAGE(TopDirSelectionProc)

  if (item >= 0) {
#if 0
    if (chdir(CURRENT_DIR)) {
      INFMESSAGE(unable to switch to current directory)
      ENDMESSAGE(TopDirSelectionProc)
      return;
    }
#endif
    savestrcpy(newpath,CURRENT_DIR);
    CHANGE_TO_HEAD_OF_DIR_SPEC(newpath);
    savestrcat(newpath,DIR_SPECIFICATION_START_STRING);
    {
      int i = -1;
      while (++i <= item) {
	if (strcmp(TOPDIR_ENTRY(i),"/")) {
	  savestrcat(newpath,TOPDIR_ENTRY(i));
	  if (i<item) savestrcat(newpath,DIR_SEPARATOR_STRING);
	}
      }
    }
    appendDirEndSeparator(newpath);
    SMESSAGE(newpath)
    VlistSetFirstVisible(FS_CURLIST, 0);
    VlistSetFirstVisible(FS_SUBLIST, 0);
    SetDirectoryView((FS_FILE_SELECTION),newpath);
    ClipWidgetSetCoordinates(FS_CURCLIP, 0, 0);

  }
  ENDMESSAGE(TopDirSelectionProc)
}

/*-------------------------------------------------------------------------------
   CurDirSelectionProc
   callback for current directory list
-------------------------------------------------------------------------------*/

static void
CurDirSelectionProc(Widget w, XtPointer client_data _GL_UNUSED, XtPointer call_data)
{
   FS_WIDGET	XtParent(XtParent(XtParent(XtParent(w))));
   char		name[10];
   int item = (int)(intptr_t) call_data;

   BEGINMESSAGE(CurDirSelectionProc)

   if (!CURDIR_ENTRY(0)) {
     INFMESSAGE(no curdir entries)
     ENDMESSAGE(CurDirSelectionProc)
     return;
   }
   if (item >= 0) {
      char *path="<path too long>";
      char tmp[FS_MAXNAMLEN];
      size_t l;
      if ((l=strlen(CURRENT_DIR)) + strlen(CURDIR_ENTRY(item)) <= FS_MAXNAMLEN-2) {
         savestrcpy(tmp,CURRENT_DIR);
         appendDirEndSeparator(tmp);
         savestrcat(tmp,CURDIR_ENTRY(item));
         path=tmp;
      }
      setText(FS_PATH,path);
      if (MULTICLICK) {
         DESTROY_MULTICLICK;
         if (IS_BUTTON(PREFERRED_BUTTON)) {
            POSITION_TO_BUTTON_NAME(PREFERRED_BUTTON,name);
            XtCallCallbacks(XtNameToWidget((Widget)FS_FILE_SELECTION,name),XtNcallback,call_data);
         }
      }
      else ENABLE_MULTICLICK;
   }
   ENDMESSAGE(CurDirSelectionProc)
}

/*-------------------------------------------------------------------------------
   SubDirSelectionProc
   callback for subdirectory list
-------------------------------------------------------------------------------*/

static void
SubDirSelectionProc(Widget w, XtPointer client_data _GL_UNUSED, XtPointer call_data)
{
  int item = (int)(intptr_t) call_data;
  FS_WIDGET XtParent(XtParent(XtParent(XtParent(w))));
  char newpath[FS_MAXNAMLEN];
   
  BEGINMESSAGE(SubDirSelectionProc)

  if (item >= 0) {
    if (!(strcmp((String)SUBDIR_ENTRY(item),".."))) {
      if (chdir(CURRENT_DIR)) {
	INFMESSAGE(unable to switch to current directory)
        ENDMESSAGE(TopDirSelectionProc)
        return;
      }
      if (chdir(ONE_STEP_UP)) {
	INFMESSAGE(unable to step up)
        ENDMESSAGE(TopDirSelectionProc)
	return;
      }
      getwd(newpath);
    } else {
      savestrcpy(newpath,CURRENT_DIR);
      appendDirEndSeparator(newpath);
      savestrcat(newpath,SUBDIR_ENTRY(item));
      appendDirEndSeparator(newpath);
    }
    SMESSAGE(newpath)
    VlistSetFirstVisible(FS_CURLIST, 0);
    VlistSetFirstVisible(FS_SUBLIST, 0);
    SetDirectoryView(FS_FILE_SELECTION,newpath);
    ClipWidgetSetCoordinates(FS_CURCLIP, 0, 0);
  }
  ENDMESSAGE(SubDirSelectionProc)
}

/*-------------------------------------------------------------------------------
   CreateTextField
-------------------------------------------------------------------------------*/

static void CreateTextField(Widget *frameP, Widget *textP, char *value, XtTranslations text_trans, String namebase, Widget parent)
{
   USE_Arg(10);
   char name[15];

   BEGINMESSAGE(CreateTextField)

               RESET_Arg;
               sprintf(name,"%sframe",namebase);
   *frameP   = ADD_Widget(name,frameWidgetClass,parent);

               RESET_Arg;
               SET_Arg(XtNdisplayCaret,     False);
               SET_Arg(XtNuseStringInPlace, False);
               SET_Arg(XtNstring,           value);
               SET_Arg(XtNeditType,         XawtextEdit);
               SET_Arg(XtNscrollHorizontal, XawtextScrollNever);
               SET_Arg(XtNscrollVertical,   XawtextScrollNever);
               SET_Arg(XtNtranslations,	    text_trans);
               SET_Arg(XtNtype,             XawAsciiString);
               SET_Arg(XtNresize,           XawtextResizeWidth);
               sprintf(name,"%stext",namebase);
   *textP = ADD_Widget_Arg(name, asciiTextWidgetClass,*frameP);

   ENDMESSAGE(CreateTextField)
}

/*-------------------------------------------------------------------------------
   CreateList
-------------------------------------------------------------------------------*/

static void cb_scroll(Widget w, XtPointer client_data, XtPointer call_data)
{
  Widget p;
  char *s;

  BEGINMESSAGE(cb_scroll)

  if (!XtIsRealized(w)) {
    ENDMESSAGE(cb_scroll)
    return;
  }

  s = XtName(w);
  /* names of the clip widgets are "topclip","curclip","subclip" */
  if (s[3] == 'c') { p = XtParent(XtParent(w)); }
  else               p = XtParent(w);
SMESSAGE(s)
SMESSAGE(XtName(p))

  {
    Widget clip=NULL,aaa=NULL,scroll=NULL,list=NULL;
    FS_WIDGET p;
    int style = (int)(intptr_t)client_data;
    if      (s[0] == 'c') { clip = FS_CURCLIP; aaa = FS_CURAAA; scroll = FS_CURSCROLL; list = FS_CURLIST; }
    else if (s[0] == 's') { clip = FS_SUBCLIP; aaa = FS_SUBAAA; scroll = FS_SUBSCROLL; list = FS_SUBLIST; }
    else style=0;
    if (style == SCROLL_SCROLLPROC || style == SCROLL_JUMPPROC) {
      int x;
      x = (int) aaa->core.x;
      ClipWidgetSetCoordinates(clip, x, 0);
      if (((int)(intptr_t)client_data)==1) 
      {
         int dy = (int)(intptr_t)call_data;
         int ly = ((VlistWidget)list)->vlist.ydelta;

         /* Just scroll one position less... */
         if (dy>ly) dy-=ly;
         if (dy<-ly) dy+=ly;
	 
         VlistMoveFirstVisible(list, VlistGetFirstVisible(list), dy);
         XawScrollbarSetThumb(scroll,VlistScrollPosition(list),
		      VlistVisibleLength(list,clip->core.height));
      }
      else
      {
         float *percent = (float *) call_data;
	 VlistSetFirstVisible(list, (int)(VlistEntries(list)**percent+0.5));
      }

    } else if (style == SCROLL_CLIPREPORT) {
      XawScrollbarSetThumb(scroll,VlistScrollPosition(list),
		      VlistVisibleLength(list,clip->core.height));
    }
  }    
  ENDMESSAGE(cb_scroll)
}

static void
CreateList(Widget *scrollP, Widget *frameP, Widget *clipP, Widget *aaaP, Widget *listP, XtTranslations list_trans, String namebase, Widget parent)
{
   USE_Arg(5);
   char name[15];

   BEGINMESSAGE(CreateList)

   if (scrollP) {
                RESET_Arg;
                sprintf(name,"%sscroll",namebase);
     *scrollP =	ADD_Widget_Arg(name,scrollbarWidgetClass,parent);
                XtAddCallback(*scrollP, XtNscrollProc,cb_scroll, (XtPointer)SCROLL_SCROLLPROC);
		XtAddCallback(*scrollP, XtNjumpProc,cb_scroll, (XtPointer)SCROLL_JUMPPROC);
   }

                RESET_Arg;
		if (namebase[0] == 'c') {
		   SET_Arg(XtNresize,	False);
		}
                sprintf(name,"%sframe",namebase);
   *frameP =	ADD_Widget_Arg(name,frameWidgetClass,parent);

                RESET_Arg;
                sprintf(name,"%sclip",namebase);
   *clipP =	ADD_Widget_Arg(name,clipWidgetClass,*frameP);
   if (scrollP) XtAddCallback(*clipP, XtNreportCallback,cb_scroll, (XtPointer)SCROLL_CLIPREPORT);

                RESET_Arg;
                sprintf(name,"%saaa",namebase);
   *aaaP =	ADD_Widget_Arg(name,aaaWidgetClass,*clipP);

                RESET_Arg;
                SET_Arg(XtNlabel,		"");
                SET_Arg(XtNvlist,		"");
                SET_Arg(XtNtranslations,	list_trans);
                SET_Arg(XtNallowMarks,	        False);
                SET_Arg(XtNjustify,	        XtJustifyLeft);
   *listP =     ADD_Widget_Arg("list",vlistWidgetClass,*aaaP);

   ENDMESSAGE(CreateList)
}

/*-------------------------------------------------------------------------------
   BuildMenu
-------------------------------------------------------------------------------*/

static Widget BuildMenu(Widget parent, String descrip, String defaultdescrip, XtCallbackProc cb)
{
  Widget m,w=NULL;
  char *md,*d,*tmp;

  m = XtCreatePopupShell("menu", simpleMenuWidgetClass,parent,NULL,(Cardinal)0);
  md = d = XtNewString(descrip);
  while (*d) {
    while (isspace(*d)) *d++='\r';
    while (*d && *d != '\n') {
      if (isspace(*d) && isspace(*(d+1))) *d='\r';
      d++;
    }
    if (*d) d++;
  }
  d = md +strlen(md);
  if (d != md) while (isspace(*(--d))) *d = '\r';
  d = tmp = md;
  while (*d) {
    if (*d != '\r') *tmp++ = *d;
    d++;
  }
  *tmp='\0';
  d = md;
  if (!*d) {
    XtFree(md);
    md = d = XtNewString(defaultdescrip);
  }
  while (d) {
    tmp=strchr(d,'\n');
    if (tmp) *tmp='\0';
    if (*d) { 
      w = ADD_Widget(d,smeBSBObjectClass,m);
      ADD_Callback(w,cb);
    }
    if (tmp) d=tmp+1;
    else d=NULL;
  }
  XtFree(md);
  return(m);
}

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
   PUBLIC ROUTINES
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*/   
   
/*########################################################################
   XawFileSelectionGetFile
########################################################################*/

char *
XawFileSelectionGetPath(Widget w)
{
   FS_WIDGET	w;
   String	path=NULL;
   USE_Arg(2);
   char tmp[FS_MAXNAMLEN];

   BEGINMESSAGE(XawFileSelectionGetPath)

   GET_Value(FS_PATH,XtNstring,&path);
   if (!path) path ="";
   savestrcpy(tmp,path);
   translateTildeInPath(tmp);
   XtFree(PATH);
   PATH = XtNewString(path);
   INFSMESSAGE(returning, PATH)

   ENDMESSAGE(XawFileSelectionGetPath)
   return(PATH);
}

/*########################################################################
   XawFileSelectionSetPath
########################################################################*/

void
XawFileSelectionSetPath(Widget w,String string)
{
   FS_WIDGET w;

   BEGINMESSAGE(XawFileSelectionSetPath)
   setText(FS_PATH,string);
   ENDMESSAGE(XawFileSelectionGetPath)
}

/*########################################################################
   XawFileSelectionScan
########################################################################*/

void
XawFileSelectionScan(Widget w,int indicator _GL_UNUSED)
{
   FS_WIDGET	w;

   BEGINMESSAGE(XawFileSelectionScan)
   SetDirectoryView(FS_FILE_SELECTION,NULL);
   ENDMESSAGE(XawFileSelectionScan)
}

/*########################################################################
   XawFileSelectionAddButton
########################################################################*/

void
XawFileSelectionAddButton(Widget w, int position, XtCallbackProc function, XtPointer param)
{
   FS_WIDGET 		w;
   char			name[10];
   Widget		button;

   BEGINMESSAGE(XawFileSelectionAddButton)

   if (IS_BUTTON(position)) {
      INFMESSAGE(desired Button Position is already used) ENDMESSAGE(XawFileSelectionAddButton)
      return;
   }

   POSITION_TO_BUTTON_NAME(position,name);
   IMESSAGE(position) SMESSAGE(name);
   button = ADD_Widget(name,buttonWidgetClass,FS_FILE_SELECTION);
   BUTTONS += POSITION(position);
   if (function) XtAddCallback(button, XtNcallback, function, param);

   ENDMESSAGE(XawFileSelectionAddButton)
}

/*########################################################################
   XawFileSelectionRemoveButton
########################################################################*/

void
XawFileSelectionRemoveButton(Widget w, int position)
{
   FS_WIDGET 		w;
   char 		name[10];
   USE_Arg(3);
   Widget button;

   BEGINMESSAGE(XawFileSelectionRemoveButton)

   if (!(IS_BUTTON(position))) { 
      INFMESSAGE(Unused Button Position) ENDMESSAGE(XawFileSelectionRemoveButton)
      return;
   }

   POSITION_TO_BUTTON_NAME(position,name);
   button = XtNameToWidget((Widget)FS_FILE_SELECTION,name);
   SET_Arg( XtNheight, 0);
   SET_Arg( XtNwidth,  0);
   SET_Values(button);
   XtDestroyWidget(button);
   BUTTONS -= POSITION(position);
   IMESSAGE(BUTTONS)
   ENDMESSAGE(XawFileSelectionRemoveButton)
}

/*########################################################################
   XawFileSelectionPreferButton
########################################################################*/

void
XawFileSelectionPreferButton(Widget w, int position)
{
   int i;

   BEGINMESSAGE(XawFileSelectionPreferButton)
   for (i=1 ; i<4 ; i++ ) SetPreferredButton(w,i,(i == position) ? 1 : 0);
   ENDMESSAGE(XawFileSelectionPreferButton)
}



/*
**
** process.c
**
** Copyright (C) 1996, 1997 Johannes Plass
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
#include <stdlib.h>
#include <ctype.h>


#   include <sys/types.h>
#   include <sys/wait.h>
#   include <unistd.h>
#   include <signal.h>
#   include <gv_signal.h>

#include "paths.h"
#include INC_X11(Intrinsic.h)
#include INC_X11(StringDefs.h)
#include INC_XAW(MenuButton.h)
#include INC_XAW(SimpleMenu.h)
#include INC_XAW(SmeBSB.h)
#include INC_X11(IntrinsicP.h)
#include INC_X11(ShellP.h)

#include "types.h"
#include "config.h"
#include "callbacks.h"
#include "process.h"
#include "main_resources.h"
#include "main_globals.h"
#include "misc_private.h"

#include "resource.h"

#define CHECK_PERIOD 500
#define ADD_TIMEOUT(widget,interval,data)       \
        XtAppAddTimeOut (                       \
                  app_con,                      \
                  ((unsigned long)(interval)),  \
                  process_notify,               \
                  ((XtPointer)(data))           \
                )

#define DISABLED	((XtIntervalId) 0)
#define DESTROY_TIMER(aaa)			\
	if ((aaa)) {				\
		XtRemoveTimeOut((aaa));		\
		aaa = DISABLED;			\
	}

static ProcessData gpd = NULL;

/*------------------------------------------------------------*/
/* process_get_pd */
/*------------------------------------------------------------*/

static ProcessData process_get_pd(void)
{
   ProcessData pd;
   Cardinal size = sizeof(ProcessDataStruct);
   BEGINMESSAGE(process_get_pd)
#  ifdef MESSAGES
      if (!gpd) { INFMESSAGE(no processes registered yet) }
#  endif
   pd = (ProcessData) XtMalloc(size);
   memset((void*) pd ,0,(size_t)size);
   if (!gpd) gpd = pd;
   else {
      ProcessData tmppd;
      tmppd = gpd; while (tmppd->next) tmppd = tmppd->next;
      tmppd->next=pd;
   }
   ENDMESSAGE(process_get_pd)
   return(pd);
}

/*------------------------------------------------------------*/
/* process_remove_pd */
/*------------------------------------------------------------*/

static void process_remove_pd(ProcessData pd)
{
   BEGINMESSAGE(process_remove_pd)
   if (gpd == pd) gpd = pd->next;
   else {
      ProcessData tmppd = gpd;
      while (tmppd->next != pd) tmppd=tmppd->next;
      tmppd->next = pd->next;
   }
   process_menu(pd,PROCESS_MENU_DEL_ENTRY);
   DESTROY_TIMER(pd->timer)
   XtFree(pd->name);
   XtFree((XtPointer)pd);
#  ifdef MESSAGES
      if (!gpd) { INFMESSAGE(no more processes registered) }
#  endif
   ENDMESSAGE(process_remove_pd)
}

/*------------------------------------------------------------*/
/* process_child_status */
/*------------------------------------------------------------*/

#define CHILD_UNKNOWN_STATUS -2
#define CHILD_ERROR          -1
#define CHILD_OKAY            0
#define CHILD_EXITED          1

static int process_child_status(ProcessData pd)
{
   int status;
   pid_t child_pid;

   BEGINMESSAGE(process_child_status)

      child_pid = waitpid(pd->pid,NULL,WNOHANG);
      if      (child_pid==pd->pid) { status=CHILD_EXITED;         INFMESSAGE(CHILD_EXITED) }
      else if (child_pid==0)       { status=CHILD_OKAY;           INFMESSAGE(CHILD_OKAY) }
      else if (child_pid==-1)      { status=CHILD_ERROR;          INFMESSAGE(CHILD_ERROR) }
      else                         { status=CHILD_UNKNOWN_STATUS; INFMESSAGE(CHILD_UNKNOWN_STATUS) }
   ENDMESSAGE(process_child_status)
   return(status);
}


/*##############################################################*/
/* cb_processKillProcess */
/*##############################################################*/

void
cb_processKillProcess(Widget w _GL_UNUSED, XtPointer client_data, XtPointer call_data _GL_UNUSED)
{
   ProcessData pd;
   BEGINMESSAGE(cb_processKillProcess)
   pd = (ProcessData)client_data;
   process_kill_process(pd);
   ENDMESSAGE(cb_processKillProcess)
}

/*##############################################################*/
/* process_kill_process */
/*##############################################################*/

void process_kill_process(ProcessData pd)
{
   int status;
   BEGINMESSAGE(process_kill_process)
   kill(pd->pid, SIGTERM);
   status=process_child_status(pd);
   if (status==CHILD_OKAY) {
      INFMESSAGE(waiting for child to exit);
      wait(NULL);
   }
   (*(pd->notify_proc))(pd->data,PROCESS_KILL);
   process_remove_pd(pd);
   ENDMESSAGE(process_kill_process)
}

/*##############################################################*/
/* process_kill_all_processes */
/*##############################################################*/

void process_kill_all_processes(void)
{
   BEGINMESSAGE(process_kill_all_processes)
   while (gpd) process_kill_process(gpd);
   ENDMESSAGE(process_kill_all_processes)
}

/*------------------------------------------------------------*/
/* process_notify */
/*------------------------------------------------------------*/

static void process_notify(XtPointer client_data, XtIntervalId *idp _GL_UNUSED)
{
   pid_t child_pid;
   ProcessData pd = (ProcessData) client_data;

   BEGINMESSAGE(process_notify)
   child_pid = process_child_status(pd);
   if (child_pid==0) {
      INFMESSAGE(child did not exit yet)
      pd->timer = ADD_TIMEOUT(toplevel,CHECK_PERIOD,client_data);
      process_menu(pd,PROCESS_MENU_PROGRESS);
   } else {
      INFMESSAGE(calling notify procedure)
      (*(pd->notify_proc))(pd->data,PROCESS_NOTIFY);
      process_remove_pd(pd);
   }
   ENDMESSAGE(process_notify)
}

/*##############################################################*/
/* process_fork */
/*##############################################################*/

ProcessData process_fork(String name, String command, ProcessNotifyProc notify_proc, XtPointer data)
{
   ProcessData pd;
   pid_t       pid;

   BEGINMESSAGE(process_fork)

   pd  = process_get_pd();

   pid = fork();

   if (pid == 0) { /* child */
      char *argv[4];
      char *c;

      INFMESSAGE(child process)
      c = command;
      SMESSAGE(c)


      argv[0] = "sh";
      argv[1] = "-c";
      argv[2] = c;
      argv[3] = NULL;

      INFMESSAGE(spawning conversion process)
/*
      if (!freopen("/dev/null", "w", stdout)) perror("/dev/null");
*/
      if (!freopen("/dev/null", "r", stdin))  perror("/dev/null");

      if (gv_gs_safeDir) {
        if (chdir(gv_safe_gs_workdir) != 0) {
	  char buf[512];
          sprintf(buf, "Chdir to %s failed", gv_safe_gs_workdir);
	  perror(buf);
	  _exit(EXIT_STATUS_ERROR);
	}
      }
      execvp(argv[0], argv);

      {
         char tmp[512];
         sprintf(tmp, execOfFailedLabel, argv[0]);
         perror(tmp);
         _exit(EXIT_STATUS_ERROR);
      }
   }
   INFMESSAGE(parent process)
   pd->name        = XtNewString(name);
   pd->notify_proc = notify_proc;
   pd->data        = data;
   pd->pid         = pid;
   pd->timer       = ADD_TIMEOUT(toplevel,CHECK_PERIOD,pd);

   process_menu(pd,PROCESS_MENU_ADD_ENTRY);

   ENDMESSAGE(process_fork)
   return(pd);
}

/*------------------------------------------------------------*/
/* process_set_shell_resize */
/*------------------------------------------------------------*/

static Boolean process_set_shell_resize(Boolean allow_resize)
{
   Boolean old_allow_resize;
   ShellWidget sw = (ShellWidget)toplevel;
   old_allow_resize = sw->shell.allow_shell_resize;
   sw->shell.allow_shell_resize=allow_resize;
   return(old_allow_resize);
}

/*##############################################################*/
/* process_menu */
/*##############################################################*/

void process_menu(ProcessData pd, int action)
{

   Arg args[5];
   Cardinal n;
   static int visible=1;
   static int progress=0;

   if (action==PROCESS_MENU_HIDE) {
      INFMESSAGE(PROCESS_MENU_HIDE)
      if (visible) {
         Boolean allow_resize;

         allow_resize=process_set_shell_resize(False);
   					        n=0;
         XtSetArg(args[n], XtNwidth,   0);	n++;
         XtSetArg(args[n], XtNheight,  0);	n++;
         XtSetValues(processButton,args,n);

         process_set_shell_resize(allow_resize);

         visible=0;
         progress=0;
      }
   }
   else if (action==PROCESS_MENU_SHOW) {
      INFMESSAGE(PROCESS_MENU_SHOW)
      if (!visible) {
         String label;
         Boolean allow_resize;

         allow_resize=process_set_shell_resize(False);

                                                n=0;
         XtSetArg(args[n], XtNlabel,  &label);	n++;
         XtGetValues(processButton,args,n);
         label=XtNewString(label);
         SMESSAGE(label)

                                                n=0;
         XtSetArg(args[n], XtNresize,  True);	n++;
         XtSetArg(args[n], XtNlabel,   "");	n++;
         XtSetValues(processButton,args,n);

     					        n=0;
         XtSetArg(args[n], XtNlabel,   label);	n++;
         XtSetValues(processButton,args,n);
   					        n=0;
         XtSetArg(args[n], XtNresize,  False);	n++;
         XtSetValues(processButton,args,n);
         XtFree(label);

         process_set_shell_resize(allow_resize);
         visible=1; 
         progress=0;
      }
   }
   else if (action==PROCESS_MENU_ADD_ENTRY) {
      Widget entry;
      char label[512];
      INFMESSAGE(PROCESS_MENU_ADD_ENTRY)
      if (!processMenu) {
                                                n=0;
         processMenu = XtCreatePopupShell("menu",
                       simpleMenuWidgetClass,processButton,args,n);
      }
      sprintf(label,"Stop %s",pd->name);

          					n=0;
         XtSetArg(args[n], XtNlabel, label);    n++;      
      entry = XtCreateManagedWidget("aaa", smeBSBObjectClass,processMenu,args,n);
      XtAddCallback(entry, XtNcallback, cb_processKillProcess, (XtPointer)pd);
      pd->menuentry  = entry;
      process_menu(NULL,PROCESS_MENU_SHOW);
      process_menu(pd,PROCESS_MENU_PROGRESS);
   }
   else if (action==PROCESS_MENU_DEL_ENTRY) {
      INFMESSAGE(PROCESS_MENU_DEL_ENTRY)
      if (!gv_exiting) {
         if (!gpd) {
            INFMESSAGE(destroying processMenu)
            XtDestroyWidget(processMenu);
            processMenu=NULL;
            process_menu(NULL,PROCESS_MENU_HIDE);
         }
         else {
            INFMESSAGE(destroying menu entry)
            XtDestroyWidget(pd->menuentry);
         }
      }
   }
   else if (action==PROCESS_MENU_PROGRESS) {
      INFMESSAGE(PROCESS_MENU_PROGRESS)
      if (visible) {
         char *label;
         char *tmp;
         size_t len;
                                                n=0;
         XtSetArg(args[n], XtNlabel,  &label);	n++;
         XtGetValues(processButton,args,n);
         len = strlen(label);
         tmp = (char*) XtMalloc(len*sizeof(char)+1);
         strcpy(tmp,&(label[progress]));
         if (progress) {
            if (progress>(int)len) progress=(int)len;
            strncpy(&(tmp[(int)len-progress]),label,(size_t)progress);
         }
         tmp[len]='\0';
         progress++;
         if (progress==(int)len+1) progress=0;
         update_label(processButton,tmp);
         XtFree(tmp);
      }
   }
}

/*##############################################################*/
/* process_disallow_quit */
/*##############################################################*/

char *process_disallow_quit(void)
{
# define MAX_DISALLOW_QUIT_MESSAGE 512
  static char message[MAX_DISALLOW_QUIT_MESSAGE];
  ProcessData pd;
  int l;

  BEGINMESSAGE(process_disallow_quit)
  if (!gpd) {
    ENDMESSAGE(process_disallow_quit)
    return NULL;
  }
  strcpy(message,stillInProgressLabel);
  l = strlen(message);
  for (pd = gpd; pd ; pd = pd->next) {
      l = l + strlen(pd->name) + 1;
      if (l + 10 < MAX_DISALLOW_QUIT_MESSAGE) { 
        strcat(message,"\n");
	strcat(message,pd->name);
      } else break;
  };
  if (pd) strcat(message,"\n...");
  ENDMESSAGE(process_disallow_quit)
  return message;
}


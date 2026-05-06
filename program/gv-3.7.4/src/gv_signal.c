/*
**
** gv_signal.c
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
#include <ac_config.h>

#include "message.h"

#include <stdio.h>
#include <signal.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>

#include "paths.h"
#include INC_X11(Intrinsic.h)


#ifdef USE_SIGNAL_HANDLER
#   if (XtSpecificationRelease >= 6)
#      define USE_XT_SIGNAL_HANDLER
#   else
#      define USE_DUMMY_SIGNAL_HANDLER
#   endif
#else
#   define USE_DUMMY_SIGNAL_HANDLER
#endif   

#include "types.h"
#include "callbacks.h"
#include "main_resources.h"
#include "main_globals.h"
#include "gv_signal.h"
#include "actions.h"

#ifdef SIGNALRETURNSINT
#   define SIGVAL int
#   define SIGNAL_RETURN return(0);
#else
#   define SIGVAL void
#   define SIGNAL_RETURN
#endif


#ifdef USE_XT_SIGNAL_HANDLER


static XtSignalId sid_updateFile = 0;
static XtSignalId sid_terminate  = 0;
static int block_terminate       = 0;
static int block_updateFile      = 0;
static pid_t gv_pid = (pid_t) 0;

/*------------------------------------------------------------*/
/* signal_signalIsOk */
/*------------------------------------------------------------*/

static int signal_signalIsOk(void)
{
  BEGINMESSAGE(signal_signalIsOk)
# ifdef MESSAGES
    IIMESSAGE(gv_pid,getpid())
    if (gv_pid == getpid()) { INFMESSAGE(signal is ok) }
    else                    { INFMESSAGE(bogus signal) }
# endif
  ENDMESSAGE(signal_signalIsOk)
  return(gv_pid == getpid() ? 1 : 0);
}

/*------------------------------------------------------------*/
/* signal_scb_terminate */
/*------------------------------------------------------------*/

static void signal_scb_terminate(XtPointer client_data _GL_UNUSED, XtSignalId *sidP _GL_UNUSED)
{
  BEGINMESSAGE(signal_scb_terminate)
  cb_doQuit(NULL,NULL,NULL);
  ENDMESSAGE(signal_scb_terminate)
}

/*------------------------------------------------------------*/
/* signal_sh_terminate */
/*------------------------------------------------------------*/

static SIGVAL signal_sh_terminate(int sig)
{
  BEGINMESSAGE(signal_fatalSignalHandler)
  if (signal_signalIsOk()) {
    if (!block_terminate) {
      fprintf(stderr, "gv: terminated by signal %d\n", sig);
      XtNoticeSignal(sid_terminate);
      block_terminate = 1;
    }
    signal(sig,signal_sh_terminate);
  } else {
    INFMESSAGE(forcing bogus caller to die)
    clean_safe_tempdir();
    exit(EXIT_STATUS_ERROR);
  }
  ENDMESSAGE(signal_sh_terminate)
  SIGNAL_RETURN
}

/*------------------------------------------------------------*/
/* signal_scb_updateFile */
/*------------------------------------------------------------*/

static void signal_scb_updateFile(XtPointer client_data _GL_UNUSED, XtSignalId *sidP _GL_UNUSED)
{
  BEGINMESSAGE(signal_scb_updateFile)
  if (gv_filename) {
    String s;
    struct stat sbuf;
    s = XtNewString(gv_filename);
    if (!stat(s, &sbuf) && mtime != sbuf.st_mtime)
       cb_checkFile(NULL,(XtPointer)CHECK_FILE_VERSION,NULL);
    XtFree(s);
  }
  block_updateFile = 0;
  ENDMESSAGE(signal_scb_updateFile)
}

/*------------------------------------------------------------*/
/* signal_sh_updateFile */
/*------------------------------------------------------------*/

static SIGVAL signal_sh_updateFile(int sig)
{
  BEGINMESSAGE(signal_sh_updateFile)
  if (signal_signalIsOk()) {
    if (!block_updateFile) {
      block_updateFile=1; 
      XtNoticeSignal(sid_updateFile);
    }
    signal(sig,signal_sh_updateFile);
  } else {
    INFMESSAGE(forcing bogus caller to die)
    clean_safe_tempdir();
    exit(EXIT_STATUS_ERROR);
  }
  ENDMESSAGE(signal_sh_updateFile)
  SIGNAL_RETURN
}

/*############################################################*/
/* signal_setSignalHandlers */
/*############################################################*/

void signal_setSignalHandlers(int on)
{
  SIGVAL (*sht)(int),(*shu)(int);

  BEGINMESSAGE(signal_setSignalHandlers)

  if (!gv_pid) gv_pid = getpid();

  if (on) {
    shu = signal_sh_updateFile;
    sht = signal_sh_terminate;
    sid_updateFile = XtAppAddSignal(app_con,signal_scb_updateFile,NULL);
    sid_terminate  = XtAppAddSignal(app_con,signal_scb_terminate,NULL);
  } else {
    shu = sht = SIG_DFL;
    if (sid_updateFile) XtRemoveSignal(sid_updateFile);
    if (sid_terminate)  XtRemoveSignal(sid_terminate);
    sid_updateFile = sid_terminate = 0;
  }
  signal(SIGHUP,  shu);
  signal(SIGQUIT, sht);
  signal(SIGTERM, sht);
#if 0
  signal(SIGINT,  sht);
  signal(SIGILL,  sht);
  signal(SIGBUS,  sht);
  signal(SIGFPE,  sht);
  signal(SIGSEGV, sht);
  signal(SIGSYS,  sht);
#endif
  ENDMESSAGE(signal_setSignalHandlers)
}

#endif /* USE_XT_SIGNAL_HANDLER */


#ifdef USE_DUMMY_SIGNAL_HANDLER

int signaldummy;

#endif /* USE_DUMMY_SIGNAL_HANDLER */

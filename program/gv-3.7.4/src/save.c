/*
**
** save.c
**
** Copyright (C) 1995, 1996, 1997 Johannes Plass
** Copyrigth (C) 2004 Jose E. Marchesi
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

#include <stdlib.h>
#include <stdio.h>

#include "paths.h"
#include INC_X11(Intrinsic.h)
#include "Ghostview.h"

#include "types.h"
#include "config.h"
#include "error.h"
#include "file.h"
#include "main_resources.h"
#include "main_globals.h"
#include "misc.h"
#include "note.h"
#include "process.h"
#include "ps.h"
#include "doc_misc.h"
#include "save.h"


#   include <sys/types.h>
#   include <unistd.h>


/*############################################################*/
/* save_alllocSaveData */
/*############################################################*/

SaveData
save_allocSaveData(void)
{
   SaveData sd;
   Cardinal size = sizeof(SaveDataStruct);

   BEGINMESSAGE(save_alllocSaveData)
   sd = (SaveData) XtMalloc(size);
   memset((void*) sd ,0,(size_t)size);
   ENDMESSAGE(save_allocSaveData)
   return(sd);
}

/*############################################################*/
/* save_freeSaveData */
/*############################################################*/

void
save_freeSaveData(SaveData sd)
{
   BEGINMESSAGE(save_freeSaveData)
   XtFree(sd->save_fn);
   XtFree(sd->src_fn);
   XtFree(sd->conv_fn);
   XtFree(sd->pagelist);
   XtFree(sd->print_cmd);
   XtFree((XtPointer)sd);
   ENDMESSAGE(save_freeSaveData)
}

/*------------------------------------------------------------*/
/* print_file */
/*------------------------------------------------------------*/


#   define SYSTEM_SUCCEEDED_ON(bbb)  (system(bbb) == 0)
#   define SYSTEM_FAILED_ON(bbb)     (system(bbb) != 0)


static String
print_file(String print_command, String print_filename)
{
   String error=NULL;
   char *print_quoted_filename;
   char *c,*p;
   Cardinal m,n;
   String printfail=GV_ERROR_PRINT_FAIL;

   BEGINMESSAGE(print_file)

   print_quoted_filename = quote_filename(print_filename);
   p = XtNewString(print_command);
   n=0;
   c=p;
   while ((c=strstr(c,"%s"))) { c+=2; n++; }
   m = (strlen(p)+(n>0?n:1)*strlen(print_quoted_filename)+5)*sizeof(char);
   c = (char*) XtMalloc(m);
   if (n>0) {
     char *e,*s;
     e=s=p;
     c[0]='\0';
     while (s) {
       s=strstr(e,"%s");
       if (s) *s='\0';
       strcat(c,e);
       if (s) {
	 strcat(c,print_quoted_filename);
         e=s+2;
       } 
       else s=NULL;
     }
   } else {
     sprintf(c, "%s %s",p,print_quoted_filename);
   }
   INFSMESSAGE(printing:,c)
   if (SYSTEM_FAILED_ON(c)) {
     m = (strlen(printfail)+strlen(c)+1)*sizeof(char);
       error = (char*) XtMalloc(m);
     sprintf(error,printfail,c); 
   }
   XtFree(c);
   XtFree(p);
   XtFree(print_quoted_filename);
   ENDMESSAGE(print_file)
   return(error);
}

/*------------------------------------------------------------*/
/* save_forkPDFToPSConversionDone */
/*------------------------------------------------------------*/

static void
save_forkPDFToPSConversionDone(XtPointer client_data, int type)
{
   char *error=NULL;
   SaveData sd = (SaveData) client_data;

   BEGINMESSAGE(save_forkPDFToPSConversionDone)

   if (type==PROCESS_NOTIFY) {
      INFMESSAGE(call is of type PROCESS_NOTIFY)
      error = save_saveFile(sd);
      if (error) {
         NotePopupShowMessage(error);
         XtFree(error);
      }
   } else if (type==PROCESS_KILL) {
      INFMESSAGE(call is of type PROCESS_KILL)
      if (sd->conv_fn)  {
         INFSMESSAGE(deleting file, sd->conv_fn)
         unlink(sd->conv_fn);
      }
      save_freeSaveData(sd);
   }

   ENDMESSAGE(save_forkPDFToPSConversionDone)
}

/*------------------------------------------------------------*/
/* save_forkPDFToPSConversion */
/*------------------------------------------------------------*/

static String
save_forkPDFToPSConversion(SaveData sd)
{
   char command[512], tmp[512];
   char proc_name[256];
   char *error=NULL;
   char *pos;
   char *pdfpos;
   char *pspos;
   char *quoted_src_fn, *quoted_conv_fn;
   char* src = command;
   char* dest = tmp;
   int spaceFound = 0;

   BEGINMESSAGE(save_forkPDFToPSConversion)

   pos=file_locateFilename(sd->src_fn);
   strcpy(proc_name,pos);
   strcat(proc_name," conversion");

   quoted_src_fn = quote_filename(sd->src_fn);
   quoted_conv_fn = quote_filename(sd->conv_fn);
   if ((pdfpos = strstr(gv_gs_cmd_conv_pdf,"%pdf")) &&
       (pspos = strstr(gv_gs_cmd_conv_pdf,"%ps"))) {
     command[0] = '\0';
     if (pdfpos < pspos) {
       strncat(command,gv_gs_cmd_conv_pdf,(pdfpos-gv_gs_cmd_conv_pdf));
       strcat(command,quoted_src_fn);
       strncat(command,pdfpos+4,(pspos-pdfpos-4));
       strcat(command,quoted_conv_fn);
       strcat(command,pspos+3);
     } else {
       strncat(command,gv_gs_cmd_conv_pdf,(pspos-gv_gs_cmd_conv_pdf));
       strcat(command,quoted_conv_fn);
       strncat(command,pspos+3,(pdfpos-pspos-3));
       strcat(command,quoted_src_fn);
       strcat(command,pdfpos+4);
     }
   } else {
     sprintf(command,gv_gs_cmd_conv_pdf,quoted_conv_fn,quoted_src_fn);
   }
   XtFree(quoted_src_fn);
   XtFree(quoted_conv_fn);
   
   if (strstr(gv_gs_cmd_scan_pdf, "-P") || !gv_gs_safer)
      strcpy(tmp, gv_gs_cmd_scan_pdf);
   else
   {
      while (*src)
      {
         int isSpace = *src == ' ';
         *(dest++) = *(src++);
	 if (!spaceFound && isSpace)
	 {
	    strcpy(dest, "-P- -dSAFER ");
	    dest+=12;
	    spaceFound = 1;
	 }
      }
      *dest = 0;
   }

   INFSMESSAGE(starting conversion:,tmp)
   process_fork(proc_name,tmp,save_forkPDFToPSConversionDone,(XtPointer)sd);
   ENDMESSAGE(save_forkPDFToPSConversion)
   return(error);
}

/*------------------------------------------------------------*/
/* save_copyToFile */
/*------------------------------------------------------------*/

static String
save_copyToFile(String save_filename, String src_filename, char *pagelist, int scanstyle)
{
   FILE *save_file=NULL;
   FILE *src_file=NULL;
   String error=NULL;
   String openfail=GV_ERROR_OPEN_FAIL;
   char *tmp;

   BEGINMESSAGE(save_copyToFile)

   if ((save_file = fopen(save_filename, "w")) == NULL) 
      error = open_fail_error(errno,openfail,save_filename,0);
   if (!error && (src_file=fopen(src_filename,"r"))==NULL)
      error = open_fail_error(errno,openfail,src_filename,0);
   if (!error) {
      if (pagelist) {
         Document src_doc=NULL;
	 String s = XtNewString(src_filename);
	 s = file_getUsefulName(s);
         INFMESSAGE(scanning document)
         doc_scanFile(&src_file,&src_doc,src_filename,s,NULL,NULL,NULL,NULL,scanstyle,gv_gs_safeDir);
         if (src_doc) {
            INFMESSAGE(calling pscopydoc)
            pscopydoc(save_file,src_filename,src_doc,pagelist);
            psfree(src_doc);
         } else {
            char *error_scan_fail = "Failed to scan file %s\n";
            char tmp[512];
            sprintf(tmp,error_scan_fail,src_filename);
            error=XtNewString(tmp);
         }
      } else {
         char buf[BUFSIZ];
         int  bytes;
         INFMESSAGE(copying file literaly)
         while ((bytes = read(fileno(src_file),buf,BUFSIZ)))
               bytes = write(fileno(save_file), buf, bytes);
      }
   }
   tmp = close_file(src_file,src_filename);   if (!error) error = tmp;
   tmp = close_file(save_file,save_filename); if (!error) error = tmp;

   ENDMESSAGE(save_copyToFile)
   return(error);
}

/*############################################################*/
/* save_saveFile */
/*############################################################*/

String
save_saveFile(sd)
   SaveData sd;
{
   char *src_fn;
   String error=NULL,s;

   BEGINMESSAGE(save_saveFile)

   if (!sd->save_fn && !sd->print_cmd && sd->save_to_file) {
      INFMESSAGE(save_filename not useful)
      ENDMESSAGE(save_saveFile)
      return(error);
   }

   if (!error && sd->convert && !sd->conv_fn) 
     {

     s = sd->save_fn ? sd->save_fn : sd->src_fn;
     s = XtNewString(s);
     s = file_getUsefulName(s);
     s = file_pdfname2psname(s);
     sd->conv_fn = file_getTmpFilename(NULL, s, NULL);
     XtFree(s);
     if (sd->conv_fn == NULL)
	     return XtNewString("Cannot create temporary file!");
     INFSMESSAGE(converting from file,sd->src_fn)
     INFSMESSAGE(converting to file,sd->conv_fn)
     error = save_forkPDFToPSConversion(sd);
     ENDMESSAGE(save_saveFile)
     return(error);
   }

   src_fn  = sd->src_fn;
   if (sd->conv_fn) src_fn = sd->conv_fn;

   if (!error && sd->save_to_file) {
      if (!sd->save_fn) {
	s = XtNewString(sd->src_fn);
	s = file_getUsefulName(s);
	s = file_pdfname2psname(s);
	sd->save_fn = file_getTmpFilename(NULL, s, NULL);
	XtFree(s);
      }
      if (!sd->save_fn) {
        error = XtNewString("Cannot create temporary file!");
      } else {
        INFSMESSAGE(saving from file,src_fn)
        INFSMESSAGE(saving to file,sd->save_fn)
        error = save_copyToFile(sd->save_fn,src_fn,sd->pagelist,sd->scanstyle);
        src_fn = sd->save_fn;
      }
   }

   if (!error && sd->save_to_printer) {
      INFSMESSAGE(saving to printer from file,src_fn)
      error = print_file(sd->print_cmd,src_fn);
      if (src_fn != sd->src_fn && !(sd->print_kills_file)) {
         INFSMESSAGE(deleting file, src_fn)
         unlink(src_fn);
      }
   }

   if (sd->conv_fn)  {
      INFSMESSAGE(deleting file, sd->conv_fn)
      unlink(sd->conv_fn);
   }
   save_freeSaveData(sd);

   ENDMESSAGE(save_saveFile)
   return(error);
}

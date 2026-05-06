/*
 * Copyright (C) 2003-2005 The Free Software Foundation, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

/*
 * win32.c
 * - utility functions for cvs under win32
 *
 */

#include "config.h"

#include <ctype.h>
#include <stdio.h>
#include <conio.h>

#ifdef HAVE_PROCESS_H
# include <process.h>
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <winsock.h>
#include <stdlib.h>

#include "cvs.h"

void
init_winsock ()
{
    WSADATA data;

    if (WSAStartup (MAKEWORD (1, 1), &data))
    {
	fprintf (stderr, "cvs: unable to initialize winsock\n");
	exit (1);
    }
}

void
wnt_cleanup (void)
{
    if (WSACleanup ())
    {
#ifdef SERVER_ACTIVE
	if (server_active || error_use_protocol)
	    /* FIXME: how are we supposed to report errors?  As of now
	       (Sep 98), error() can in turn call us (if it is out of
	       memory) and in general is built on top of lots of
	       stuff.  */
	    ;
	else
#endif
	    fprintf (stderr, "cvs: cannot WSACleanup: %s\n",
		     sock_strerror (WSAGetLastError ()));
    }
}

unsigned sleep(unsigned seconds)
{
	Sleep(1000*seconds);
	return 0;
}

/*
 * Sleep at least useconds microseconds.
 */
int usleep(unsigned long useconds)
{
    /* Not very accurate, but it gets the job done */
    Sleep(useconds/1000 + (useconds%1000 ? 1 : 0));
    return 0;
}



char *win32getlogin()
{
    static char name[256];
    DWORD dw = 256;
    GetUserName (name, &dw);
    if (name[0] == '\0')
	return NULL;
    else
	return name;
}


#ifndef HAVE_GETPID
pid_t
getpid (void)
{
    return (pid_t) GetCurrentProcessId();
}
#endif

char *
getpass (const char *prompt)
{
    static char pwd_buf[128];
    size_t i;

    fputs (prompt, stderr);
    fflush (stderr);
    for (i = 0; i < sizeof (pwd_buf) - 1; ++i)
    {
	pwd_buf[i] = _getch ();
	if (pwd_buf[i] == '\r')
	    break;
    }
    pwd_buf[i] = '\0';
    fputs ("\n", stderr);
    return pwd_buf;
}

/*
**
** gv_messages.h
**
** Copyright (C) 1995, 1996, 1997 Johannes Plass
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
*/

char *message_usage = "\
Usage: gv [OPTION]... [FILE]\n\
PostScript and PDF viewer.\n\
  [-]-monochrome                      display document using only black and white\n\
  [-]-grayscale                       display document without colors\n\
  [-]-color                           display document as usual\n\
  [-]-safer                           start ghostscript in safe mode\n\
  [-]-nosafer                         do not start ghostscript in safe mode\n\
  [-]-safedir                         start ghostscript from a safe diretory\n\
  [-]-nosafedir                       do not start ghostscript from a safe diretory\n\
  [-]-quiet                           start ghostscript with the -dQUIET option\n\
  [-]-noquiet                         do not start ghostscript with the -dQUIET option\n\
  [-]-infoSilent                      do not show any messages in the info popup window\n\
  [-]-infoErrors                      do not show warning messages in the info popup window\n\
  [-]-infoAll                         do show all messages in the info popup window\n\
  [-]-arguments=ARGS                  start ghostscript with additional options as specified\n\
                                      by the string ARGS\n\
  [-]-page=LABEL                      display the page with label LABEL first\n\
  [-]-center                          the page should be centered automatically\n\
  [-]-nocenter                        the page should not be centered automatically\n\
  [-]-media=MEDIA                     selects the paper size to be used\n\
  [-]-orientation=ORIENTATION         sets the orientation of the page\n\
  [-]-scale=N|f.f                     selects the scale N, or arbitrary scale f.f\n\
  [-]-scalebase=N                     selects the scale base N\n\
  [-]-swap                            interchange the meaning of the orientations landscape\n\
                                      and seascape\n\
  [-]-noswap                          do not interchange the meaning of the orientation\n\
                                      landscape and seascape\n\
  [-]-antialias                       use antialiasing\n\
  [-]-noantialias                     do not use antialiasing\n\
  [-]-dsc                             dsc comments are respected\n\
  [-]-nodsc                           dsc comments are not respected\n\
  [-]-eof                             ignore the postscript EOF comment while scanning\n\
                                      documents\n\
  [-]-noeof                           do not ignore the postscript EOF comment while\n\
                                      scanning documents\n\
  [-]-pixmap                          use backing pixmap\n\
  [-]-nopixmap                        do not use backing pixmap\n\
  [-]-watch                           watch the document file for changes\n\
  [-]-nowatch                         do not watch the document file for changes\n\
  [-]-help                            print a help message and exit\n\
  [-]-usage                           print a usage message and exit\n\
  [-]-resize                          fit the size of the window to the size of the page\n\
  [-]-noresize                        do not fit the size of the window to the size of the page\n\
  -geometry [<width>][x<height>][{+-}<xoffset>{+-}<yoffset>]\n\
  [-]-ad=FILE                         read and use additional resources from FILE\n\
  [-]-style=FILE                      read and use additional resources from FILE. These resources\n\
                                      have lower priority than those provided on the context of --ad\n\
  [-]-password=PASSWORD               Sets the password for opening encrypted PDF files\n\
  [-]-spartan                         shortcut for --style=gv_spartan.dat\n\
  [-]-widgetless                      shortcut for --style=gv_widgetless.dat\n\
  [-]-fullscreen                      start in fullscreen mode (needs support from WM)\n\
  [-]-presentation                    Presentation mode (fullscreen, Fit to window,\n\
                                      widgetless and no resizing of window)\n\
  [-]-version                         show gv version and exit\n\
";

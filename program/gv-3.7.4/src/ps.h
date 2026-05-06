/*
 * ps.h -- Include file for PostScript routines.
 * Copyright (C) 1992  Timothy O. Theisen
 * Copyright (C) 2004 Jose E. Marchesi
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU gv; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 *   Author: Tim Theisen           Systems Programmer
 * Internet: tim@cs.wisc.edu       Department of Computer Sciences
 *     UUCP: uwvax!tim             University of Wisconsin-Madison
 *    Phone: (608)262-0438         1210 West Dayton Street
 *      FAX: (608)262-9777         Madison, WI   53706
*/

/* Constants used to index into the bounding box array. */

#define LLX 0
#define LLY 1
#define URX 2
#define URY 3

	/* Constants used to store keywords that are scanned. */
	/* NONE is not a keyword, it tells when a field was not set */

/*
enum {ATEND = -1, NONE = 0, PORTRAIT, LANDSCAPE, ASCEND, DESCEND, SPECIAL};
*/

#define ATEND        (-1)
#define NONE            0
#define PORTRAIT        1
#define LANDSCAPE       2
#define SEASCAPE        3
#define UPSIDEDOWN      4
#define ASCEND          5
#define DESCEND         6
#define SPECIAL         7
#define AUTOMATIC       8

#define PSLINELENGTH 257	/* 255 characters + 1 newline + 1 NULL */

typedef struct document {
#ifdef GV_CODE
    int  structured;                    /* toc will be useful */ 
    int  labels_useful;                 /* page labels are distinguishable, hence useful */ 
#endif
    int  epsf;				/* Encapsulated PostScript flag. */
    char *title;			/* Title of document. */
    char *date;				/* Creation date. */
    int  pageorder;			/* ASCEND, DESCEND, SPECIAL */
    gv_off_t beginheader, endheader, beginpreview, endpreview, begindefaults, enddefaults,
          beginprolog, endprolog, beginsetup, endsetup, begintrailer, endtrailer;
    gv_off_t lenheader;
    gv_off_t lenpreview;
    gv_off_t lendefaults;
    gv_off_t lenprolog;
    gv_off_t lensetup;
    gv_off_t lentrailer;
    int  boundingbox[4];
    int  default_page_boundingbox[4];
    int  orientation;			/* PORTRAIT, LANDSCAPE */
    int  default_page_orientation;	/* PORTRAIT, LANDSCAPE */
    int nummedia;
    struct documentmedia *media;
    Media default_page_media;
    int numpages;
    struct page *pages;
} *Document;

struct page {
    char *label;
    int  boundingbox[4];
    struct documentmedia *media;
    int  orientation;			/* PORTRAIT, LANDSCAPE */
    gv_off_t begin, end;			/* offsets into file */
    gv_off_t len;
};

	/* scans a PostScript file and return a pointer to the document
	   structure.  Returns NULL if file does not Conform to commenting
	   conventions . */

#define SCANSTYLE_NORMAL     0
#define SCANSTYLE_IGNORE_EOF (1<<0)
#define SCANSTYLE_IGNORE_DSC (1<<1)

Document				psscan (
    FILE **,
    char *,
    char *,
    char **,
    char *,
    char **,
    char *,
    int,     /* scanstyle */
    int	     /* gv_gs_scanDir */
);

void					psfree (
    struct document *
);

extern void				pscopydoc (
    FILE *,
    char *,
    Document,
    char *
);

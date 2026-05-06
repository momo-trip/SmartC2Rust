/* Copyright 2014-2020 Free Software Foundation, Inc.

This file is part of groff, the GNU roff typesetting system.

groff is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation, either version 3 of the License, or (at your
option) any later version.

groff is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>. */

/*
 * DviChar.h
 *
 * descriptions for mapping dvi names to
 * font indexes and back.  Dvi fonts are all
 * 256 elements (actually only 256-32 are usable).
 *
 * The encoding names are taken from X -
 * case insensitive, a dash separating the
 * CharSetRegistry from the CharSetEncoding
 */

# define DVI_MAX_SYNONYMS	10
# define DVI_MAP_SIZE		256
# define DVI_HASH_SIZE		256

typedef struct _dviCharNameHash {
	struct _dviCharNameHash	*next;
	const char		*name;
	int			position;
} DviCharNameHash;

typedef struct _dviCharNameMap {
    const char		*encoding;
    int			special;
    const char		*dvi_names[DVI_MAP_SIZE][DVI_MAX_SYNONYMS];
    DviCharNameHash	*buckets[DVI_HASH_SIZE];
} DviCharNameMap;

DviCharNameMap		*DviFindMap (char *);
void			DviRegisterMap (DviCharNameMap *);
#ifdef NOTDEF
char			*DviCharName (DviCharNameMap *, int, int);
#else
#define DviCharName(map,index,synonym)	((map)->dvi_names[index][synonym])
#endif
int			DviCharIndex (DviCharNameMap *, const char *);

// Local Variables:
// fill-column: 72
// mode: C
// End:
// vim: set cindent noexpandtab shiftwidth=2 textwidth=72:

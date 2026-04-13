#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#if ENABLE_NLS
# include "gettext.h"
#else
# ifndef gettext
#  define gettext(msgid) msgid
# endif
#endif
#ifndef _
# define _(msgid) gettext(msgid)
#endif
#ifndef N_
# define N_(s) s
#endif

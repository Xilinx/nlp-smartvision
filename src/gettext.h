/*
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *  Copyright (C) 2019 Red Hat Inc.
 *  Authors: Jaroslav Kysela <perex@perex.cz>
 */
 
#ifndef __MY_GETTEXT_H
#define __MY_GETTEXT_H

#ifdef USES_CURSES
#define ENABLE_NLS_TEST ENABLE_NLS_IN_CURSES
#else
#define ENABLE_NLS_TEST ENABLE_NLS
#endif

#if ENABLE_NLS_TEST
# include <libintl.h>
#else
# define gettext(msgid) (msgid)
# define textdomain(domain)
# define bindtextdomain(domain, dir)
#endif

#define _(msgid) gettext (msgid)
#define gettext_noop(msgid) msgid
#define N_(msgid) gettext_noop (msgid)

#endif /* __MY_GETTEXT_H */

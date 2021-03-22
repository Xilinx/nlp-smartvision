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
 
/* include/aconfig.h.in.  Generated from configure.ac by autoheader.  */

/* directory containing alsa configuration */
#undef DATADIR

/* Define to 1 if translation of program messages to the user's native
   language is requested. */
#undef ENABLE_NLS

/* Define if curses-based programs can show translated messages. */
#undef ENABLE_NLS_IN_CURSES

/* Define to 1 if you have the <alsa/mixer.h> header file. */
#undef HAVE_ALSA_MIXER_H

/* Define to 1 if you have the <alsa/pcm.h> header file. */
#undef HAVE_ALSA_PCM_H

/* Define to 1 if you have the <alsa/rawmidi.h> header file. */
#undef HAVE_ALSA_RAWMIDI_H

/* Define to 1 if you have the <alsa/seq.h> header file. */
#undef HAVE_ALSA_SEQ_H

/* Define to 1 if you have the <alsa/topology.h> header file. */
#undef HAVE_ALSA_TOPOLOGY_H

/* Define to 1 if you have the <alsa/use-case.h> header file. */
#undef HAVE_ALSA_USE_CASE_H

/* Define to 1 if you have the Mac OS X function CFLocaleCopyCurrent in the
   CoreFoundation framework. */
#undef HAVE_CFLOCALECOPYCURRENT

/* Define to 1 if you have the Mac OS X function CFPreferencesCopyAppValue in
   the CoreFoundation framework. */
#undef HAVE_CFPREFERENCESCOPYAPPVALUE

/* Have clock gettime */
#undef HAVE_CLOCK_GETTIME

/* Have curses set_escdelay */
#undef HAVE_CURSES_ESCDELAY

/* Define if the GNU dcgettext() function is already present or preinstalled.
   */
#undef HAVE_DCGETTEXT

/* Define to 1 if you have the <form.h> header file. */
#undef HAVE_FORM_H

/* Define if the GNU gettext() function is already present or preinstalled. */
#undef HAVE_GETTEXT

/* Define if you have the iconv() function and it works. */
#undef HAVE_ICONV

/* Define to 1 if you have the <inttypes.h> header file. */
#undef HAVE_INTTYPES_H

/* Define to 1 if you have the `asound' library (-lasound). */
#undef HAVE_LIBASOUND

/* Define to 1 if you have the `atopology' library (-latopology). */
#undef HAVE_LIBATOPOLOGY

/* Define to 1 if you have the `fftw3f' library (-lfftw3f). */
#undef HAVE_LIBFFTW3F

/* Define to 1 if you have the `m' library (-lm). */
#undef HAVE_LIBM

/* Define to 1 if you have the `pthread' library (-lpthread). */
#undef HAVE_LIBPTHREAD

/* Have librt */
#undef HAVE_LIBRT

/* Define to 1 if you have the `tinyalsa' library (-ltinyalsa). */
#undef HAVE_LIBTINYALSA

/* Define to 1 if you have the <memory.h> header file. */
#undef HAVE_MEMORY_H

/* Define to 1 if you have the <menu.h> header file. */
#undef HAVE_MENU_H

/* Define to 1 if you have the <panel.h> header file. */
#undef HAVE_PANEL_H

/* Define to 1 if you have the <samplerate.h> header file. */
#undef HAVE_SAMPLERATE_H

/* alsa-lib supports snd_seq_client_info_get_card */
#undef HAVE_SEQ_CLIENT_INFO_GET_CARD

/* alsa-lib supports snd_seq_client_info_get_pid */
#undef HAVE_SEQ_CLIENT_INFO_GET_PID

/* Define to 1 if you have the <stdint.h> header file. */
#undef HAVE_STDINT_H

/* Define to 1 if you have the <stdlib.h> header file. */
#undef HAVE_STDLIB_H

/* Define to 1 if you have the <strings.h> header file. */
#undef HAVE_STRINGS_H

/* Define to 1 if you have the <string.h> header file. */
#undef HAVE_STRING_H

/* Define to 1 if you have the <sys/stat.h> header file. */
#undef HAVE_SYS_STAT_H

/* Define to 1 if you have the <sys/types.h> header file. */
#undef HAVE_SYS_TYPES_H

/* Define to 1 if you have the <unistd.h> header file. */
#undef HAVE_UNISTD_H

/* Name of package */
#undef PACKAGE

/* Define to the address where bug reports for this package should be sent. */
#undef PACKAGE_BUGREPORT

/* Define to the full name of this package. */
#undef PACKAGE_NAME

/* Define to the full name and version of this package. */
#undef PACKAGE_STRING

/* Define to the one symbol short name of this package. */
#undef PACKAGE_TARNAME

/* Define to the home page for this package. */
#undef PACKAGE_URL

/* Define to the version of this package. */
#undef PACKAGE_VERSION

/* directory containing sample data */
#undef SOUNDSDIR

/* Define to 1 if you have the ANSI C header files. */
#undef STDC_HEADERS

/* Define to 1 if you can safely include both <sys/time.h> and <time.h>. */
#undef TIME_WITH_SYS_TIME

/* ALSA util version */
#undef VERSION

/* Define if FFADO library is available */
#undef WITH_FFADO

/* Enable large inode numbers on Mac OS X 10.5.  */
#ifndef _DARWIN_USE_64_BIT_INODE
# define _DARWIN_USE_64_BIT_INODE 1
#endif

/* Number of bits in a file offset, on hosts where this is settable. */
#undef _FILE_OFFSET_BITS

/* Define for large files, on AIX-style hosts. */
#undef _LARGE_FILES

/* Define to empty if `const' does not conform to ANSI C. */
#undef const

/* Define to `__inline__' or `__inline' if that's what the C compiler
   calls it, or to nothing if 'inline' is not supported under any name.  */
#ifndef __cplusplus
#undef inline
#endif
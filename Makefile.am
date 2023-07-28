#       Copyright (C) 1995-1996, 2000 Tomoaki HAYASAKA.
# 
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#


#
# Makefile for EUPPlayer
#

AM_CPPFLAGS = @AM_CPPFLAGS@

bin_PROGRAMS = eupplay

man_MANS = eupplay.1

eupplay_LDADD = -lstdc++

eupplay_SOURCES = eupplay.cc \
		eupplayer.cc eupplayer.h \
		eupplayer_towns.h \
		eupplayer_townsEmulator.cc eupplayer_townsEmulator.h \
		eupplayer_townsInternal.cc eupplayer_townsInternal.h \
		sintbl.cc sintbl.h

EXTRA_DIST = README-ja NEWS-ja mksintbl.c eupplay.1.in

CLEANFILES = sintbl

.PHONY: au aiff wav
au: eupplay
	cd examples; $(MAKE) au
aiff: eupplay
	cd examples; $(MAKE) aiff
wav: eupplay
	cd examples; $(MAKE) wav

sintbl.cc: mksintbl
	./mksintbl >$@

mksintbl: mksintbl.c
	$(CC) $(CFLAGS) $< -lm -o $@

eupplay.1: eupplay.1.in
	sed -e 's/@!VERSION!@/Version @VERSION@/' <$< >$@

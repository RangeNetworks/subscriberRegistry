#
# Copyright 2013, 2014 Range Networks, Inc.
#
# This software is distributed under the terms of the GNU Public License.
# See the COPYING file in the main directory for details.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

include $(top_srcdir)/Makefile.common

DESTDIR := 

ourlibs = \
	$(GLOBALS_LA) \
	$(COMMON_LA) \
	$(NODEMANAGER_LA)

sbin_PROGRAMS = sipauthserve comp128
bin_SCRIPTS = syslogextractor hexmapper 
confdir = /etc/OpenBTS
conf_DATA = sipauthserve.example.sql 

sipauthserve_SOURCES =  sipauthserve.cpp \
	../servershare.cpp \
	../SubscriberRegistry.cpp \
	../NodeManager/JSONDB/JSONDB.cpp
sipauthserve_LDADD = $(ourlibs) -losipparser2 -losip2

comp128_SOURCES = comp128.c
comp128_LDADD = $(ourlibs)

sipauthserve.example.sql: sipauthserve
	( ./sipauthserve --gensql > sipauthserve.example.sql || true )

tmp_DB = /tmp/SR.db
TESTS = checkdb.sh
CLEANFILES = $(tmp_DB)

EXTRA_DIST = \
	subscriberRegistry.example.sql \
	sipauthserve.conf

# Populating comp128 in both locations for backwards compat
install: sipauthserve comp128
	mkdir -p "$(DESTDIR)/OpenBTS/"
	install comp128 "$(DESTDIR)/OpenBTS/"
	install sipauthserve "$(DESTDIR)/OpenBTS/"
	mkdir -p "$(DESTDIR)/usr/local/bin/"
	install comp128 "$(DESTDIR)/usr/local/bin/"
	install syslogextractor "$(DESTDIR)/usr/local/bin/"
	install hexmapper "$(DESTDIR)/usr/local/bin/"
	mkdir -p "$(DESTDIR)/etc/supervisor/conf.d/"
	install sipauthserve.conf "$(DESTDIR)/etc/supervisor/conf.d/"
	mkdir -p "$(DESTDIR)/etc/OpenBTS/"
	install subscriberRegistry.example.sql "$(DESTDIR)/etc/OpenBTS/"

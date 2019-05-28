#
# PrlUuid.pro
#
# Copyright (c) 1999-2017, Parallels International GmbH
# Copyright (c) 2017-2019 Virtuozzo International GmbH. All rights reserved.
#
# This file is part of Virtuozzo SDK. Virtuozzo SDK is free
# software; you can redistribute it and/or modify it under the
# terms of the GNU Lesser General Public License as published by
# the Free Software Foundation; either version 2.1 of the License,
# or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library.  If not, see
# <http://www.gnu.org/licenses/>.
#
# Our contact details: Virtuozzo International GmbH, Vordergasse 59, 8200
# Schaffhausen, Switzerland.
#

TEMPLATE = lib
CONFIG += staticlib

include(PrlUuid.pri)

unix {
	DEFINES += \
		HAVE_UNISTD_H \
		HAVE_STDLIB_H \
		HAVE_SYS_IOCTL_H \
		HAVE_SYS_SOCKET_H \
		HAVE_NET_IF_H \
		HAVE_NETINET_IN_H

	INCLUDEPATH *= libuuid_unix
    SOURCES += \
		libuuid_unix/clear.c \
		libuuid_unix/compare.c \
		libuuid_unix/copy.c \
		libuuid_unix/gen_uuid.c \
		libuuid_unix/isnull.c \
		libuuid_unix/pack.c \
		libuuid_unix/parse.c \
		libuuid_unix/unpack.c \
		libuuid_unix/unparse.c \
		libuuid_unix/uuid_time.c
}


SOURCES += PrlUuid.cpp \
	Uuid.cpp
HEADERS += PrlUuid.h \
	PrlUuidCommon.h \
	Uuid.h

headers.files = $${HEADERS}
headers.path = $${PREFIX}/include/prlcommon/PrlUuid
INSTALLS += headers

HEADERS_UNIX = libuuid_unix/uuid.h

headers_unix.files = $${HEADERS_UNIX}
headers_unix.path = $${PREFIX}/include/prlcommon/PrlUuid/libuuid_unix
INSTALLS += headers_unix

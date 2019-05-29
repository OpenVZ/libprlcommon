#
# Messaging.pro
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

TEMPLATE     = lib
CONFIG += staticlib

include(Messaging.pri)

DIR_HEADERS = \
	$$PWD/CResult.h \
	$$PWD/CVmEventBase.h \
	$$PWD/CVmEvent.h \
	$$PWD/CVmEventParameter.h \
	$$PWD/CVmEventParameters.h \
	$$PWD/CVmEventParameterList.h \
	$$PWD/CVmEventValue.h \
	$$PWD/CVmBinaryEventParameter.h

HEADERS += $${DIR_HEADERS}

SOURCES += \
	$$PWD/CResult.cpp \
	$$PWD/CVmEventBase.cpp \
	$$PWD/CVmEvent.cpp \
	$$PWD/CVmEventParameter.cpp \
	$$PWD/CVmEventParameters.cpp \
	$$PWD/CVmEventParameterList.cpp \
	$$PWD/CVmEventValue.cpp \
	$$PWD/CVmBinaryEventParameter.cpp

headers_Messaging.files = $${DIR_HEADERS}
headers_Messaging.path = $${PREFIX}/include/prlcommon/Messaging
INSTALLS += headers_Messaging


#
# Copyright (c) 1999-2017, Parallels International GmbH
# Copyright (c) 2017-2021 Virtuozzo International GmbH. All rights reserved.
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

unix {
	VERSION = $$cat(../Makefile.version)
	VERSIONS = $$split(VERSION, ".")
	VER_MAJ = $$member(VERSIONS,0)
	VER_MIN = $$member(VERSIONS,1)
	VER_PAT = $$member(VERSIONS,2)
}

include(../common.pri)
include(../Libraries/CAuth/CAuth.pri)
include(../Libraries/Messaging/Messaging.pri)
include(../Libraries/PrlCommonUtilsBase/PrlCommonUtilsBase.pri)
include(../Libraries/Logging/Logging.pri)
include(../Libraries/Std/Std.pri)
include(../Libraries/PrlObjects/PrlObjects.pri)
include(../Libraries/PrlDataSerializer/PrlDataSerializer.pri)

PROJ_PATH = $$PWD
#include(../Build/qmake/build_target.pri)

TARGET = prlTestsUtils
LIBTARGET = prlTestsUtils
PROJ_FILE = $$PWD/TestsUtils.pro
QTCONFIG = core xml

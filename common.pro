#
# common.pro
#
# Copyright (c) 1999-2017, Parallels International GmbH
#
# This file is part of Parallels SDK. Parallels SDK is free
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
# Our contact details: Parallels International GmbH, Vordergasse 59, 8200
# Schaffhausen, Switzerland.
#

TEMPLATE = subdirs

include(Build/Options.pri)
include(common.pri)

TARGET = prlcommon
PROJ_PATH = $$PWD
include(Build/qmake/build_target.pri)

target.path = $${PREFIX}/lib64
INSTALLS += target

include(common.deps)

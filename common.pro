#
# common.pro
#
# Copyright (C) 1999-2015 Parallels IP Holdings GmbH
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
# Our contact details: Parallels IP Holdings GmbH, Vordergasse 59, 8200
# Schaffhausen, Switzerland.
#

TEMPLATE = subdirs

include(Build/Options.pri)
include(common.pri)

include($$LIBS_LEVEL/PrlCommonUtilsBase/PrlCommonUtilsBase.pri)
include($$LIBS_LEVEL/HostUtils/HostUtils.pri)
include($$LIBS_LEVEL/Logging/Logging.pri)
include($$LIBS_LEVEL/OpenSSL/OpenSSL.pri)
include($$LIBS_LEVEL/OpenSSL/utils/OpenSSL_utils.pri)
include($$LIBS_LEVEL/IOService/IOCommunication.pri)
include($$LIBS_LEVEL/IOService/IOService.pri)
include($$LIBS_LEVEL/IOService/src/Common/Common.pri)
include($$LIBS_LEVEL/IOService/src/IOCommunication/IOCommunication.pri)
include($$LIBS_LEVEL/PrlUuid/PrlUuid.pri)
include($$LIBS_LEVEL/PrlDataSerializer/PrlDataSerializer.pri)

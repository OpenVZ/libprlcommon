#
# PrlCommonUtilsBase.pro
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

TARGET = prl_common_utils_base
TEMPLATE = lib
CONFIG += staticlib

DEFINES += PRINTABLE_TARGET=cmn_utils_base

include(PrlCommonUtilsBase.pri)

HEADERS = ParallelsDirs.h \
		Common.h \
		SysError.h \
		PrlStringifyConsts.h \
            ParallelsDirsDefs.h \
            ParallelsDirsBase.h \
            CommandLine.h \
            OsInfo.h \
            CSimpleFileHelper.h \
            StringUtils.h \
            EnumToString.h \
            ErrorSimple.h \
            NetworkUtils.h \
            CFeaturesMatrix.h \
            CGuestOsesHelper.h \
            CommandConvHelper.h \
            countof.h \
            CHardDiskHelper.h \
            CUrlParser.h \
            netutils.h \
            Archive.h


SOURCES = ParallelsDirs.cpp \
		ParallelsNamespace.cpp \
		Common.cpp \
		SysError.cpp \
            ParallelsDirsBase.cpp \
            CommandLine.cpp \
            OsInfo.c \
            CSimpleFileHelper.cpp \
            StringUtils.cpp \
            EnumToString.cpp \
            NetworkUtils.cpp \
            CFeaturesMatrix.cpp \
            CGuestOsesHelper.cpp \
            CommandConvHelper.cpp \
            CHardDiskHelper.cpp \
            CUrlParser.cpp \
            netutils.cpp \
            Archive.cpp

SDK_VALUES = \
    $$SDK_HEADERS/prlsdk/PrlErrorsValues.h \
    $$SDK_HEADERS/prlsdk/PrlEventsValues.h \
    $$SDK_HEADERS/prlsdk/PrlEnums.h

QMAKE_EXTRA_COMPILERS += gen_sdk
gen_sdk.input = SDK_VALUES
gen_sdk.commands = $$LIBS_LEVEL/PrlCommonUtilsBase/GenStringify.py $$SDK_HEADERS
gen_sdk.output = PrlStringifyConsts.cpp
gen_sdk.variable_out = SOURCES
gen_sdk.CONFIG += combine
gen_sdk.depends += ./GenStringify.py

headers.files = $${HEADERS}
headers.path = $${PREFIX}/include/prlcommon/PrlCommonUtilsBase
INSTALLS += headers

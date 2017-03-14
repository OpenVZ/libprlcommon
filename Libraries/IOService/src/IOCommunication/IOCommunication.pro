#
# IOCommunication.pro
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

TEMPLATE = lib
CONFIG += staticlib warn_on thread

include(IOCommunication.pri)

HEADERS = \
          IOClient.h \
          IOClientInterface.h \
          IOConnection.h \
          IODataBuffer.h \
          IOProtocol.h \
          IOProtocolCommon.h \
          IORoutingTable.h \
          IORoutingTableHelper.h \
          IOSenderHandle.h \
          IOSendJob.h \
          IOSendJobInterface.h \
          IOServer.h \
          IOServerInterface.h \
          IOServerPool.h \
          \
          Socket/SocketClient_p.h \
          Socket/SocketListeners_p.h \
          Socket/SocketServer_p.h \
          Socket/Socket_p.h \
          IOSSLInterface.h \
          Socket/SslHelper.h

headers.files = $${HEADERS}
headers.path = $${PREFIX}/include/prlcommon/IOService/IOCommunication/
INSTALLS += headers

HEADERS_S = \
          Socket/SocketClient_p.h \
          Socket/SocketListeners_p.h \
          Socket/SocketServer_p.h \
          Socket/Socket_p.h \
          IOSSLInterface.h \
          Socket/SslHelper.h

headers_s.files = $${HEADERS_S}
headers_s.path = $${PREFIX}/include/prlcommon/IOService/IOCommunication/Socket/
INSTALLS += headers_s

HEADERS += $${HEADERS_S}

SOURCES = \
          IOClient.cpp \
          IODataBuffer.cpp \
          IOProtocol.cpp \
          IORoutingTable.cpp \
          IORoutingTableHelper.cpp \
          IOSendJob.cpp \
          IOServer.cpp \
          IOServerPool.cpp \
          \
          Socket/SocketClient_p.cpp \
          Socket/SocketServer_p.cpp \
          Socket/Socket_p.cpp \
          IOSSLInterface.cpp \
          Socket/SslHelper.cpp


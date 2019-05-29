/*
 * HostUtilsBase.cpp: Base part of host utils functions
 *
 * Copyright (c) 1999-2017, Parallels International GmbH
 * Copyright (c) 2017-2019 Virtuozzo International GmbH. All rights reserved.
 *
 * This file is part of Virtuozzo SDK. Virtuozzo SDK is free
 * software; you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License,
 * or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/> or write to Free Software Foundation,
 * 51 Franklin Street, Fifth Floor Boston, MA 02110, USA.
 *
 * Our contact details: Virtuozzo International GmbH, Vordergasse 59, 8200
 * Schaffhausen, Switzerland.
 */


#include "HostUtils.h"

#ifndef _WIN_
	#include <unistd.h>
#else
	#include <Windows.h>
#endif

#include "Libraries/PrlUuid/Uuid.h"
#include <QRegExp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/random_device.hpp>

/**
 * @function Sleep for a while
 * @brief Sleep for a while
 *
 * @param Sleep time in milliseconds
 *
 * @return none
 *
 * @author antonz@
 */
void HostUtils::Sleep(UINT uiMsec)
{
#ifdef _WIN_
	::Sleep(uiMsec);
#elif defined (_LIN_)
	usleep(uiMsec * 1000);
#endif
}

QString HostUtils::parseMacAddress(const QString& mac)
{
	return mac.toUpper().remove(QRegExp("[:\\-\\.]"));
}

QString HostUtils::generateMacAddress (HostUtils::MacPrefixType prefix)
{
	QString macAddress;
	boost::random::random_device g;
	boost::uniform_int<quint32> d(16, (1<<24) - 1);

	QString macPrefix;
	switch ( prefix ) {
	case MAC_PREFIX_CT:
		macPrefix = "001851";
		break;
	case MAC_PREFIX_VM:
		macPrefix = "001C42";
		break;
	};
	macAddress.sprintf("%06X", d(g));
	return macPrefix + macAddress;
}

QString HostUtils::generateHostInterfaceName(const QString& mac)
{
	if (!checkMacAddress(mac, false))
		return QString();

	return QString("vme") + mac.toLower();
}

bool HostUtils::checkMacAddress(const QString &sMacAddress, bool bCheckPrlAddress)
{
	//Check MAC address length
	if (sMacAddress.size() != 12)
		return (false);

	//Check MAC address consistency
	bool bOk = false;
	qlonglong nMac = sMacAddress.toLongLong(&bOk, 16);
	//Check whether all symbols are proper hexadecimal digits
	if (!bOk)
		return (false);

	//Check whether non null valie specified
	if (!nMac)
		return (false);

	//Check small MAC address bit not set
	int nFirstByte = sMacAddress.left(2).toInt(0, 16);
	if ((nFirstByte & 1) != 0)
		return (false);

	//Check whether value belongs to VZ MAC addresses interval
	if ( bCheckPrlAddress
		 && !sMacAddress.startsWith( HostUtils::generateMacAddress().left( 6 ), Qt::CaseInsensitive ) )
		return (false);

	return (true);
}


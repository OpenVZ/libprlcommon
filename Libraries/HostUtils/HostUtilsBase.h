/*
 * HostUtilsBase.h: Base part of host utils functions.
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


#ifndef HostUtilsBase_H
#define HostUtilsBase_H

	/**
	 * Sleep for a while
	 */
	static void Sleep(UINT uiMsec);

	/** Prefix types for generated MAC-address */
	enum MacPrefixType {
		/** MAC-address prefix for containers */
		MAC_PREFIX_CT,
		/** MAC-address prefix for virtual machines */
		MAC_PREFIX_VM,
	};

	/**
	* generate VZ mac address
	*/
	static QString parseMacAddress (const QString& mac);
	static QString generateMacAddress (HostUtils::MacPrefixType prefix = HostUtils::MAC_PREFIX_VM);
	static QString generateHostInterfaceName (const QString& mac);

	/**
	* Checks whether specified value is proper MAC address
	* @param sMacAddress MAC address as string
	* @param bCheckPrlAddress check on VZ MAC address group
	*/
	static bool checkMacAddress(const QString &sMacAddress, bool bCheckPrlAddress);

#endif


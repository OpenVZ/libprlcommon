/*
 * CHardDiskHelper.h: Class helper for the hard disk additional
 * verification.
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


#ifndef HARD_DISK_HELPER_H
#define HARD_DISK_HELPER_H

class CHardDiskHelper
{
public:

	static bool isEfiPartition(unsigned int nPartitionType);

	static bool isBootCampPartition(unsigned int nPartitionType);

	static bool isSataSupportedForOs( unsigned int ioOsType, unsigned int uiOsVer );

	static bool isSataSupportedForChipset( unsigned int chipsetType );

	static bool isSataSupportedForVmConfig( unsigned int osType, unsigned int osVersion, unsigned int chipsetType );
};


#endif	// HARD_DISK_HELPER_H

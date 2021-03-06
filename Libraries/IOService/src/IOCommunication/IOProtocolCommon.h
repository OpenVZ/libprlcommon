/*
 * IOProtocolCommon.h: Common IO protocol definitions.
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


#ifndef IOPROTOCOLCOMMON_H
#define IOPROTOCOLCOMMON_H

#include "../../Interfaces/VirtuozzoTypes.h"

namespace IOService
{
	// IP version
	enum IPVersion
	{
		IPv4 = 0,
		IPv6
	};

	namespace IOCommunication
	{
#include "../../Interfaces/packed.h"
		/** First struct, which we receive from other side. */
		struct ProtocolVersion
		{
			char    whoWeAre[4];   /**< Just magic 4 bytes */
			UINT16	majorNumber;   /**< Major number */
			UINT16	minorNumber;   /**< Minor number */
			char    whatWeAre[64]; /**< Full description (build info, etc) */
		} PACKED;
#include "../../Interfaces/unpacked.h"
	}

}

#endif

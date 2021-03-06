/*
 * CommandConvHelper.h: Command converter from SDK to dispatcher
 * constants and vice versa
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


#ifndef COMMAND_CONV_HELPER_H
#define COMMAND_CONV_HELPER_H

#include "PrlStringifyConsts.h"
#include "../Interfaces/VirtuozzoNamespace.h"
#include <prlsdk/PrlEnums.h>

/**
 * Small helper that converts dispatcher protocol commands IDs into
 * PRL_JOB_OPERATION_CODE values
 */
struct DispatcherCmdsToJobTypeConverter {
	/**
	 * Converts specified dispatcher protocol ID into corresponding PRL_JOB_OPERATION_CODE value
	 * @param dispatcher protocol command identifier
	 * @returns converted PRL_JOB_OPERATION_CODE value
	 */
	static PRL_JOB_OPERATION_CODE Convert(PVE::IDispatcherCommands nCmdId);
	/**
	 * Converts specified PRL_JOB_OPERATION_CODE value into dispatcher protocol ID
	 * @param PRL_JOB_OPERATION_CODE value
	 * @returns dispatcher protocol command identifier
	 */
	static PVE::IDispatcherCommands Convert(PRL_JOB_OPERATION_CODE nJobOpCode);
};

#endif	// COMMAND_CONV_HELPER_H

/*
 * Copyright (C) 1999-2016 Parallels IP Holdings GmbH
 *
 * This file is part of Parallels SDK. Parallels SDK is free
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
 * Our contact details: Parallels IP Holdings GmbH, Vordergasse 59, 8200
 * Schaffhausen, Switzerland.
 */

#include <prlsdk/PrlErrors.h>

#include <ploop/libploop.h>

#include "PloopDisk.h"

namespace VirtualDisk
{
///////////////////////////////////////////////////////////////////////////////
// struct Ploop

PRL_RESULT Ploop::open(const QString& fileName,
		const PRL_DISK_OPEN_FLAGS flags)
{
	Q_UNUSED(fileName)
	Q_UNUSED(flags)

	return PRL_ERR_SUCCESS;
}

PRL_RESULT Ploop::read(void *data, PRL_UINT32 sizeBytes,
		PRL_UINT64 offSec)
{
	Q_UNUSED(data)
	Q_UNUSED(sizeBytes)
	Q_UNUSED(offSec)
	return PRL_ERR_SUCCESS;
}

PRL_RESULT Ploop::write(const void *data, PRL_UINT32 sizeBytes,
		PRL_UINT64 offSec)
{
	Q_UNUSED(data)
	Q_UNUSED(sizeBytes)
	Q_UNUSED(offSec)
	return PRL_ERR_SUCCESS;
}

PRL_RESULT Ploop::close(void)
{
	return PRL_ERR_SUCCESS;
}

} // namespace VirtualDisk

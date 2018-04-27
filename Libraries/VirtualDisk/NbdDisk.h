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
#ifndef __VIRTUAL_DISK_NBD__
#define __VIRTUAL_DISK_NBD__

#include <QString>

#include "VirtualDisk.h"
#include "Util.h"

struct nbd_client;
struct nbd_functions;

namespace VirtualDisk
{
///////////////////////////////////////////////////////////////////////////////
// struct Nbd

struct NbdDisk : Format
{
	NbdDisk();
	~NbdDisk();

	virtual PRL_RESULT open(const QString &fileName,
			const PRL_DISK_OPEN_FLAGS flags,
			const policyList_type &policies = policyList_type());
	virtual PRL_RESULT read(void *data, PRL_UINT32 sizeBytes,
			PRL_UINT64 offSec);
	virtual PRL_RESULT write(const void *data, PRL_UINT32 sizeBytes,
			PRL_UINT64 offSec);
	virtual Parameters::disk_type getInfo(void);
	virtual PRL_RESULT close(void);

	virtual PRL_RESULT cloneState(const QString &uuid,
			const QString &target);
	virtual CSparseBitmap *getUsedBlocksBitmap(UINT32 granularity,
			PRL_RESULT &err);
	virtual CSparseBitmap *getTrackingBitmap();

private:
	struct nbd_client    *m_clnt;
	struct nbd_functions *m_nbd;

	QString m_fileName;
	bool m_connected;
};

} // namespace VirtualDisk
#endif // __VIRTUAL_DISK_PLOOP__

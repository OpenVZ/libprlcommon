/*
 * Copyright (c) 2018 Virtuozzo International GmbH.  All rights reserved.
 *
 * This file is part of OpenVZ. OpenVZ is free software;
 * you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation;
 * either version 2 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 * Our contact details: Virtuozzo International GmbH, Vordergasse 59, 8200
 * Schaffhausen, Switzerland.
 */
#ifndef __VIRTUAL_DISK_NBD__
#define __VIRTUAL_DISK_NBD__

#include <QString>
#include <QScopedPointer>

#include "VirtualDisk.h"
#include "SparseBitmap.h"
#include "Util.h"

enum {
	SECTOR_SIZE = 512,
	DEFAULT_GRANULARITY = 128,
	DEFAULT_BLK_STATUS_RANGE = 2*1024*1024*1024U,
};

namespace VirtualDisk
{
///////////////////////////////////////////////////////////////////////////////

// struct NbdClient
class NbdContext;

// struct NbdDisk
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
	virtual CSparseBitmap *getTrackingBitmap(const QString& uuid);

private:
	/* Policy base:allocation*/
	struct Allocation
	{
		enum {
			NBD_STATE_HOLE = 1,
			NBD_STATE_ZERO = 2,
		};

		const char *getName() const
		{
			return "base:allocation";
		}
		const Uuid& getUuid() const
		{
			return m_uuid;
		}
		bool operator()(int flags) const
		{
			return flags & (NBD_STATE_HOLE|NBD_STATE_ZERO);
		}
	private:
		Uuid m_uuid;
	};
	/* Policy qemu:dirty-bitmap */
	struct Dirty
	{
		enum {
			NBD_STATE_DIRTY = 1,
		};

		explicit Dirty(const QString& uuid) : m_uuid(uuid)
		{
			m_name = QString("qemu:dirty-bitmap:").append(uuid).toUtf8();
		}
		const char *getName() const
		{
			return m_name.constData();
		}
		const Uuid& getUuid() const
		{
			return m_uuid;
		}
		bool operator()(int flags) const
		{
			return flags != NBD_STATE_DIRTY;
		}
	private:
		QByteArray m_name;
		Uuid m_uuid;
	};
	struct Bitmap : private QScopedPointer<CSparseBitmap>
	{
		Bitmap(NbdContext *nbd) : m_nbd(nbd) { }
		template <class T>
		PRL_RESULT operator()(T, int granularity = DEFAULT_GRANULARITY);
		using QScopedPointer<CSparseBitmap>::take;
	private:
		template <class T>
		static void setRange(PRL_UINT64 offs, PRL_UINT32 size, PRL_UINT32 flags, void *arg);

		NbdContext *m_nbd;
	};

	QUrl	m_url;
	QString	m_uuid;

	QScopedPointer<NbdContext> m_nbd;
};

} // namespace VirtualDisk
#endif // __VIRTUAL_DISK_PLOOP__

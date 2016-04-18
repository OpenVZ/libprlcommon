///////////////////////////////////////////////////////////////////////////////
///
/// @file Qcow2Disk.h
///
/// VirtualDisk implementation for qcow2.
///
/// @author mperevedentsev
///
/// Copyright (c) 2016 Parallels IP Holdings GmbH
///
/// This file is part of Virtuozzo SDK. Virtuozzo SDK is free
/// software; you can redistribute it and/or modify it under the terms
/// of the GNU General Public License as published by the Free Software
/// Foundation; either version 2 of the License, or (at your option) any
/// later version.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program; if not, write to the Free Software
/// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
/// 02110-1301, USA.
///
/// Our contact details: Parallels IP Holdings GmbH, Vordergasse 59, 8200
/// Schaffhausen, Switzerland.
///
///////////////////////////////////////////////////////////////////////////////
#ifndef __VIRTUAL_DISK_QCOW2__
#define __VIRTUAL_DISK_QCOW2__

#include <QSharedPointer>
#include <QProcess>

#include "VirtualDisk.h"
#include "Util.h"

namespace VirtualDisk
{
namespace Nbd
{

///////////////////////////////////////////////////////////////////////////////
// Driver

struct Driver
{
	static QStringList getDeviceList();

private:
	static PRL_RESULT insertModule();
};

struct Qemu;
typedef Prl::Expected<QSharedPointer<Qemu>, Error::Simple> qemu_type;

///////////////////////////////////////////////////////////////////////////////
// Qemu

struct Qemu
{
	static qemu_type create();

	Qemu()
	{
	}

	~Qemu()
	{
		disconnect();
	}

	PRL_RESULT setDevice(const QString &device);
	PRL_RESULT setImage(const QString &image, bool readOnly,
	                    PRL_UINT64 offset = 0);

	PRL_RESULT disconnect();

	const QString& getDevice() const
	{
		return m_device;
	}

private:
	QString m_device;
	QProcess m_process;
};

} // namespace Nbd

///////////////////////////////////////////////////////////////////////////////
// Qcow2

struct Qcow2: Format
{
	virtual ~Qcow2()
	{
		close();
	}

	static PRL_RESULT create(const QString &fileName,
	                         const Parameters::Disk &params);

	virtual PRL_RESULT open(const QString &fileName,
	                        const PRL_DISK_OPEN_FLAGS flags,
	                        const policyList_type &policies = policyList_type());
	virtual PRL_RESULT read(void *data, PRL_UINT32 sizeBytes,
	                        PRL_UINT64 offSec);
	virtual PRL_RESULT write(const void *data, PRL_UINT32 sizeBytes,
	                         PRL_UINT64 offSec);
	virtual Parameters::disk_type getInfo();
	virtual PRL_RESULT close(void);
	static bool isValid(const QString &fileName);

private:
	void closeForce();

	QString m_fileName;
	bool m_readOnly;
	QSharedPointer<Nbd::Qemu> m_device;
	IO::File m_file;
};

} // namespace VirtualDisk

#endif // __VIRTUAL_DISK_QCOW2__

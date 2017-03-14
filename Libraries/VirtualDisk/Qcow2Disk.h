///////////////////////////////////////////////////////////////////////////////
///
/// @file Qcow2Disk.h
///
/// VirtualDisk implementation for qcow2.
///
/// @author mperevedentsev
///
/// Copyright (c) 2016-2017, Parallels International GmbH
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
/// Our contact details: Parallels International GmbH, Vordergasse 59, 8200
/// Schaffhausen, Switzerland.
///
///////////////////////////////////////////////////////////////////////////////
#ifndef __VIRTUAL_DISK_QCOW2__
#define __VIRTUAL_DISK_QCOW2__

#include <QSharedPointer>
#include <QProcess>
#include <boost/serialization/strong_typedef.hpp>

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

///////////////////////////////////////////////////////////////////////////////
// struct Process

struct Process: QProcess
{
	void addChannel(int channel_);

protected:
	void setupChildProcess();

private:
	QList<int> m_channels;
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
	                    const QStringList &args = QStringList());

	PRL_RESULT disconnect();

	const QString& getDevice() const
	{
		return m_device;
	}

	void addFd(quint32 fd_)
	{
		m_process.addChannel(fd_);
	}

private:
	PRL_RESULT waitDevice();

	QString m_device;
	Process m_process;
};

} // namespace Nbd

namespace Policy
{
namespace Qcow2
{

typedef QString base_type;
typedef PRL_UINT64 size_type;
BOOST_STRONG_TYPEDEF(QString, unix_type)
BOOST_STRONG_TYPEDEF(PRL_UINT16, port_type)
BOOST_STRONG_TYPEDEF(PRL_INT32, fd_type)
BOOST_STRONG_TYPEDEF(bool, autoDevice_type)
BOOST_STRONG_TYPEDEF(bool, compressed_type)
BOOST_STRONG_TYPEDEF(bool, cached_type)

} // namespace Qcow2
} // namespace Policy

typedef boost::variant<
	Policy::Qcow2::base_type,
	Policy::Qcow2::size_type,
	Policy::Qcow2::unix_type,
	Policy::Qcow2::port_type,
	Policy::Qcow2::fd_type,
	Policy::Qcow2::autoDevice_type,
	Policy::Qcow2::compressed_type,
	Policy::Qcow2::cached_type
> qcow2Policy_type;
typedef std::vector<qcow2Policy_type> qcow2PolicyList_type;

///////////////////////////////////////////////////////////////////////////////
// Qcow2

struct Qcow2: Format
{
	virtual ~Qcow2()
	{
		close();
	}

	static PRL_RESULT create(const QString &fileName,
	                         const qcow2PolicyList_type &policies);

	virtual PRL_RESULT open(const QString &fileName,
	                        const PRL_DISK_OPEN_FLAGS flags,
	                        const policyList_type &policies = policyList_type());
	PRL_RESULT open(const QString &filename,
					const PRL_DISK_OPEN_FLAGS flags,
					const qcow2PolicyList_type &qcow2,
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

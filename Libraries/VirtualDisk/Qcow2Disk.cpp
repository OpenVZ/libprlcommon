///////////////////////////////////////////////////////////////////////////////
///
/// @file Qcow2Disk.cpp
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
#include "Qcow2Disk.h"
#include "Util.h"

#include <sys/ioctl.h>
#include <linux/fs.h>
#include <fcntl.h>
#include <cstdlib>

#include <QStringList>
#include <QDir>
#include <QElapsedTimer>
#include <QtGlobal>
#include <boost/property_tree/json_parser.hpp>

#include <prlsdk/PrlErrorsValues.h>
#include "../Logging/Logging.h"
#include "../HostUtils/HostUtils.h"
#include "Util.h"

namespace pt = boost::property_tree;

namespace VirtualDisk
{
namespace
{

enum {SECTOR_SIZE = 512};
enum {
	CMD_WORK_TIMEOUT = 60 * 60 * 1000,
	CMD_FAIL_TIMEOUT = 500,
	CMD_WAIT_TIMEOUT = 60000,
};

const char MODPROBE[] = "/usr/sbin/modprobe";
const char QEMU_NBD[] = "/usr/bin/qemu-nbd";
const char QEMU_IMG[] = "/usr/bin/qemu-img";

const char DEV[] = "/dev";
const char NBD_PATTERN[] = "nbd*";

QString enquote(const QString &s)
{
	return QString("\"%1\"").arg(s);
}

bool isAligned(PRL_UINT64 value)
{
	return !(value % SECTOR_SIZE);
}

bool isAligned(const void *value)
{
	return !((uintptr_t)value % SECTOR_SIZE);
}

} // namespace

namespace Nbd
{

///////////////////////////////////////////////////////////////////////////////
// Driver

PRL_RESULT Driver::insertModule()
{
	QStringList cmdLine = QStringList() << MODPROBE << "nbd" << "max_part=16";
	QString out;
	if (!HostUtils::RunCmdLineUtility(
			cmdLine.join(" "), out, CMD_WORK_TIMEOUT))
	{
		WRITE_TRACE(DBG_FATAL, "Cannot insert nbd kernel module");
		return PRL_ERR_DISK_GENERIC_ERROR;
	}
	return PRL_ERR_SUCCESS;
}

QStringList Driver::getDeviceList()
{
	if (PRL_FAILED(insertModule()))
		return QStringList();

	QDir dir(DEV);
	QStringList files = dir.entryList(QStringList() << NBD_PATTERN,
	                                  QDir::AllEntries | QDir::System);
	// e.g. nbd0, skip e.g. nbd0p1
	QRegExp deviceRe(QString("^%1\\d+$").arg(NBD_PATTERN));
	QStringList devices = files.filter(deviceRe);

	QStringList out;
	Q_FOREACH(const QString &device, devices)
		out << dir.filePath(device);

	return out;
}

///////////////////////////////////////////////////////////////////////////////
// struct Process

void Process::addChannel(int channel_)
{
	m_channels << channel_;
}

void Process::setupChildProcess()
{
	int x = 3;

	QProcess::setupChildProcess(); // requires vz-built qt version

	if (m_channels.empty())
		return;

	qSort(m_channels);
	foreach(int fd, m_channels) {
		if (x != fd)
		{
			if (dup2(fd, x) == -1)
			{
				WRITE_TRACE(DBG_FATAL, "dup2(%d,%d) failed: %m", fd, x);
				return;
			}
		}
		fcntl(x, F_SETFD, ~FD_CLOEXEC);
		x++;
	}

	qputenv("LISTEN_FDS", QString::number(m_channels.count()).toUtf8());
	qputenv("LISTEN_PID", QString::number(getpid()).toUtf8());
}

///////////////////////////////////////////////////////////////////////////////
// Qemu

PRL_RESULT Qemu::setDevice(const QString &device)
{
	IO::File file;
	PRL_RESULT res = file.open(device, O_RDONLY);
	if (PRL_FAILED(res))
		return res;

	size_t size;
	res = file.ioctl(BLKGETSIZE64, &size);
	if (PRL_FAILED(res))
		return res;

	if (size != 0)
		return PRL_ERR_INVALID_ARG;

	m_device = device;
	return PRL_ERR_SUCCESS;
}

qemu_type Qemu::create()
{
	QSharedPointer<Qemu> qemu(new Qemu());

	QStringList devices = Driver::getDeviceList();
	Q_FOREACH(const QString &device, devices)
	{
		if (PRL_SUCCEEDED(qemu->setDevice(device)))
			return qemu;
	}

	WRITE_TRACE(DBG_FATAL, "Cannot find free NBD device");
	return Error::Simple(PRL_ERR_DISK_GENERIC_ERROR);
}

PRL_RESULT Qemu::setImage(const QString &image, bool readOnly,
                          const QStringList &args)
{
	QStringList cmdLine = QStringList()
		<< QEMU_NBD << "-v"
		<< "-f" << "qcow2" << "--detect-zeroes=on"
		<< enquote(image);
	if (readOnly)
		cmdLine << "-r";
	if (!getDevice().isEmpty())
		cmdLine << "-c" << enquote(getDevice());
	else
	{
		// forbid server to stop after first connection
		cmdLine << "--persistent";
	}
	cmdLine << args;

	m_process.start(cmdLine.join(" "));
	if (!m_process.waitForStarted())
	{
		WRITE_TRACE(DBG_FATAL, "Cannot connect device using qemu-nbd:"
			"waitForStarted() failed");
		return PRL_ERR_DISK_FILE_OPEN_ERROR;
	}

	if (getDevice().isEmpty())
	{
		if (m_process.waitForFinished(CMD_FAIL_TIMEOUT))
		{
			WRITE_TRACE(DBG_FATAL, "Cannot connect device using qemu-nbd:"
				"waitForFinished() failed");
			return PRL_ERR_DISK_FILE_OPEN_ERROR;
		}
		return PRL_ERR_SUCCESS;
	}

	return waitDevice();
}

PRL_RESULT Qemu::disconnect()
{
	if (getDevice().isEmpty())
		m_process.terminate();
	else
	{
		QStringList cmdLine = QStringList() << QEMU_NBD << "-d" << enquote(getDevice());
		QString out;
		if (!HostUtils::RunCmdLineUtility(cmdLine.join(" "), out, CMD_WORK_TIMEOUT))
		{
			WRITE_TRACE(DBG_FATAL, "Cannot disconnect device using qemu-nbd");
			return PRL_ERR_DISK_GENERIC_ERROR;
		}
	}
	// Wait infinitely to catch disconnect errors.
	m_process.waitForFinished(-1);
	return PRL_ERR_SUCCESS;
}

PRL_RESULT Qemu::waitDevice()
{
	IO::File f;
	PRL_RESULT e = f.open(getDevice(), O_RDONLY);
	if (PRL_FAILED(e))
		return e;

	QElapsedTimer t;
	size_t s = 0;
	for (t.start(); t.elapsed() < CMD_WAIT_TIMEOUT;)
	{
		// check whether block device is ready
		e = f.ioctl(BLKGETSIZE64, &s);
		if (PRL_FAILED(e))
			return e;
		if (s)
			break;
		// maybe qemu-nbd already exited?
		if (m_process.waitForFinished(CMD_FAIL_TIMEOUT))
		{
			WRITE_TRACE(DBG_FATAL, "Cannot connect device using qemu-nbd");
			return PRL_ERR_DISK_FILE_OPEN_ERROR;
		}
	}

	if (s == 0)
	{
		WRITE_TRACE(DBG_FATAL, "Timeout elapsed while waiting for "
			"nbd device to become ready");
		return PRL_ERR_DISK_FILE_OPEN_ERROR;
	}

	return PRL_ERR_SUCCESS;
}

} // namespace Nbd

namespace Command
{

///////////////////////////////////////////////////////////////////////////////
// SetImage

struct SetImage
{
	SetImage():
		m_offset(0), m_compressed(false), m_cache("none"), m_aio("native"),
		m_device(QSharedPointer<Nbd::Qemu>(new Nbd::Qemu))
	{
	}

	PRL_RESULT operator() (const QString &image, bool readOnly)
	{
		if (m_device.isFailed())
			return m_device.error().code();
		return m_device.value()->setImage(image, readOnly, buildArgs());
	}

	const QSharedPointer<Nbd::Qemu>& getDevice() const
	{
		return m_device.value();
	}

	void setOffset(PRL_UINT64 offset)
	{
		m_offset = offset;
	}

	void setUnix(const QString &u)
	{
		if (!u.isEmpty())
			m_args = QStringList() << "-k" << enquote(u);
	}

	void setPort(PRL_UINT16 port)
	{
		if (port)
			m_args = QStringList() << "-p" << QString::number(port);
	}

	void setFd(PRL_INT32 fd)
	{
		if (fd >= 0) {
			m_device.value()->addFd(fd);
		}
	}

	void setAutoDevice(bool autoDevice)
	{
		m_device = autoDevice ? Nbd::Qemu::create() : QSharedPointer<Nbd::Qemu>(new Nbd::Qemu);
	}

	void setCompressed(bool compressed)
	{
		m_compressed = compressed;
	}

	void setCache(const QString& cache)
	{
		m_cache = cache;
	}

	void setAio(const QString& aio)
	{
		m_aio = aio;
	}

	void setExportName(const QString& value_)
	{
		m_exportName = value_;
	}

private:
	QStringList buildArgs() const
	{
		QStringList a(m_args);
		if (m_offset)
			a << "-o" << QString::number(m_offset);
		if (m_compressed)
			a << "-C";
		a << QString("--cache=%1").arg(m_cache);
		a << QString("--aio=%1").arg(m_aio);
		if (!m_exportName.isEmpty())
			a << "-x" << m_exportName;

		return a;
	}

	PRL_UINT64 m_offset;
	bool m_compressed;
	QString m_cache;
	QString m_aio;
	QString m_exportName;

	QStringList m_args;
	Nbd::qemu_type m_device;
};

///////////////////////////////////////////////////////////////////////////////
// Open

struct Open: boost::static_visitor<>
{
	template <typename T>
		void operator() (const T& value)
	{
		Q_UNUSED(value);
	}

	SetImage& getSetImage()
	{
		return m_setImage;
	}

private:
	SetImage m_setImage;
};

template<> void Open::operator() (const Policy::Offset &offset)
{
	m_setImage.setOffset(offset.getData());
}

template<> void Open::operator() (const Policy::Qcow2::unix_type &u)
{
	m_setImage.setUnix(u);
}

template<> void Open::operator() (const Policy::Qcow2::port_type &port)
{
	m_setImage.setPort(port);
}

template<> void Open::operator() (const Policy::Qcow2::fd_type &fd)
{
	m_setImage.setFd(fd);
}

template<> void Open::operator() (const Policy::Qcow2::autoDevice_type &dev)
{
	m_setImage.setAutoDevice(dev);
}

template<> void Open::operator() (const Policy::Qcow2::compressed_type &c)
{
	m_setImage.setCompressed(c);
}

template<> void Open::operator() (const Policy::Qcow2::cached_type &c)
{
	m_setImage.setCache(c ? "writeback" : "none");
	m_setImage.setAio(c ? "threads" : "native");
}

template<>
void Open::operator() (const Policy::Qcow2::exportName_type& value_)
{
	m_setImage.setExportName(value_);
}

///////////////////////////////////////////////////////////////////////////////
// Create

struct Create: boost::static_visitor<>
{
	explicit Create(const QString &fileName);

	template <typename T>
		void operator() (const T &value)
	{
		Q_UNUSED(value);
	}

	QStringList getCommandLine() const
	{
		return QStringList() << QEMU_IMG << "create"
			<< "-f" << "qcow2"
			<< "-o" << QString("cluster_size=%1,lazy_refcounts=on")
				.arg(m_clusterSize)
			<< m_cmdLine;
	}

private:
	QStringList m_cmdLine;
	PRL_UINT32 m_clusterSize;
};

Create::Create(const QString &fileName) :
		m_clusterSize(1024 * 1024)
{
	m_cmdLine << enquote(fileName);
}

template<> void Create::operator() (const Policy::Qcow2::size_type &value)
{
	m_cmdLine << "-o" << QString("size=%1").arg(value);
}

template<> void Create::operator() (const Policy::Qcow2::base_type &value)
{
	m_cmdLine << "-o" << QString("backing_fmt=%2,backing_file=%1")
		.arg(enquote(value.getUrl())).arg(enquote(value.getFormat()));
}

template<> void Create::operator() (const Policy::Qcow2::clusterSize_type &value)
{
	m_clusterSize = value;
}

} // namespace Command

///////////////////////////////////////////////////////////////////////////////
// Qcow2

PRL_RESULT Qcow2::create(const QString &fileName, const qcow2PolicyList_type &policies)
{
	QFileInfo info(fileName);
	if (info.exists() || !info.dir().mkpath("."))
	{
		WRITE_TRACE(DBG_FATAL, "File exists or cannot be created");
		return PRL_ERR_DISK_FILE_EXISTS;
	}

	if (policies.empty())
		return PRL_ERR_INVALID_ARG;

	Command::Create v(fileName);
	std::for_each(policies.begin(), policies.end(), boost::apply_visitor(v));
	QStringList cmdLine = v.getCommandLine();

	QString out;
	if (!HostUtils::RunCmdLineUtility(cmdLine.join(" "), out, CMD_WORK_TIMEOUT))
	{

		WRITE_TRACE(DBG_FATAL, "Cannot create image");
		return PRL_ERR_DISK_CREATE_IMAGE_ERROR;
	}

	return PRL_ERR_SUCCESS;
}

PRL_RESULT Qcow2::open(const QString &fileName, PRL_DISK_OPEN_FLAGS flags,
                       const policyList_type &policies)
{
	return open(fileName, flags, qcow2PolicyList_type(
				1, Policy::Qcow2::autoDevice_type(true)), policies);
}

PRL_RESULT Qcow2::open(const QString &fileName, PRL_DISK_OPEN_FLAGS flags,
                       const qcow2PolicyList_type &qcow2,
                       const policyList_type &policies)
{
	if (m_device)
	{
		WRITE_TRACE(DBG_FATAL, "Disk is already opened");
		return PRL_ERR_DISK_FILE_OPEN_ERROR;
	}

	if (flags & PRL_DISK_FAKE_OPEN)
		return isValid(fileName) ? PRL_ERR_SUCCESS : PRL_ERR_DISK_FILE_OPEN_ERROR;

	flags_type openFlags = convertFlags(flags);
	if (openFlags.isFailed())
		return openFlags.error().code();

	// TODO: Lock disk.

	Command::Open v;
	std::for_each(policies.begin(), policies.end(), boost::apply_visitor(v));
	std::for_each(qcow2.begin(), qcow2.end(), boost::apply_visitor(v));
	PRL_RESULT res = v.getSetImage()(fileName, !(flags & PRL_DISK_WRITE));
	if (PRL_FAILED(res))
		return res;

	m_device = v.getSetImage().getDevice();

	if (!m_device->getDevice().isEmpty() &&
		PRL_FAILED(res = m_file.open(m_device->getDevice(), openFlags.value())))
	{
		closeForce();
		return res;
	}

	m_readOnly = !(bool)(flags & PRL_DISK_WRITE);
	m_fileName = fileName;
	return PRL_ERR_SUCCESS;
}

PRL_RESULT Qcow2::close()
{
	if (!m_device)
		return PRL_ERR_DISK_DISK_NOT_OPENED;

	closeForce();
	return PRL_ERR_SUCCESS;
}

void Qcow2::closeForce()
{
	m_file.close();
	m_device.clear();
	m_fileName.clear();
}

PRL_RESULT Qcow2::read(void *data, PRL_UINT32 sizeBytes, PRL_UINT64 offSec)
{
	if (!isAligned(data) || !isAligned(sizeBytes))
		return PRL_ERR_INVALID_ARG;

	if (sizeBytes == 0)
		return PRL_ERR_SUCCESS;

	return m_file.pread(data, sizeBytes, offSec * SECTOR_SIZE);
}

PRL_RESULT Qcow2::write(const void *data, PRL_UINT32 sizeBytes, PRL_UINT64 offSec)
{
	if (m_readOnly)
	{
		WRITE_TRACE(DBG_FATAL, "Image is opened read-only");
		return PRL_ERR_DISK_OPERATION_NOT_ALLOWED;
	}

	if (!isAligned(data) || !isAligned(sizeBytes))
		return PRL_ERR_INVALID_ARG;

	if (sizeBytes == 0)
		return PRL_ERR_SUCCESS;

	return m_file.pwrite(data, sizeBytes, offSec * SECTOR_SIZE);
}

Parameters::disk_type Qcow2::getInfo()
{
	Parameters::Disk disk;

	size_t size, blockSize;
	PRL_RESULT res = m_file.ioctl(BLKGETSIZE64, &size);
	if (PRL_FAILED(res))
		return Error::Simple(res);

	res = m_file.ioctl(BLKBSZGET, &blockSize);
	if (PRL_FAILED(res))
		return Error::Simple(res);

	disk.setSizeInSectors(size / SECTOR_SIZE);
	disk.setBlockSize(blockSize / SECTOR_SIZE);

	Parameters::Image image;
	image.setType(PRL_IMAGE_TRY_GUESS);
	image.setStart(0);
	image.setSize(size);
	image.setFilename(m_fileName);

	disk.addStorage(image);

	return disk;
}

bool Qcow2::isValid(const QString &fileName)
{
	QStringList cmdLine = QStringList()
		<< QEMU_IMG << "info" << "--force-share" << "--output=json"
		<< fileName;
	QString out;
	if (!HostUtils::RunCmdLineUtility(
			cmdLine.join(" "), out, CMD_WORK_TIMEOUT))
	{
		WRITE_TRACE(DBG_FATAL, "Cannot get image info");
		return false;
	}

	pt::ptree pt;
	QByteArray bytes(out.toUtf8());
	std::istringstream stream(bytes.constData());
	try
	{
		read_json(stream, pt);
	}
	catch(const pt::json_parser_error &e)
	{
		WRITE_TRACE(DBG_FATAL, "Cannot parse qemu-img info output");
		return false;
	}

	boost::optional<std::string> format = pt.get_optional<std::string>("format");
	if (!format)
	{
		WRITE_TRACE(DBG_FATAL, "Cannot get image format");
		return false;
	}

	return *format == "qcow2";
}

CSparseBitmap *Qcow2::getUsedBlocksBitmap(UINT32 granularity,
                PRL_RESULT &err)
{
	Q_UNUSED(granularity)
	Q_UNUSED(err)

	return NULL;
}

CSparseBitmap *Qcow2::getTrackingBitmap(const QString &uuid)
{
	Q_UNUSED(uuid);
	return NULL;
}

PRL_RESULT Qcow2::cloneState(const QString &uuid, const QString &target)
{
	Q_UNUSED(uuid)
	Q_UNUSED(target)
	return PRL_ERR_UNIMPLEMENTED;
}

} // namespace VirtualDisk

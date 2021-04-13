///////////////////////////////////////////////////////////////////////////////
///
/// @file Qcow2Disk.cpp
///
/// VirtualDisk implementation for qcow2.
///
/// @author mperevedentsev
///
/// Copyright (c) 2016-2017, Parallels International GmbH
/// Copyright (c) 2017-2020 Virtuozzo International GmbH, All rights reserved.
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
/// Our contact details: Virtuozzo International GmbH, Vordergasse 59, 8200
/// Schaffhausen, Switzerland.
///
///////////////////////////////////////////////////////////////////////////////
#include "Qcow2Disk.h"
#include "Qcow2Disk_p.h"
#include "Util.h"

#include <sys/ioctl.h>
#include <linux/nbd.h>
#include <linux/fs.h>
#include <fcntl.h>
#include <cstdlib>
#include <json.h>
#include <QStringList>
#include <QDir>
#include <errno.h>
#include <QtGlobal>
#include <sys/file.h>
#include <boost/bind.hpp>
#include <boost/atomic.hpp>
#include "../Logging/Logging.h"
#include "../HostUtils/HostUtils.h"
#include "Util.h"
#include <boost/algorithm/string.hpp>

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
namespace
{
///////////////////////////////////////////////////////////////////////////////
// struct Crutch

struct Crutch: QThread
{
	void run()
	{
		if (0 == startTimer(150000))
			abort();
		WRITE_TRACE(DBG_FATAL, "running the crutch");
		if (0 != exec())
			abort();
	}

protected:
	void timerEvent(QTimerEvent* event_)
	{
		Q_UNUSED(event_);
		exit(1);
	}
};

} // namespace

///////////////////////////////////////////////////////////////////////////////
// Driver

PRL_RESULT Driver::insertModule()
{
	if (access("/sys/block/nbd0", F_OK) == 0)
		return PRL_ERR_SUCCESS;

	Crutch q;
	QStringList cmdLine = QStringList() << MODPROBE << "nbd" << "max_part=16";
	QString out;
	q.moveToThread(&q);
	q.start();
	PRL_RESULT output = PRL_ERR_SUCCESS;
	if (!HostUtils::RunCmdLineUtility(
			cmdLine.join(" "), out, CMD_WORK_TIMEOUT))
		output = PRL_ERR_DISK_GENERIC_ERROR;

	q.quit();
	q.wait();
	if (PRL_FAILED(output))
		WRITE_TRACE(DBG_FATAL, "Cannot insert nbd kernel module");

	return output;
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

	HostUtils::sanitizeEnv(*this, true);

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

namespace State
{
///////////////////////////////////////////////////////////////////////////////
// struct Stopping

Retired Stopping::retire()
{
	token_type t = getToken();
	if (!t.isNull())
	{
		t->reportResult(PRL_ERR_SUCCESS);
		t->reportFinished();
	}
	m_machine.clear();

	return Retirable::retire();
}

///////////////////////////////////////////////////////////////////////////////
// struct Wakeup

void Wakeup::bind(QObject& target_)
{
	QTimer* t = new QTimer();
	t->setSingleShot(true);
	target_.connect(t, SIGNAL(timeout()), SLOT(reactTimeout()));
	t->connect(t, SIGNAL(timeout()), SLOT(deleteLater()));
	t->start(CMD_FAIL_TIMEOUT);

	m_timer = t;
}

void Wakeup::cancel()
{
	if (NULL != m_timer)
	{
		m_timer->stop();
		m_timer->deleteLater();
		m_timer = NULL;
	}
}

} // namespace State

namespace Machine
{
///////////////////////////////////////////////////////////////////////////////
// struct Carcass

Carcass::~Carcass()
{
}

///////////////////////////////////////////////////////////////////////////////
// struct Generic<T>

template<class T>
template<class U>
Generic<T>::Generic(const Script& script_, T& state_, U flavor_):
	m_state(state_), m_flavor(flavor_)
{
	QTimer* t = new QTimer();
	Process* p = new Process();
	m_engine.reset(p);
	t->setSingleShot(true);
	connect(t, SIGNAL(timeout()), SLOT(reactTimeout()));
	t->connect(t, SIGNAL(timeout()), SLOT(deleteLater()));
	t->connect(p, SIGNAL(started()), SLOT(stop()));
	t->connect(p, SIGNAL(started()), SLOT(deleteLater()));

	connect(p, SIGNAL(started()), SLOT(reactStarted()));
	connect(p, SIGNAL(finished(int , QProcess::ExitStatus )),
			SLOT(reactFinish(int , QProcess::ExitStatus )));

	script_(*p);
	t->start(30000);
}

///////////////////////////////////////////////////////////////////////////////
// struct Flavor<Persistent::state_type>

void Flavor<Persistent::state_type>::terminate(QScopedPointer<QProcess>& nbd_)
{
	if (NULL == nbd_.data())
		return;

	switch (nbd_->state())
	{
	case QProcess::NotRunning:
		nbd_.reset();
		break;
	default:
		nbd_->terminate();
	}
}

///////////////////////////////////////////////////////////////////////////////
// struct Flavor<Export::state_type>

void Flavor<Export::state_type>::terminate(QScopedPointer<QProcess>& nbd_)
{
	Flavor<Persistent::state_type>().terminate(nbd_);
	if (NULL == nbd_.data())
		m_guard.clear();
}

} // namespace Machine

namespace Export
{
namespace State
{
///////////////////////////////////////////////////////////////////////////////
// struct Disconnecting

Nbd::State::Stopping Disconnecting::stop() const
{
	Nbd::State::Stopping output(m_machine);
	output.adopt(getToken());

	return output;
}

Nbd::State::Terminating Disconnecting::terminate(PRL_RESULT reason_) const
{
	token_type t = getToken();
	if (!t.isNull())
		t->reportFinished(&reason_);

	return Nbd::State::Terminating(m_machine);
}

///////////////////////////////////////////////////////////////////////////////
// struct Running

Disconnecting Running::disconnect() const
{
	QFutureWatcher<PRL_RESULT>* w = new QFutureWatcher<PRL_RESULT>();
	m_machine->connect(w, SIGNAL(finished()), SLOT(reactCompleted()));
	w->connect(w, SIGNAL(finished()), SLOT(deleteLater()));
	PRL_RESULT (* f)(const QString&) = &Running::disconnect;
	w->setFuture(QtConcurrent::run(boost::bind(f, m_machine->getDevice())));

	return Disconnecting(m_machine);
}

PRL_RESULT Running::disconnect(const QString& device_)
{
	WRITE_TRACE(DBG_FATAL, "DISCONNECT %s", qPrintable(device_));
	int fd;

	fd = open(qPrintable(device_), O_RDWR);
	if (fd == -1)
	{
		WRITE_TRACE(DBG_FATAL, "Cannot open %s: %m", qPrintable(device_));
		return PRL_ERR_DISK_GENERIC_ERROR;
	}

	ioctl(fd, NBD_CLEAR_QUE);
	ioctl(fd, NBD_DISCONNECT);
	ioctl(fd, NBD_CLEAR_SOCK);
	close(fd);

	return PRL_ERR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// struct Steady

Steady::Steady(const machineCarrier_type& machine_, const token_type& token_):
	m_token(token_), m_machine(machine_)
{
	m_timer.start();
}

Running Steady::run() const
{
	m_token->reportResult(PRL_ERR_SUCCESS);
	m_token->reportFinished();
	return Running(m_machine);
}

Nbd::State::Retired Steady::retire()
{
	m_retry.cancel();
	m_token->reportResult(PRL_ERR_FAILURE);
	m_token->reportFinished();
	return Nbd::State::Retired();
}

Nbd::State::Terminating Steady::terminate() const
{
	return Nbd::State::Terminating(m_machine);
}

Prl::Expected<size_t, PRL_RESULT> Steady::probe()
{
	m_retry = Nbd::State::Wakeup();
	Prl::Expected<size_t, PRL_RESULT> output;
	if (CMD_WAIT_TIMEOUT > m_timer.elapsed())
		output = m_machine->probe();
	else
	{
		WRITE_TRACE(DBG_FATAL, "Timeout elapsed while waiting for "
			"nbd device to become ready");
		output = PRL_ERR_DISK_FILE_OPEN_ERROR;
	}
	if (output.isFailed())
		m_token->reportFinished(&output.error());

	return output;
}

Steady Steady::retry() const
{
	Steady output = *this;
	output.m_retry.bind(*m_machine);

	return output;
}

} // namespace State

namespace Visitor
{
///////////////////////////////////////////////////////////////////////////////
// struct Started

Started::result_type Started::operator()(const State::Starting& value_) const
{
	State::Steady x = value_.steady();
	return visit(x);
}

Started::result_type Started::visit(State::Steady& value_) const
{
	Prl::Expected<size_t, PRL_RESULT> p(value_.probe());
	if (p.isFailed())
		return result_type(value_.terminate());
	else if (0 < p.value())
		return result_type(value_.run());

	return result_type(value_.retry());
}

///////////////////////////////////////////////////////////////////////////////
// struct Completed

Completed::result_type Completed::operator()(State::Disconnecting& value_) const
{
	if (PRL_FAILED(m_status))
		return result_type(value_.terminate(m_status));

	return result_type(value_.stop());
}

} // namespace Visitor

///////////////////////////////////////////////////////////////////////////////
// struct Machine

Machine::Machine(Script script_, state_type& state_, QSharedPointer<QFile> guard_):
	Nbd::Machine::Generic<state_type>
		(script_.addArgument("-c").addArgument(enquote(guard_->fileName())), state_, guard_),
	m_guard(guard_.toWeakRef())
{
}

QString Machine::getDevice() const
{
	return m_guard.isNull() ? QString() : m_guard.data()->fileName();
}

Prl::Expected<size_t, PRL_RESULT> Machine::probe() const
{
	QFile* f = m_guard.data();
	if (NULL == f)
		return PRL_ERR_UNINITIALIZED;

	size_t output = 0;
	if (-1 == ::ioctl(f->handle(), BLKGETSIZE64, &output))
	{
		WRITE_TRACE(DBG_FATAL, "ioctl() failed: %m");
		return PRL_ERR_DISK_GENERIC_ERROR;
	}

	return output;
}

void Machine::reactCompleted()
{
	PRL_RESULT s = static_cast<QFutureWatcher<PRL_RESULT>* >(sender())->result();
	m_state = boost::apply_visitor(Visitor::Completed(s), m_state);
}

} // namespace Export

namespace Persistent
{
namespace State
{
///////////////////////////////////////////////////////////////////////////////
// struct Linger

Linger::Linger(const machineCarrier_type& machine_, const token_type& token_):
	m_token(token_), m_machine(machine_)
{
	m_delay.bind(*machine_);
}

Running Linger::expire()
{
	m_delay = Nbd::State::Wakeup();
	m_token->reportResult(PRL_ERR_SUCCESS);
	m_token->reportFinished();

	return Running(m_machine);
}

Nbd::State::Retired Linger::retire()
{
	m_delay.cancel();
	m_token->reportResult(PRL_ERR_FAILURE);
	m_token->reportFinished();
	WRITE_TRACE(DBG_FATAL, "Cannot connect device using qemu-nbd:"
		"waitForFinished() failed");

	return Nbd::State::Retired();
}

///////////////////////////////////////////////////////////////////////////////
// struct Starting

Nbd::State::Retired Starting::retire() const
{
	Nbd::State::Terminating x(m_machine);
	m_token->reportResult(PRL_ERR_FAILURE);
	m_token->reportFinished();

	return x.retire();
}

} // namespace State
} // namespace Persistent

namespace Backend
{
///////////////////////////////////////////////////////////////////////////////
// struct Flavor<Persistent::state_type>

Flavor<Persistent::state_type>::startVisitor_type
Flavor<Persistent::state_type>::start(Script script_, const token_type& token_, state_type& state_)
{
	return startVisitor_type(
		Persistent::State::Starting(
			Persistent::machineCarrier_type(
				new Nbd::Machine::Generic<state_type>(
					script_.addArgument("--persistent"), state_, Nbd::Machine::Flavor<state_type>())),
			token_));
}

///////////////////////////////////////////////////////////////////////////////
// struct Flavor<Export::state_type>

Flavor<Export::state_type>::startVisitor_type
Flavor<Export::state_type>::start(const Script& script_, const token_type& token_, Export::state_type& state_)
{
	return startVisitor_type(
		Export::State::Starting(
			Export::machineCarrier_type(
				new Export::Machine(script_, state_, QSharedPointer<QFile>(m_guard.take()))),
			token_));
}

} // namespace Backend

///////////////////////////////////////////////////////////////////////////////
// struct Script

Script::Script(const QString& image_, const portList_type& portList_):
	m_image(image_), m_logUuid(Uuid::createUuid().toStringWithoutBrackets()),
	m_portList(portList_)
{
}

Script& Script::addArgument(const QString& one_)
{
	m_commandLine << one_;
	return *this;
}

Script& Script::addArguments(const QStringList& bunch_)
{
	m_commandLine << bunch_;
	return *this;
}

class Sleeper : public QThread
{
public:
	static void sleep(unsigned long secs){QThread::sleep(secs);}
};

void Script::operator()(Process& target_) const
{
	QString dev = findConnectedDevice(m_image);
	if (!dev.isEmpty())
	{
		Export::State::Running::disconnect(dev);
		Sleeper::sleep(5);
	}

	foreach(quint32 p, m_portList)
	{
		target_.addChannel(p);
	}
	QStringList a;
	a << QEMU_NBD << "-v"
//		<< "-T" << QString("enable=*,file=/vz/tmp/rempy/%1").arg(m_logUuid)
		<< "-f" << "qcow2" << "--detect-zeroes=on"
		<< m_commandLine << m_image;
	target_.start(a.join(" "));
}


/* Search running qemu-nbd with assigned image and return NBD device */
QString Script::findConnectedDevice(const QString& image_) const
{
	foreach(const QString &dev, Nbd::Driver::getDeviceList())
	{
		QString pid;
		QFile f(QString("/sys/block/%1/pid").arg(QFileInfo(dev).fileName()));
		if (f.open(QIODevice::ReadOnly | QIODevice::Text))
		{
			QTextStream s(&f);
			pid = s.readLine();
		}
		if (pid.isEmpty())
			continue;

		QFile c(QString("/proc/%1/cmdline").arg(pid));
		if (c.open(QIODevice::ReadOnly))
		{
			QByteArray cmd = c.readAll();
			if (cmd.startsWith(QEMU_NBD) && cmd.contains(qPrintable(image_)))
				return dev;
		}
	}

	return QString();;
}

///////////////////////////////////////////////////////////////////////////////
// struct Frontend

Frontend::Frontend(): m_nbd()
{
	qRegisterMetaType<Script>("Script");
	qRegisterMetaType<QFuture<PRL_RESULT> >("future_type");
}

PRL_RESULT Frontend::stop()
{
    if (nullptr == m_nbd)
        return PRL_ERR_UNINITIALIZED;

    future_type f;
    bool x = QMetaObject::invokeMethod(m_nbd, "stop",
                Qt::BlockingQueuedConnection,
                Q_RETURN_ARG(future_type, f));
    if (!x)
        return PRL_ERR_FAILURE;

    PRL_RESULT output = f.result();

    m_device.clear();
    m_nbd->deleteLater();
    m_nbd = nullptr;

    return output;
}

PRL_RESULT Frontend::bind(const QString& device_)
{
	if (NULL != m_nbd)
		return PRL_ERR_OPERATION_PENDING;

	QScopedPointer<QFile> f(new QFile(device_));
	if (!f->open(QIODevice::ReadOnly))
	{
		WRITE_TRACE(DBG_FATAL, "Cannot open file '%s': %m",
			qPrintable(device_));
		return PRL_ERR_DISK_FILE_OPEN_ERROR;
	}
	if (-1 == TEMP_FAILURE_RETRY(::flock(f->handle(), LOCK_EX | LOCK_NB)))
	{
		WRITE_TRACE(DBG_FATAL, "Device '%s' has already been locked",
			qPrintable(device_));
		return PRL_ERR_INVALID_ARG;
	}
	if (QFileInfo(QString("/sys/block/%1/pid").arg(QFileInfo(device_).fileName())).exists())
	{
		WRITE_TRACE(DBG_FATAL, "Device '%s' is being used by someone else",
			qPrintable(device_));
		return PRL_ERR_INVALID_ARG;
	}

	m_guard.reset(f.take());
	return PRL_ERR_SUCCESS;
}

PRL_RESULT Frontend::start(const Script& script_)
{
	if (NULL != m_nbd)
		return PRL_ERR_OPERATION_PENDING;

	if (NULL == QCoreApplication::instance())
	{
		WRITE_TRACE(DBG_FATAL, "Qt application is not running");
		return PRL_ERR_UNINITIALIZED;
	}
	QString d;
	Backend::Base* b = NULL;
	if (NULL == m_guard.data())
	{
		b = new Backend::Generic<Persistent::state_type>
			(Backend::Flavor<Persistent::state_type>());
	}
	else
	{
		d = m_guard->fileName();
		b = new Backend::Generic<Export::state_type>(m_guard.take());
	}

	b->moveToThread(QCoreApplication::instance()->thread());
	b->connect(this, SIGNAL(destroyed()), SLOT(deleteLater()));

	future_type f;
	bool x = QMetaObject::invokeMethod(b, "start",
		Qt::BlockingQueuedConnection, Q_RETURN_ARG(future_type, f),
		Q_ARG(Script, script_));
	if (!x)
	{
		b->deleteLater();
		return PRL_ERR_FAILURE;
	}
	if (PRL_FAILED(f.result()))
	{
		b->deleteLater();
		return f.result();
	}
	m_nbd = b;
	m_device = d;
	return PRL_ERR_SUCCESS;
}

} // namespace Nbd

namespace Command
{

///////////////////////////////////////////////////////////////////////////////
// SetImage

struct SetImage
{
	typedef QSharedPointer<Nbd::Frontend> qemuCarrier_type;

	SetImage():
		m_offset(0), m_compressed(false), m_cache("none"), m_aio("native"),
		m_device(qemuCarrier_type(new Nbd::Frontend()))
	{
	}

	PRL_RESULT operator() (const QString &image, bool readOnly)
	{
		if (m_device.isFailed())
			return m_device.error();

		Nbd::Script x(image, m_portList);
		if (readOnly)
			x.addArgument("-r");

		return m_device.value()->start(x.addArguments(buildArgs()));
	}

	const qemuCarrier_type& getDevice() const
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
			m_portList << fd;
		}
	}

	void setAutoDevice(bool autoDevice)
	{
		qemuCarrier_type qemu(new Nbd::Frontend());
		if (!autoDevice)
			return (void)(m_device = qemu);

		foreach(const QString &device, Nbd::Driver::getDeviceList())
		{
			if (PRL_SUCCEEDED(qemu->bind(device)))
				return (void)(m_device = qemu);
		}

		WRITE_TRACE(DBG_FATAL, "Cannot find free NBD device");
		m_device = PRL_ERR_DISK_GENERIC_ERROR;
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
	Nbd::Script::portList_type m_portList;

	QStringList m_args;
	Prl::Expected<qemuCarrier_type, PRL_RESULT> m_device;
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
	m_cmdLine << "-o" << QString("size=%1").arg(value) << "-u";
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
	m_device->stop();
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
	QByteArray bytes(out.toUtf8());
	struct json_object* j = json_tokener_parse(bytes.constData());
	if (NULL == j)
	{
		WRITE_TRACE(DBG_FATAL, "Cannot parse qemu-img info output");
		return false;
	}
	json_object_object_foreach(j, k, v)
	{
		if (boost::equals(k, "format"))
		{
			const char* V = json_object_get_string(v);
			if (NULL == V)
				break;

			bool output = boost::equals(V, "qcow2");
			json_object_put(j);
			return output;
		}
	}
	json_object_put(j);

	WRITE_TRACE(DBG_FATAL, "Cannot get image format");
	return false;
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

/*
 * HostUtils.h: Collection of some useful functions
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


//////////////////////////////////////////////////////////////////////////
///
///    @class HostUtils HostUtils.h "HostUtils.h"
///    @version 0.1
///
///    @brief Collection of some useful functions. To call them use
///    HostUtils::Fucntion(..) format
///
///    @author Some beautiful people
///
//////////////////////////////////////////////////////////////////////////
#ifndef _HOST_UTILS_H_
#define _HOST_UTILS_H_

#ifdef _WIN_
#include <windows.h>
// these two def/undef below are a kludge for MS incompatibilities
// in various intrin.h (VC, DDK). They are required to avoid build
// failures when these includes are mixed (environment with VC, PSDK and DDK)
#define _interlockedbittestandset _kludge1_
#define _interlockedbittestandreset _kludge2_
#include <intrin.h>
#undef _interlockedbittestandset
#undef _interlockedbittestandreset
#pragma intrinsic(__rdtsc)
#else
#include <pthread.h>
#include <sys/time.h>
#endif

#include "../Interfaces/VirtuozzoTypes.h"
#include <prlsdk/PrlEnums.h>
#include <prlsdk/PrlTypes.h>
#include <prlsdk/PrlErrors.h>
#include "../Logging/Logging.h"
#include "../Interfaces/VirtuozzoQt.h"

#include <QFileInfo>
#include <QFile>
#include <QString>
#include <QStringList>
#include <QMap>
#include <QProcess>

#include <boost/optional/optional.hpp>

// Fat file size limit in GB
#define FS_FAT_FILE_SIZE_LIMIT_GB 	(2)
// Fat file size limit in bytes
#define FS_FAT_FILE_SIZE_LIMIT 		((PRL_UINT64)FS_FAT_FILE_SIZE_LIMIT_GB << 30)
// Maximum filesystem name size
#define FS_NAME_SIZE 				(256)

// Size of default Floppy Image
#define CFG_144_IMAGE_SIZE 	(1474560)

#define MBYTE (1024 * 1024LL)
#define GBYTE (1024 * MBYTE)

struct HostUser
{
	QString m_qsSysName;
	QString m_qsFriendlyName;
	QByteArray m_baIcon;
};

struct HostMemUsage
{
	/* Memory size in bytes */
	UINT64 total;
	UINT64 free;
	UINT64 inactive;
	UINT64 active;
	UINT64 wired;
	UINT64 swap_used;
	UINT64 cached;

	UINT32 pagesize;
};

struct RunCmdResult
{
	explicit RunCmdResult(enum QProcess::ProcessError error) :
		m_executed(false), m_error(error)
	{
	}

	explicit RunCmdResult(int retcode) :
		m_executed(true), m_retcode(retcode)
	{
	}

	template <typename Handler>
	Handler &operator()(Handler &handler)
	{
		if (m_executed) {
			handler.exitCode(m_retcode);
			return handler;
		}
		switch (m_error)
		{
		case QProcess::FailedToStart:
			handler.startFailed();
			break;
		case QProcess::Timedout:
			handler.waitFailed();
			break;
		case QProcess::Crashed:
			handler.crashed();
			break;
		default:
			break;
		}
		return handler;
	}

private:
	bool m_executed;
	union {
		enum QProcess::ProcessError m_error;
		int m_retcode;
	};
};

struct ExecHandlerBase
{
	typedef void (*AfterStartCallback)(QProcess*);

	ExecHandlerBase(QProcess &process, const QString &cmd, int loglevel = DBG_FATAL) :
		m_process(&process), m_cmd(cmd), m_loglevel(loglevel)
	{
	}

	void startFailed() const
	{
		WRITE_TRACE(m_loglevel, "Program '%s' start error !", QSTR2UTF8(m_cmd));
	}

	void waitFailed() const
	{
		WRITE_TRACE(m_loglevel, "Program '%s' wait error !", QSTR2UTF8(m_cmd));
	}

	void crashed() const
	{
		WRITE_TRACE(m_loglevel, "Program '%s' was crashed !", QSTR2UTF8(m_cmd));
	}

	void exitCode(int code) const
	{
		if (code != 0)
			logExitCode();
	}

	QProcess &getProcess()
	{
		return *m_process;
	}

	const QByteArray &getStderr() const
	{
		if (!m_stderr)
			m_stderr = m_process->readAllStandardError();
		return m_stderr.get();
	}

protected:

	void logExitCode() const
	{
		WRITE_TRACE(m_loglevel, "Program '%s' returned exit code: '%d' !",
						QSTR2UTF8(m_cmd), m_process->exitCode());

		const QByteArray a = getStderr();
		if (!a.isEmpty())
			WRITE_TRACE(m_loglevel, "Program '%s' returned with error: '%s' !",
							QSTR2UTF8(m_cmd), a.constData());
	}

	QProcess *m_process;
	QString m_cmd;
	int m_loglevel;
	mutable boost::optional<QByteArray> m_stderr;
};

struct DefaultExecHandler : public ExecHandlerBase
{
	DefaultExecHandler(QProcess &process, const QString &cmd, int loglevel = DBG_FATAL) :
		ExecHandlerBase(process, cmd, loglevel), m_success(false)
	{
	}

	void exitCode(int code)
	{
		ExecHandlerBase::exitCode(code);
		m_success = code == 0;
	}

	bool isSuccess() const
	{
		return m_success;
	}
private:

	bool m_success;
};

class HostUtils
{
public:

	HostUtils() {};
	~HostUtils() {};

	/**
	 * Flush file buffers for QFile
	 */
	static bool FlushQFile(QFile& file);

	/**
	 * Check is file a device or an ordinary file
	 */
	static bool IsDevice(const QString& fileName);

	/**
	 * Get last error value
	 */
	static UINT GetLastError();

	/**
	 * Set last error value
	 */
	static void SetLastError( UINT err );

	/**
	 * Set size(end) of file
	 */
	static bool SetFileSize(
		#ifdef _WIN_
			HANDLE hFile,
		#else
			int hFile,
		#endif
			UINT64 uiSize
		);

#include "HostUtilsBase.h"

	// Get maximum file size for given filename
	static PRL_UINT64 GetMaxFileSize(const QString& fileName);

	// Check a mounted device to compatibility with EFI-64 boot
	static bool IsEfi64BootDevice(const QString& mountPoint);

	// Get FS type
	static PRL_FILE_SYSTEM_FS_TYPE GetFSType(const QString& fileName);

	// Get free space on disk
	static PRL_UINT64 GetFreeSpaceByName(const QString& fileName);

	// Get free space on disk
	static QString GetVolumeID(const QString& fileName);
	static QString GetVolumeGUID(const QString& fileName);

	/**
	 * Get all processes info as string table
	 * @return all processes info
	 */
	static QString GetAllProcesses(bool bDetailed = false);

	/**
	 * Get pid list by process name
	 * @return list of pids
	 */
	static QStringList GetProcessPidsByName(const QString& procName);

	/**
	 * Get more host info
	 * @return system info about host
	 */
	static QString GetMoreHostInfo();

	/**
	 * Gets output of mount utility
	 */
	static QString GetMountInfo();

	/**
	 * Gets info about loaded drivers
	 */
	static QString GetLoadedDrivers();

	/**
	 * Get memory usage
	 */
	static int GetMemoryUsage(struct HostMemUsage *mu);

	/**
	 * Get current TSC counter value ( Time Stamp Counter ).
	 * XXX: Do not use this function for time measurements !!!
	 */
	static PRL_UINT64 GetTsc()
	{
#ifdef _WIN_
		return __rdtsc();
#elif defined(_AMD64_)
		unsigned int a, d;
		__asm__ volatile ("rdtsc" : "=a" (a), "=d" (d));
		return (((PRL_UINT64)d) << 32) | ((PRL_UINT64)a);
#else
		PRL_UINT64 tsc;
		__asm__ __volatile__ ("rdtsc\n": "=A" (tsc) : : "memory");
		return tsc;
#endif
	}

	static int TimeStampToDayTimeString(UINT64 useconds, char *buf, int sz)
	{
#if defined(_LIN_)
		time_t sec = useconds/1000000l;
		int len = strftime(buf, sz, "%m-%d %H:%M:%S", localtime(&sec));
		int msec = (useconds / 1000) % 1000;
		len += snprintf(buf+len, sz-len, ".%03d", msec);
		return len;
#else
		Q_UNUSED(useconds);
		Q_UNUSED(buf);
		Q_UNUSED(sz);
		return 0;
#endif
	}


	static UINT GetAvailCpu();

	static void GetCpuid(UINT& uEAX, UINT& uECX, UINT& uEDX, UINT& uEBX);
	static UINT GetCpuidEax(UINT uLevel, UINT uSubLeaf = 0);
	static UINT GetCpuidEcx(UINT uLevel, UINT uSubLeaf = 0);
	static UINT GetCpuidEdx(UINT uLevel, UINT uSubLeaf = 0);
	static UINT GetCpuidEbx(UINT uLevel, UINT uSubLeaf = 0);
	static UINT GetCurrentRealCpuId();

	static UINT GetCPUMhz();
	static UINT GetBusMhz();

	// Copy security descriptors, ACLs, access rights and owner
	static PRL_RESULT CopyAccessRights(const QString& oldFile, const QString& newFile);

	/**
	 * Get all host registered users info
	 * @return list of users info as HostUser
	 */
	static QList<HostUser> GetAllHostUsers();

	// Check is the file path absolute or relative
	static bool isPathAbsolute(const QString& path);

	/**
	 * Checks whether current host related to the Apple hardware
	 */
	static bool CheckWhetherAppleHardware();

	static QString GetOsVersionName();

	static uint GetRecommendedMaxVmMemory( uint hostMemory );

	static unsigned long GetThreadId() {
		#ifdef _WIN_
			return (unsigned long) GetCurrentThreadId();
		#else
			return (unsigned long) pthread_self();
		#endif
	}

	static PRL_RESULT GetAtaDriveRate(const QString& devName, unsigned  int& rate);
	static PRL_RESULT GetAtaDriveRate(Qt::HANDLE devHandle, unsigned  int& rate);

	static RunCmdResult RunCmdLineUtilityEx(const QStringList& cmdline,  QProcess &process, int timeout = 0,
			void (*afterStartCallback)(QProcess*) = 0);

	static RunCmdResult RunCmdLineUtilityEx(const QString &cmdline, QProcess &process, int timeout = 0,
			void (*afterStartCallback)(QProcess*) = 0);

	/* OBSOLETED */
	/**
	 * Runs any console utility
	 * qsCmdLine [in]           - command line with arguments if need they
	 * qsOutput [out]           - standard output from running process
	 * nFinishTimeout [in][opt] - timeout of finishing running process (0 - default (30 msecs), -1 - infinity waiting)
	 * pProcess [in][opt]       - external process object
	 * pfnAfterStartCAllback [in][opt] - callback function pointer to control process after its starting
	 * [return] - true - success, false - fail
	 */
	static bool RunCmdLineUtility(const QString& qsCmdLine,
								  QString& qsOutput,
								  int nFinishTimeout = 0,	// in miliseconds
								  QProcess* pProcess = 0,
								  void (*pfnAfterStartCAllback)(QProcess* ) = 0);

	static bool RunCmdLineUtility(const QStringList& qsCmdLine,
								  QString& qsOutput,
								  int nFinishTimeout = 0,	// in miliseconds
								  QProcess* pProcess = 0,
								  void (*pfnAfterStartCAllback)(QProcess* ) = 0);


	static bool isFastRebootNodeAllowed();
	static bool ReadXCr0(UINT64* pXCr0);

	static quint32 convertWeightToIoprio(quint32 weight_);
	static quint32 convertIoprioToWeight(quint32 prio_);

	/* clear environment from unwanted vars like LD_PRELOAD */
	static void sanitizeEnv(QProcess &process, bool inplace = false);
private:
	// Get FS type (win)
	static PRL_FILE_SYSTEM_FS_TYPE GetFSType_Win(const QString& fileName);
	// Get FS type (lin)
	static PRL_FILE_SYSTEM_FS_TYPE GetFSType_Lin(const QString& fileName);

	static UINT GetHostCPUMhz();
	static UINT GetCPUMhzByTscOne();
	static UINT GetCPUMhzByTsc();
	static RunCmdResult waitProcessResult(QProcess &process, int timeout, void (*afterStartCallback)(QProcess*));
};

#endif


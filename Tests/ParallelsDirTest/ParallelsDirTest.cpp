/////////////////////////////////////////////////////////////////////////////
///
/// Copyright (c) 2007-2017, Parallels International GmbH
///
/// This file is part of Virtuozzo Core. Virtuozzo Core is free
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
/// @file
///		ParallelsDirTest.cpp
///
/// @author
///		sergeyt@
///
/// @brief
///		Tests fixture class for testing ParallelsDirs class functionality.
///
/// @brief
///
/// last version of specification is available on http://wiki/index.php/Paths_to_Configuration_Files
///
/////////////////////////////////////////////////////////////////////////////
#include "ParallelsDirTest.h"
#include "Libraries/PrlCommonUtilsBase/ParallelsDirs.h"

#include "Interfaces/ParallelsQt.h"
#include "Libraries/Logging/Logging.h"
#include <prlsdk/PrlOses.h>
#include "Build/Current.ver"

#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <errno.h>

void ParallelsDirTest::testGetDispatcherConfigDir()
{
//	QVERIFY (currentProcessHasRootPermission());

	QString expectedPath;
	QString path=ParallelsDirs::getDispatcherConfigDir();

	switch(getOsType())
	{
	case osWinXp: expectedPath="C:/Documents and Settings/All Users/Application Data/Parallels";
		break;
	case osWinVista: expectedPath="C:/ProgramData/Parallels";
		break;
	case osLinux: expectedPath="/etc/vz";
		break;
	case osMac: expectedPath="/Library/Preferences/Parallels";
		break;
	default:
		QFAIL ("Someshit happend.");
	}

	QCOMPARE(path, expectedPath);
}
void ParallelsDirTest::testGetCallerUserPreferencesDir()
{
	QString path;
	QString expectedPath = QDir::homePath();

	path=ParallelsDirs::getCallerUserPreferencesDir();

	switch(getOsType())
	{
	case osWinXp: expectedPath=expectedPath + "/Application Data/Parallels";
		break;
	case osWinVista: expectedPath=expectedPath + "/AppData/Roaming/Parallels";
		break;
	case osLinux: expectedPath=expectedPath + "/.vz";
		break;
	default:
		QFAIL ("Someshit happend.");
	}
	QCOMPARE(path, expectedPath);
}

void ParallelsDirTest::testGetDefaultVmCatalogue_serverMode()
{
//	QVERIFY (currentProcessHasRootPermission());

	QString path;
	QString expectedPath;

	path=ParallelsDirs::getCommonDefaultVmCatalogue();

	switch(getOsType())
	{
	case osWinXp: expectedPath="C:/Documents and Settings/All Users/Documents/Public Parallels";
		break;
	case osWinVista: expectedPath="C:/Users/Public/Documents/Public Parallels";
		break;
	case osLinux: expectedPath="/vz/vmprivate";
		break;
	case osMac: expectedPath="/Users/Shared/Parallels";
		break;
	default:
		QFAIL ("Someshit happend.");
	}
	QCOMPARE(path, expectedPath);
}


void ParallelsDirTest::testGetToolsBaseImagePath()
{
	// NOTE: windows path depends from product installation path
	PRL_APPLICATION_MODE mode;
	QString path;
	QString expectedPath;

	// Server
	mode = PAM_SERVER;
	path = ParallelsDirs::getToolsBaseImagePath(mode);
	expectedPath = "/usr/share/virtuozzo/";
	QCOMPARE(path, expectedPath);
}


void ParallelsDirTest::testGetToolsImage()
{
	PRL_APPLICATION_MODE mode = PAM_SERVER;
	unsigned int nOsVersion;
	QString path;
	QString expectedPath;

	nOsVersion = PVS_GUEST_VER_LIN_OTHER;
	path = ParallelsDirs::getToolsImage(mode, nOsVersion);
	expectedPath = "/usr/share/virtuozzo/vz-guest-tools-lin.iso";
	QCOMPARE(path, expectedPath);

	nOsVersion = PVS_GUEST_VER_WIN_OTHER;
	path = ParallelsDirs::getToolsImage(mode, nOsVersion);
	expectedPath = "/usr/share/virtuozzo/vz-guest-tools-win.iso";
	QCOMPARE(path, expectedPath);
}


void ParallelsDirTest::testGetLinReconfigImage()
{
	QString expectedPath;
	QString path = ParallelsDirs::getLinReconfigImage(PAM_UNKNOWN);

	expectedPath = "/usr/share/parallels-reconfiguration/reconfiguration.iso";

	QCOMPARE(path, expectedPath);
}


bool ParallelsDirTest::currentProcessHasRootPermission()
{
	return 0==getuid();
}

ParallelsDirTest::OsType ParallelsDirTest::getOsType()
{
	return osLinux;
}

void ParallelsDirTest::unixImpersonateTo(const QString& userName)
{
	m_last_euid=geteuid();
	struct passwd* pwd=getpwnam(QSTR2UTF8(userName));
	if (!pwd)
	{
		LOG_MESSAGE(DBG_FATAL, "getpwnam() failed by error %d", errno);
		return;
	}
	int ret = seteuid(pwd->pw_uid);
	(void)ret;
}

void ParallelsDirTest::unixRevertToSelf()
{
	int ret = seteuid(m_last_euid);
	(void)ret;
}

QString ParallelsDirTest::linuxGetUserHomePath(const QString& userName)
{
	m_last_euid=geteuid();
	struct passwd* pwd=getpwnam(QSTR2UTF8(userName));
	if (!pwd)
	{
		LOG_MESSAGE(DBG_FATAL, "getpwnam() failed by error %d", errno);
		return "";
	}
	return UTF8_2QSTR(pwd->pw_dir);
}


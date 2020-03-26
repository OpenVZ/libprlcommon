/////////////////////////////////////////////////////////////////////////////
///
/// Copyright (c) 2007-2017, Parallels International GmbH
/// Copyright (c) 2017-2019 Virtuozzo International GmbH, All rights reserved.
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
/// Our contact details: Virtuozzo International GmbH, Vordergasse 59, 8200
/// Schaffhausen, Switzerland.
///
/// @file
///		VirtuozzoDirTest.cpp
///
/// @author
///		sergeyt@
///
/// @brief
///		Tests fixture class for testing VirtuozzoDirs class functionality.
///
/// @brief
///
/// last version of specification is available on http://wiki/index.php/Paths_to_Configuration_Files
///
/////////////////////////////////////////////////////////////////////////////
#include "VirtuozzoDirTest.h"
#include "Libraries/PrlCommonUtilsBase/VirtuozzoDirs.h"

#include "Interfaces/VirtuozzoQt.h"
#include "Libraries/Logging/Logging.h"
#include <prlsdk/PrlOses.h>
#include "Build/Current.ver"

#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <errno.h>

void VirtuozzoDirTest::testGetDispatcherConfigDir()
{
//	QVERIFY (currentProcessHasRootPermission());

	QString expectedPath;
	QString path=VirtuozzoDirs::getDispatcherConfigDir();

	switch(getOsType())
	{
	case osLinux: expectedPath="/etc/vz";
		break;
	default:
		QFAIL ("Someshit happend.");
	}

	QCOMPARE(path, expectedPath);
}
void VirtuozzoDirTest::testGetCallerUserPreferencesDir()
{
	QString path;
	QString expectedPath = QDir::homePath();

	path=VirtuozzoDirs::getCallerUserPreferencesDir();

	switch(getOsType())
	{
	case osLinux: expectedPath=expectedPath + "/.vz";
		break;
	default:
		QFAIL ("Someshit happend.");
	}
	QCOMPARE(path, expectedPath);
}

void VirtuozzoDirTest::testGetDefaultVmCatalogue_serverMode()
{
//	QVERIFY (currentProcessHasRootPermission());

	QString path;
	QString expectedPath;

	path=VirtuozzoDirs::getCommonDefaultVmCatalogue();

	switch(getOsType())
	{
	case osLinux: expectedPath="/vz/vmprivate";
		break;
	default:
		QFAIL ("Someshit happend.");
	}
	QCOMPARE(path, expectedPath);
}


void VirtuozzoDirTest::testGetToolsBaseImagePath()
{
	// NOTE: windows path depends from product installation path
	PRL_APPLICATION_MODE mode;
	QString path;
	QString expectedPath;

	// Server
	mode = PAM_SERVER;
	path = VirtuozzoDirs::getToolsBaseImagePath(mode);
	expectedPath = "/usr/share/vz-guest-tools/";
	QCOMPARE(path, expectedPath);
}


void VirtuozzoDirTest::testGetToolsImage()
{
	PRL_APPLICATION_MODE mode = PAM_SERVER;
	unsigned int nOsVersion;
	QString path;
	QString expectedPath;

	nOsVersion = PVS_GUEST_VER_LIN_OTHER;
	path = VirtuozzoDirs::getToolsImage(mode, nOsVersion);
	expectedPath = "/usr/share/vz-guest-tools/vz-guest-tools-lin.iso";
	QCOMPARE(path, expectedPath);

	nOsVersion = PVS_GUEST_VER_WIN_OTHER;
	path = VirtuozzoDirs::getToolsImage(mode, nOsVersion);
	expectedPath = "/usr/share/vz-guest-tools/vz-guest-tools-win.iso";
	QCOMPARE(path, expectedPath);
}


bool VirtuozzoDirTest::currentProcessHasRootPermission()
{
	return 0==getuid();
}

VirtuozzoDirTest::OsType VirtuozzoDirTest::getOsType()
{
	return osLinux;
}

void VirtuozzoDirTest::unixImpersonateTo(const QString& userName)
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

void VirtuozzoDirTest::unixRevertToSelf()
{
	int ret = seteuid(m_last_euid);
	(void)ret;
}

QString VirtuozzoDirTest::linuxGetUserHomePath(const QString& userName)
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


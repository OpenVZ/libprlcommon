/*
 * VirtuozzoDirs.h: Helper class for getting default virtuozzo
 * configs locations.
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


#ifndef __VIRTUOZZO_DIRS_H__
#define __VIRTUOZZO_DIRS_H__

#include <QString>
#include <QFile>
#include <QPair>
#include <QFlags>
#include <QMutex>
#include "../Interfaces/VirtuozzoNamespace.h"
#include <prlsdk/PrlEnums.h>
#include "VirtuozzoDirsDefs.h"

class QDir;

class VirtuozzoDirs
{
public:

#include "VirtuozzoDirsBase.h"

	static QString getAppGuiName( PRL_APPLICATION_MODE nAppMode );
	static QString getAppSwitcherAppName();
	static QString getLearnVideoAppName();

	static QString getDispatcherConfigDir();
    static QString getConfigScriptsDir();
	static QString getDispatcherConfigFilePath();
	static QString getDispatcherVmCatalogueFilePath();
	static QString getLicensesFilePath();

	// return  path to NetworkConfigFile for current app mode.
	static QString getNetworkConfigFilePath();

	// return  path to NetworkConfigFile for specified appMode.
	static QString getNetworkConfigFilePath( PRL_APPLICATION_MODE appMode );

	static QString getCallerUserPreferencesDir();

	static QString getCommonDefaultVmCatalogue();
	static QString getUserDefaultVmCatalogue(const VirtuozzoDirs::UserInfo* pUserInfo);

	static QString getUserHomePath(const VirtuozzoDirs::UserInfo* pUserInfo);

	// #132602
	static QString getPathToDispatcherTesterConfig();

	static QString getVirtuozzoDriversDir();
	/// returns path to the directory where Virtuozzo applications is located
	static QString getVirtuozzoApplicationDir();
	static QString getVirtuozzoScriptsDir();

	/// returns true if build is runt under developers build environment.
	static bool isDevelopersBuild();

	//get system temp directory
	static QString getSystemTempDir();

	/**
	 * Get current user temp directory. Actually it invokes GetTempDirW for
	 * windows platform, and getSystemTempDir for others.
	 */
	static QString getCurrentUserTempDir();

	// get Windows applications directory -
	// in this directory associations with Guest windows application stored
	static QString getMappingApplicationsDir(const QString & strVmHomeDir);

	// get Windows Disks directory -
	// in this directory associations with Guest windows disks stored
	static QString getMappingDisksDir(const QString & strVmHomeDir);

	// get Snapshots directory -
	// in this directory Snapshot storage present
	static QString getSnapshotsDir(const QString & strVmHomeDir);

	// get Guest Crash Dumps directory -
	// in this directory guest crash dumps stored
	static QString getVmGuestCrashDumpsDir(const QString & strVmHomeDir);

	// get full path to VmInfo file for VM with [strVmHomeDir] home path
	static QString getVmInfoPath(const QString & strVmHomeDir);

	// get Virtuozzo application directory
	static QString getVirtuozzoDirName();

	// get base path to Virtuozzo Tools .iso image
	static QString getToolsBaseImagePath(PRL_APPLICATION_MODE mode);
	// get full path to .iso with Virtuozzo Tools for the given guest OS
	// Note: if function returns an empty string then OS is not supported
	static QString getToolsImage(PRL_APPLICATION_MODE mode, unsigned int nOsVersion);

	// get full path to .tar.gz with Virtuozzo Tools for the given guest OS
	// Note: if function returns an empty string then OS is not supported
	static QString getToolsTarGz(PRL_APPLICATION_MODE mode, unsigned int nOsVersion);

	// get Virtuozzo Tools installer name for the given guest OS
	// Note: if function returns an empty string then OS is not supported
	static QString getToolsInstallerName(unsigned int nOsVersion);

	// get full path to .fdd with Virtuozzo Tools
	// return "" if fdd image doesn't required for this guest
	static QString getFddToolsImage( PRL_APPLICATION_MODE mode, unsigned int uGuestOsType );

	// get full path to .fdd with Virtuozzo Tools
	static QString getFddToolsImageBaseName( unsigned int uGuestOsType );

	// get full path to .iso with Virtuozzo Tools
	static QString getToolsFileName(unsigned int uGuestOsType);

	// get full path to .iso with unattended floppy drivers
	static QString getWindowsUnattendedFloppy(unsigned int osVersion_);
	static QString getCrashDumpsPath();

	static QString getSystemLogPath();
	static QString getDefaultSystemLogPath(PRL_APPLICATION_MODE nAppMode);
	static QString getClientLogPath();

	static QString getCurrentUserHomeDir();

	static QString getPVAInstallLogPath();

	// developers issue to load app mode from file "appPath" + ".params"
	//		need to start applications from Run.py
	// if error - return PAM_UNKNOWN
	static QPair<PRL_APPLICATION_MODE,InitOptions> loadAppExecuteMode( const QString& appPath );

	//return execution mode specified during
	//Gen.py [forceserver|...].
	//Gen.py save according macros,
	//then getBuildExecutionMode() select mode by macros.
	//return PAM_UNKNOWN if macros are not specified
	static PRL_APPLICATION_MODE getBuildExecutionMode();

	static QString getDefaultPramPath();
	static QString getDefaultBackupDir();

	static QString getDefaultSwapPathForVMOnNetworkShares();

	static QStringList getInstallationLogFilePaths();

	// Get path to dispatcher local UNIX socket
	static QString getDispatcherLocalSocketPath();

	// returns Vm memory file location (directory)
	static QString getVmMemoryFileLocation(
		const QString &sVmUuid,
		const QString &sVmHomeDir,
		const QString &sSwapDir,
		const QString &sSwapPathForSharedVm,
		bool bUseSwapPathForSharedVm,
		UINT64 uMemSize);

	static QString getVmAppPath(bool bX64);
	static QString getVmStarterPath();
	static QString getConvertToolPath( const QDir& baseDir );
	static QString getDiskToolPath( const QDir& baseDir );
	static QString getVmScriptsDir(const QString &sBaseDir);
	static QString getVmActionScriptPath(const QString &sBaseDir, PRL_VM_ACTION nAction);
	static QString getVmConfigurationSamplePath(const QString &sName);
	static QString getUpdaterUrl(PRL_APPLICATION_MODE appMode);

	static QString getServiceAppName();

private:
	static QString getDefaultVmCatalogue(const VirtuozzoDirs::UserInfo* pUserInfo);
	static QString getIPCPath(const QString& fileName, const QString& humanName);

public:
	struct UserInfo
	{
		UserInfo( const QString& userName, const QString& homePath);

		UserInfo( const UserInfo& );
		explicit UserInfo();

		UserInfo& operator=( const UserInfo& );
		void printUserInfo();
		QString getHomePath() const {return m_homePath; }

	private:
		friend class VirtuozzoDirs;

		bool isValid()  const;

		QString m_userName;
		QString m_homePath;
	}; //UserInfo

private:
	static PRL_APPLICATION_MODE ms_nApplicationMode;
	static bool ms_bAppModeInited;
	static InitOptions ms_nInitOptions;

};
Q_DECLARE_OPERATORS_FOR_FLAGS( VirtuozzoDirs::InitOptions );

#endif // __VIRTUOZZO_DIRS_H__


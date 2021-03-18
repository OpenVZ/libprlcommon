/*
 * VirtuozzoDirs.cpp: Helper class for getting default virtuozzo
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

#include "VirtuozzoDirs.h"
#include "Interfaces/VirtuozzoQt.h"

#include "Libraries/Logging/Logging.h"
#include "Libraries/HostUtils/HostUtils.h"
#include "Interfaces/VirtuozzoTypes.h"
#include <prlsdk/PrlOses.h>
#include "Libraries/PrlCommonUtilsBase/Common.h"
#include "Libraries/Std/PrlAssert.h"

#include "CommandLine.h"
#include "OsInfo.h"
#include "VirtuozzoDirsDefs.h"

#include <QCoreApplication>
#include <QFileInfo>
#include <QDir>
#include <QSettings>
#include <QProcess>
#include <QLocale>
#include <QMutexLocker>

#include "Build/Current.ver"
// #include "Build/Current-locale.ver"

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/un.h>
#include <pwd.h>
#include <errno.h>
#include <string.h>
#include <sys/utsname.h>
#include <sys/stat.h>


PRL_APPLICATION_MODE VirtuozzoDirs::ms_nApplicationMode =  PAM_UNKNOWN;
VirtuozzoDirs::InitOptions VirtuozzoDirs::ms_nInitOptions = VirtuozzoDirs::smNormalMode;
bool VirtuozzoDirs::ms_bAppModeInited =  false;

namespace
{
	const char g_strVirtuozzoDirName[] = "vz";
}

VirtuozzoDirs::UserInfo::UserInfo()
{
}

VirtuozzoDirs::UserInfo::UserInfo(const QString& userName, const QString& homePath)
: m_userName(userName),
  m_homePath( homePath )
{
}

bool VirtuozzoDirs::UserInfo::isValid()  const
{
	return !m_userName.isEmpty();
}

VirtuozzoDirs::UserInfo::UserInfo( const VirtuozzoDirs::UserInfo& ui)
{
		m_userName = ui.m_userName;
		m_homePath = ui.m_homePath;
}

VirtuozzoDirs::UserInfo& VirtuozzoDirs::UserInfo::operator=( const VirtuozzoDirs::UserInfo& ui )
{
	if( &ui == this )
		return *this;

	m_userName = ui.m_userName;
	m_homePath = ui.m_homePath;

	return *this;
}

void VirtuozzoDirs::UserInfo::printUserInfo()
{
	WRITE_TRACE( DBG_WARNING, "User with home path %s",
					QSTR2UTF8( m_homePath ) );
}

QString VirtuozzoDirs::getConfigScriptsDir()
{
    PRL_APPLICATION_MODE appMode = getAppExecuteMode();
    QString folder;
    switch( appMode )
    {
    case PAM_SERVER: folder = "server"; break;
    default:
        PRL_ASSERT(false);
        return "";
    }
    return QDir::toNativeSeparators(getDispatcherConfigDir() + '/' + folder);
}

QString VirtuozzoDirs::getDispatcherConfigDir()
{
	//Lin: /etc/vz

	QString path;

	do
	{
		path = Prl::getenvU(PVS_DISPATCHER_CONFIG_DIR_ENV);
		if( !path.isEmpty () )
		{
			WRITE_TRACE(DBG_FATAL, "PVS_DISPATCHER_CONFIG_DIR_ENV: was set from enviroment: '%s'"
				, QSTR2UTF8( path ) );
			break;
		}

		path="/etc/";

		path+=UTF8_2QSTR(g_strVirtuozzoDirName);

		path=QDir::fromNativeSeparators(path);
	}while(0);
	return path;
}

QString VirtuozzoDirs::getDispatcherConfigFilePath()
{
	QString fName;

	PRL_APPLICATION_MODE appMode = getAppExecuteMode();
	switch( appMode )
	{
	case PAM_SERVER:
		fName = DISPATCHER_CONFIGURATION_SERVER_XML_FILE;
		break;
	default:
		fName = "fake.disp.xml";
		WRITE_TRACE(DBG_FATAL, "%s:  Not supported appMode = %d. config fname = %s"
			, __FUNCTION__
			, appMode
			, QSTR2UTF8( fName ) );
	}

	QString strDispConfigFile = QString( "%1/%2" ).
		arg( getDispatcherConfigDir() ).
		arg( fName );

	return strDispConfigFile;
}

QString VirtuozzoDirs::getDispatcherVmCatalogueFilePath()
{
	QString fName;

	PRL_APPLICATION_MODE appMode = getAppExecuteMode();
	switch( appMode )
	{
	case PAM_SERVER:
		fName = VMDIR_DEFAULT_CATALOGUE_SERVER_FILE;
		break;
	default:
		fName = "fake.vmdirlist.xml";
		WRITE_TRACE(DBG_FATAL, "%s:  Not supported appMode = %d. config fname = %s", __FUNCTION__, appMode, QSTR2UTF8( fName ) );
	}

	QString path = QString( "%1/%2" ).
		arg( getDispatcherConfigDir() ).
		arg( fName );
	return path;
}

QString VirtuozzoDirs::getLicensesFilePath()
{
	QString strLicensesFilePath = QString( "%1/%2" ).
		arg( getDispatcherConfigDir() ).
		arg( VIRTUOZZO_LICENSES_XML_FILE );

	return strLicensesFilePath;
}

QString VirtuozzoDirs::getNetworkConfigFilePath( PRL_APPLICATION_MODE appMode )
{
	QString fName;
	switch( appMode )
	{
	case PAM_SERVER:
		fName = NETWORK_CONFIGURATION_SERVER_XML_FILE;
		break;
	default:
		WRITE_TRACE(DBG_FATAL, "Not supported appMode = %d", appMode );
		return "";
	}

	QString path = QString( "%1/%2" ).
		arg( getDispatcherConfigDir() ).
		arg( fName );

	return path;
}


QString VirtuozzoDirs::getNetworkConfigFilePath()
{
	PRL_APPLICATION_MODE appMode = getAppExecuteMode();
	QString path = getNetworkConfigFilePath( appMode );
	if( path.isEmpty() )
	{
		path = QString( "%1/%2" ).
			arg( getDispatcherConfigDir() ).
			arg( "fake.network.xml" );
		WRITE_TRACE(DBG_FATAL, "%s:  Not supported appMode = %d. config fname = %s"
			, __FUNCTION__
			, (int)appMode
			, QSTR2UTF8( QFileInfo(path).fileName() ) );
	}

	return path;
}

QString VirtuozzoDirs::getCallerUserPreferencesDir()
{
//Lin: $HOME/.vz

	QString path;
	do
	{
		uid_t euid=geteuid();
		struct passwd* pPswd=getpwuid(euid);
		if (!pPswd || !pPswd->pw_dir || !strlen(pPswd->pw_dir) )
		{
			WRITE_TRACE (DBG_FATAL, "Can't get profile by error %d, pswd=%p, pw_dir=%p"
				, errno, pPswd, pPswd?pPswd->pw_dir:"null");
			break;
		}
		//get home
		path=UTF8_2QSTR(pPswd->pw_dir);
		path+="/";
		path+= QString(".") + UTF8_2QSTR(g_strVirtuozzoDirName);
		path=QDir::fromNativeSeparators(path);

	}while(0);
	return path;
}

QString VirtuozzoDirs::getUserDefaultVmCatalogue(const VirtuozzoDirs::UserInfo*pUserInfo)
{
	return getDefaultVmCatalogue(pUserInfo);
}


QString VirtuozzoDirs::getCommonDefaultVmCatalogue()
{
	// Call PRIVATE METHOD. THAT SUPPORT ONLY SERVER or DESKTOP mode.
	return getDefaultVmCatalogue(0);

}

QString VirtuozzoDirs::getUserHomePath(const VirtuozzoDirs::UserInfo* pUserInfo)
{
	if ( ! pUserInfo || ! pUserInfo->isValid() )
		return "";

	return pUserInfo->m_homePath;
}

QString VirtuozzoDirs::getDefaultVmCatalogue(
	const VirtuozzoDirs::UserInfo* pUserInfo
	)
{
//Lin:
//Server: /vz/vmprivate

	QString path;
	do
	{
		// Try get strDefaultCommonVmCatalogueDir value from environment
		if(NULL == pUserInfo)
		{
			path = Prl::getenvU(PVS_DISPATCHER_CONFIG_DIR_ENV);
			if( !path.isEmpty () )
			{
				WRITE_TRACE(DBG_FATAL, "PVS_VMCATALOGUE_DIR_ENV: was set from enviroment: '%s'"
					, QSTR2UTF8( path ) );
				break;
			}
		}

		{
			if(NULL == pUserInfo)
			{
				path = "/vz/vmprivate";
			}
			else
			{
				struct passwd* pwd=getpwnam(QSTR2UTF8(pUserInfo->m_userName));
				if (!pwd || !pwd->pw_dir || !strlen(pwd->pw_dir) )
				{
					WRITE_TRACE (DBG_FATAL, "Can't get profile by error %d, pswd=%p, pw_dir=%p"
						, errno, pwd, pwd?pwd->pw_dir:"null");
					break;
				}
				path=UTF8_2QSTR(pwd->pw_dir);//home dir
				path+=QString("/")+ UTF8_2QSTR(g_strVirtuozzoDirName);
			}

		}
		path=QDir::fromNativeSeparators(path);

	} while(0);

	return path;
}

QString VirtuozzoDirs::getVirtuozzoApplicationDir()
{
	QString sVirtuozzoInstallDir = Prl::getenvU(PVS_VM_EXECUTABLE_ENV);
	if( sVirtuozzoInstallDir.isEmpty() )
	{
		sVirtuozzoInstallDir = QCoreApplication::applicationDirPath();
	}
	return sVirtuozzoInstallDir;
}


QString VirtuozzoDirs::getVirtuozzoScriptsDir()
{
	QString sVirtuozzoScriptsDir = Prl::getenvU(PVS_VM_SCRIPTS_ENV);
	if( sVirtuozzoScriptsDir.isEmpty() )
	{
		QDir dir(QCoreApplication::applicationDirPath());
		dir.cd("scripts");
		sVirtuozzoScriptsDir = dir.absolutePath();
	}
	return sVirtuozzoScriptsDir;
}


QString VirtuozzoDirs::getVirtuozzoDriversDir()
{
	QString currDir = QCoreApplication::applicationDirPath();

	// 1. get parent directory of binary
	QString parentOfBinDir;

	parentOfBinDir = currDir + "/..";

	// 2. if it is 'z-Build' is development version
	//      otherwise - installed version

	parentOfBinDir = QDir( parentOfBinDir ).absolutePath();

	QString path;

	if ( QDir( parentOfBinDir ).dirName() == "z-Build" )
		path = parentOfBinDir + "/Drivers";
	else
	{
		struct utsname un;
		if ( 0 != uname ( &un ))
			WRITE_TRACE(DBG_FATAL, "can't get kernel version err = %d, %s" , errno, strerror( errno ) );
		else
		{
			path = "/lib/modules/";
			path += un.release;
		}//if uname
	}

	LOG_MESSAGE(DBG_DEBUG, "This path = (%s)", QSTR2UTF8( path ) );

	return path;
}


// returns true if we are runt under developers build environment
bool VirtuozzoDirs::isDevelopersBuild()
{
	static bool s_bBuildModeInitialized = false;
	static bool s_bDevelopersBuild = false;

	if (!s_bBuildModeInitialized)
	{
		s_bBuildModeInitialized = true;

		QString currDir = QCoreApplication::applicationDirPath();
		currDir = QDir::toNativeSeparators(QDir( currDir ).absolutePath());
		if ( currDir.contains(QDir::toNativeSeparators("z-Build/Release")) ||
				currDir.contains(QDir::toNativeSeparators("z-Build/Debug")) )
			s_bDevelopersBuild = true;
		else
			s_bDevelopersBuild = false;
	}
	return s_bDevelopersBuild;
}

QString VirtuozzoDirs::getSystemTempDir()
{
	return "/vz/tmp";
}

QString VirtuozzoDirs::getCurrentUserTempDir()
{
	return VirtuozzoDirs::getSystemTempDir();
}

// get Mapping applications directory -
// in this directory associations with Guest windows application stored
QString VirtuozzoDirs::getMappingApplicationsDir(const QString & strVmHomeDir)
{
	return strVmHomeDir + "/" + QObject::tr(VM_GENERATED_WINDOWS_APPLICATION_DIR);
}

// get Mapping Disks directory -
// in this directory associations with Guest disks stored
QString VirtuozzoDirs::getMappingDisksDir(const QString & strVmHomeDir)
{
	return strVmHomeDir + "/" + QObject::tr(VM_GENERATED_WINDOWS_WIN_DISKS_DIR);
}

// get Snapshots directory -
// in this directory Snapshot storage present
QString VirtuozzoDirs::getSnapshotsDir(const QString & strVmHomeDir)
{
	return strVmHomeDir + "/" + QObject::tr(VM_GENERATED_WINDOWS_SNAPSHOTS_DIR);
}

// get Guest Crash Dumps directory -
// in this directory guest crash dumps stored
QString VirtuozzoDirs::getVmGuestCrashDumpsDir(const QString & strVmHomeDir)
{
	return strVmHomeDir + "/" + VM_GENERATED_GUEST_CRASH_DUMPS_DIR;
}

// get full path to VmInfo file for VM with [strVmHomeDir] home path
QString VirtuozzoDirs::getVmInfoPath(const QString & strVmHomeDir)
{
	return strVmHomeDir + "/" + VM_INFO_FILE_NAME;
}

// get Virtuozzo application directory
QString VirtuozzoDirs::getVirtuozzoDirName()
{
	return QString(g_strVirtuozzoDirName);
}

// get base path to Virtuozzo Tools .iso image
QString VirtuozzoDirs::getToolsBaseImagePath(PRL_APPLICATION_MODE mode)
{
	QString path;

	if (mode == PAM_SERVER)
		path = "/usr/share/vz-guest-tools/";
	return path;
}

QString VirtuozzoDirs::getToolsImage(PRL_APPLICATION_MODE mode, unsigned int nOsVersion)
{
	QString qsFileName;
	if (IS_WINDOWS(nOsVersion) && (nOsVersion >= PVS_GUEST_VER_WIN_2K))
	{
		qsFileName = "vz-guest-tools-win.iso";
	}
	else if IS_MACOS(nOsVersion)
	{
		qsFileName = "vz-guest-tools-mac.iso";
	}
	else if IS_LINUX(nOsVersion)
	{
		qsFileName = "vz-guest-tools-lin.iso";
	}
	else
	{
		qsFileName = "vz-guest-tools-other.iso";
	}

	QString qsToolsImage;
	if (!qsFileName.isEmpty())
	{
		qsToolsImage = getToolsBaseImagePath(mode) + qsFileName;
	}
	return qsToolsImage;
}

QString VirtuozzoDirs::getToolsTarGz(PRL_APPLICATION_MODE mode, unsigned int nOsVersion)
{
	QString qsFileName;
	if (IS_WINDOWS(nOsVersion) && (nOsVersion >= PVS_GUEST_VER_WIN_2K))
	{
		qsFileName = "vz-guest-tools-win.tar.gz";
	}
	else if IS_MACOS(nOsVersion)
	{
		qsFileName = "vz-guest-tools-mac.tar.gz";
	}
	else if IS_LINUX(nOsVersion)
	{
		qsFileName = "vz-guest-tools-lin.tar.gz";
	}
	else
	{
		qsFileName = "vz-guest-tools-other.tar.gz";
	}

	QString qsToolsImage;
	if (!qsFileName.isEmpty())
	{
		qsToolsImage = getToolsBaseImagePath(mode) + qsFileName;
	}
	return qsToolsImage;
}

QString VirtuozzoDirs::getToolsInstallerName(unsigned int nOsVersion)
{
	QString qsFileName;
	if (IS_WINDOWS(nOsVersion) && (nOsVersion >= PVS_GUEST_VER_WIN_2K))
	{
		qsFileName = "vz-guest-tools-win.tar.gz";
	}
	else if IS_MACOS(nOsVersion)
	{
		qsFileName = "vz-guest-tools-mac.iso";
	}
	else if IS_LINUX(nOsVersion)
	{
		qsFileName = "vz-guest-tools-lin.iso";
	}

	return qsFileName;
}

// get base fil name of .fdd Virtuozzo Tools
QString VirtuozzoDirs::getFddToolsImageBaseName( unsigned int uGuestOsType )
{
	if (uGuestOsType == PVS_GUEST_TYPE_OS2)
		return "vz-guest-tools-os2.fdd";
	else
		return "";
}
// get full path to .fdd with Virtuozzo Tools
QString VirtuozzoDirs::getFddToolsImage( PRL_APPLICATION_MODE mode, unsigned int uGuestOsType )
{
	if (uGuestOsType == PVS_GUEST_TYPE_OS2 )
		return getToolsBaseImagePath(mode) + VirtuozzoDirs::getFddToolsImageBaseName( uGuestOsType );
	else
		return "";
}

static const char* getShortWinVersion(unsigned int osVersion_)
{
	switch (osVersion_)
	{
	case PVS_GUEST_VER_WIN_2003:
		return "2003";
	case PVS_GUEST_VER_WIN_2008:
		return "2008";
	case PVS_GUEST_VER_WIN_WINDOWS7:
		return "7";
	case PVS_GUEST_VER_WIN_WINDOWS8:
		return "8";
	case PVS_GUEST_VER_WIN_WINDOWS8_1:
		return "8_1";
    case PVS_GUEST_VER_WIN_WINDOWS10:
        return "10";
	case PVS_GUEST_VER_WIN_2012:
		return "2012";
	case PVS_GUEST_VER_WIN_2016:
		return "2016";
	case PVS_GUEST_VER_WIN_2019:
		return "2019";
	}
	return "";
}

QString VirtuozzoDirs::getWindowsUnattendedFloppy(unsigned int osVersion_)
{
	PRL_ASSERT(IS_WINDOWS(osVersion_));

	return QString("/usr/share/vz-guest-tools/floppy_win%1.vfd").arg(getShortWinVersion(osVersion_));
}

namespace{
	QStringList getSubDirsPaths( const QString& sPath )
	{
		QStringList lst;

		QFileInfo fi(sPath);
		PRL_ASSERT(fi.isAbsolute());
		if( !fi.exists() )
			return lst;

		QDir::Filters
			dirFilter  = QDir::AllDirs | QDir::NoDotAndDotDot | QDir::Hidden | QDir::NoSymLinks;

		QFileInfoList fiList = QDir(sPath).entryInfoList( dirFilter );

		foreach( QFileInfo fi, fiList )
		{
			lst << fi.absoluteFilePath();
			if(fi.isSymLink())
				continue;
			// #125021 to prevent infinity recursion by QT bug in QDir::entryInfoList()
			if( QFileInfo(sPath) != fi )
				lst << getSubDirsPaths( fi.absoluteFilePath() );
		}

		return lst;
	}

} // namespace

QString VirtuozzoDirs::getCrashDumpsPath()
{
	return getSystemTempDir() + "/vz_crash_dumps";
}

QString VirtuozzoDirs::getSystemLogPath()
{
	return QString("%1/%2").arg(UTF8_2QSTR(GetDefaultLogFilePath()))
						   .arg(GetProdDefaultLogFileName());
}

QString VirtuozzoDirs::getDefaultSystemLogPath(PRL_APPLICATION_MODE)
{
	QString fileName = PRL_LOG_FILE_NAME_DEFAULT;
	return QString("%1/%2").arg(UTF8_2QSTR(GetDefaultLogFilePath()))
						   .arg(fileName);
}

QString VirtuozzoDirs::getClientLogPath()
{
	return QString("%1/%2").arg(UTF8_2QSTR(GetUserHomeDir()))
						   .arg(GetProdDefaultLogFileName());
}

// get for currently logged user home path
QString VirtuozzoDirs::getCurrentUserHomeDir()
{
	//Lin: $HOME

	QString path;
	do
	{
		uid_t euid=geteuid();
		struct passwd* pPswd=getpwuid(euid);
		if (!pPswd || !pPswd->pw_dir || !strlen(pPswd->pw_dir) )
		{
			WRITE_TRACE (DBG_FATAL, "Can't get profile by error %d, pswd=%p, pw_dir=%p"
				, errno, pPswd, pPswd?pPswd->pw_dir:"null");
			break;
		}

		//get home
		path=UTF8_2QSTR(pPswd->pw_dir);
		path=QDir::fromNativeSeparators(path);
	}while(0);
	return path;
}

// get full path to .iso with Virtuozzo Tools
QString VirtuozzoDirs::getToolsFileName(unsigned int uGuestOsType)
{
	QString strToolsPath;

	if (IS_WINDOWS(uGuestOsType))
	{
		strToolsPath = "vz-guest-tools-win.iso";
	}
	else if IS_MACOS(uGuestOsType)
	{
		strToolsPath = "vz-guest-tools-mac.iso";
	}
	else if IS_LINUX(uGuestOsType)
	{
		strToolsPath = "vz-guest-tools-lin.iso";
	}

	return strToolsPath;
}

QString VirtuozzoDirs::getPathToDispatcherTesterConfig()
{
	return QString( "%1/%2" )
		.arg( getDispatcherConfigDir())
		.arg( "dispatcher.tester.conf" );
}

QPair<PRL_APPLICATION_MODE, VirtuozzoDirs::InitOptions>
	VirtuozzoDirs::loadAppExecuteMode( const QString& appPath )
{
	QFile fAppMode( appPath + ".params" );
	if( ! fAppMode.open( QIODevice::ReadOnly ) )
	{
		WRITE_TRACE( DBG_INFO, "Can't open file '%s' ", QSTR2UTF8( fAppMode.fileName() ) );
		return qMakePair(PAM_UNKNOWN, InitOptions(smNormalMode) );
	}

	QString  argvCmdLine =  QString("%1 %2")
								.arg(appPath)
								.arg( QString( fAppMode.readLine() ) );
	CommandLine::Parser parser(argvCmdLine);

	//////////////////////////////////////////////////////////////////////////
	// set execute mode
	PRL_APPLICATION_MODE mode = PAM_UNKNOWN;

	const QString value=parser.getValueByKey(CommandLine::g_strCommonKeyName_ModeName);
	if (value==CommandLine::g_strCommonValue_ModeName_PS)
		mode = PAM_SERVER;
	else
		WRITE_TRACE( DBG_INFO, "Wrong value of execute mode %s.", QSTR2UTF8(value) );

	InitOptions subMode = smNormalMode;
	if( parser.hasKey(CommandLine::g_strCommonKeyName_AppStoreMode) )
		subMode = smAppStoreMode;

	return qMakePair(mode, subMode);
}

PRL_APPLICATION_MODE VirtuozzoDirs::getBuildExecutionMode()
{
	PRL_APPLICATION_MODE mode = PAM_SERVER;
	return mode;
}

QString VirtuozzoDirs::getDefaultBackupDir()
{
	QString sVirtuozzoBackupDir = getCommonDefaultVmCatalogue();
	sVirtuozzoBackupDir += "/backups";
	return sVirtuozzoBackupDir;
}

QString VirtuozzoDirs::getDefaultPramPath()
{
	return "/mnt/pram_vms";
}

QString VirtuozzoDirs::getAppGuiName( PRL_APPLICATION_MODE nAppMode )
{
	switch (nAppMode)
	{
		case PAM_SERVER: return PRL_PRODUCT_NAME_SERVER;
		default: return PRL_PRODUCT_NAME_UNKNOWN;
	}
}

QString	VirtuozzoDirs::getAppSwitcherAppName()
{
	return PRL_APP_SWITCHER_NAME;
}

QString VirtuozzoDirs::getLearnVideoAppName()
{
	return PRL_APP_LEARN_VIDEO_NAME;
}

#define VM_SWAP_SUBDIR	"swap"

QString VirtuozzoDirs::getDefaultSwapPathForVMOnNetworkShares()
{
	QString sqSwapPath = QString("/var/.") + UTF8_2QSTR(g_strVirtuozzoDirName)  +
						 QString("_") + QString(VM_SWAP_SUBDIR);

	return sqSwapPath;
}


// Get Installation log file path
QStringList VirtuozzoDirs::getInstallationLogFilePaths()
{
	QStringList lstPathes;

	PRL_APPLICATION_MODE appMode = getAppExecuteMode();
	switch( appMode )
	{
		case PAM_SERVER:
			lstPathes << "/var/log/yum.log" << "/var/log/anaconda/anaconda.packaging.log";
			break;
		default:
			WRITE_TRACE(DBG_FATAL, "%s:  Not supported appMode = %d"
				, __FUNCTION__
				, appMode );
	}

	WRITE_TRACE( DBG_DEBUG, "installation log pathes == %s",
		QSTR2UTF8( lstPathes.join("\n") ) );
	return lstPathes;
}

static QString getLocalUnixSocketFileName()
{
	return "prl_disp_service.socket";
}

QString VirtuozzoDirs::getDispatcherLocalSocketPath()
{
	return getIPCPath(getLocalUnixSocketFileName(), "UnixSockPath");
}

QString VirtuozzoDirs::getIPCPath( const QString& fileName, const QString& humanName)
{
	QString path;
	path = QString( "/var/run/%1" ).arg(fileName);

	LOG_MESSAGE(DBG_FATAL, "========== %s: %s='%s' ", __FUNCTION__
		, QSTR2UTF8(humanName), QSTR2UTF8(path));

	return path;
}

/*
parameters are:
- Vm uuid
- Vm home directory (not config path)
- per-Vm swap dir
- dispatcher's swap path for shared Vm
- use dispatcher's swap path for shared Vm by default
*/
QString VirtuozzoDirs::getVmMemoryFileLocation(
		const QString &sVmUuid,
		const QString &sVmHomeDir,
		const QString &sSwapDir,
		const QString &sSwapPathForSharedVm,
		bool bUseSwapPathForSharedVm,
		UINT64 uMemSize)
{
	QString sMemFilePath;

	if (sSwapDir.isEmpty()) {
		PRL_UINT64 uMaxFileSize = HostUtils::GetMaxFileSize(sVmHomeDir);

		if (bUseSwapPathForSharedVm || (uMaxFileSize < uMemSize)) {

			sMemFilePath = QString( "%1/%2" ).arg( sSwapPathForSharedVm ).arg( sVmUuid );
			QDir dir(sMemFilePath);
			if (!dir.exists()) {
				//
				// Create subdirectory only
				//
				dir.mkdir(sMemFilePath);
				//
				// And we are not going to check if we failed or succeeded. Check has to be done from above
				//
			}
		} else {
			sMemFilePath = sVmHomeDir;
		}
	} else {
		sMemFilePath = sSwapDir;
		Prl::ProcessEnvVariables(sMemFilePath);
	}
	return sMemFilePath;
}


QString VirtuozzoDirs::getVmAppPath(bool bX64)
{
	QString strVmExecutableDir = UTF8_2QSTR(getenv(PVS_VM_EXECUTABLE_ENV));

	// If environment variable PVS_VM_EXECUTABLE_ENV is not set,
	// we set full VM Controller path as: [current executable dir] + [VM_EXECUTABLE]
	if (strVmExecutableDir.isEmpty())
	{
		strVmExecutableDir = (QCoreApplication::instance())->applicationDirPath();
	}

	return strVmExecutableDir + (bX64 ? VM_EXECUTABLE64 : VM_EXECUTABLE);
}

QString VirtuozzoDirs::getVmStarterPath()
{
	QString strVmExecutableDir;

	strVmExecutableDir = (QCoreApplication::instance())->applicationDirPath();

	return strVmExecutableDir + VM_STARTER_EXECUTABLE;
}


QString VirtuozzoDirs::getConvertToolPath( const QDir& baseDir )
{
	QString sConvertToolPath = CONVERT_TOOL_EXECUTABLE;
	if ( !QFile::exists( sConvertToolPath ) )//Might we have a deal with developer's build?
		sConvertToolPath = QFileInfo(sConvertToolPath).fileName();
	return baseDir.absoluteFilePath( sConvertToolPath );
}

QString VirtuozzoDirs::getDiskToolPath( const QDir& baseDir )
{
	QString sDiskToolPath = DISK_TOOL_EXECUTABLE;
	if ( !QFile::exists( sDiskToolPath ) )//Might we have a deal with developer's build?
		sDiskToolPath = QFileInfo(sDiskToolPath).fileName();
	return baseDir.absoluteFilePath( sDiskToolPath );
}

QString VirtuozzoDirs::getVmScriptsDir(const QString &sBaseDir)
{
	PRL_ASSERT( !sBaseDir.isEmpty() );
	if( sBaseDir.isEmpty() )
		return "";
	return sBaseDir + "/scripts";
}

QString VirtuozzoDirs::getVmActionScriptPath(const QString &sBaseDir, PRL_VM_ACTION nAction)
{
	PRL_ASSERT( !sBaseDir.isEmpty() );
	if( sBaseDir.isEmpty() )
		return "";

	switch (nAction)
	{
	case PVA_PRESTART:
		return getVmScriptsDir(sBaseDir) + "/prestart";
	case PVA_POSTSTART:
		return getVmScriptsDir(sBaseDir) + "/poststart";
	case PVA_PRESTOP:
		return getVmScriptsDir(sBaseDir) + "/prestop";
	case PVA_POSTSTOP:
		return getVmScriptsDir(sBaseDir) + "/poststop";
	}
	return QString();
}

QString VirtuozzoDirs::getVmConfigurationSamplePath(const QString &sName)
{
	QString sPath = QString( "%1/samples/%2.pvs" ).
		arg( getDispatcherConfigDir() ).
		arg( sName );

	return sPath;
}

QString VirtuozzoDirs::getServiceAppName()
{
	return DISPATCHER_SERVICE_COMMON_NAME;
}

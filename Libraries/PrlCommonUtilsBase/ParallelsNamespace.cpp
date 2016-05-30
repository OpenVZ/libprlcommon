/*
 * ParallelsNamespace.cpp
 *
 * Copyright (C) 1999-2014 Parallels IP Holdings GmbH
 *
 * This file is part of Parallels SDK. Parallels SDK is free
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
 * Our contact details: Parallels IP Holdings GmbH, Vordergasse 59, 8200
 * Schaffhausen, Switzerland.
 */

#include <Interfaces/ParallelsNamespace.h>

const char* PVE::DispatcherCommandToString( unsigned int cmd )
{
	#define CASE_DISPATCHER_COMMAND( c ) \
		case ((unsigned int) c) : return #c;

	switch ( (unsigned int) cmd )
	{
		CASE_DISPATCHER_COMMAND( DspIllegalCommand )
		CASE_DISPATCHER_COMMAND( DspCmdVmStart )
		CASE_DISPATCHER_COMMAND( DspCmdVmStop )
		CASE_DISPATCHER_COMMAND( DspCmdVmSetConfig )
		CASE_DISPATCHER_COMMAND( DspCmdVmGetConfig )
		CASE_DISPATCHER_COMMAND( DspCmdVmGetProblemReport )
		CASE_DISPATCHER_COMMAND( DspCmdVmGetPackedProblemReport )
		CASE_DISPATCHER_COMMAND( DspCmdVmReset )
		CASE_DISPATCHER_COMMAND( DspCmdVmPause )
		CASE_DISPATCHER_COMMAND( DspCmdVmSuspend )
		CASE_DISPATCHER_COMMAND( DspCmdVmResume )
		CASE_DISPATCHER_COMMAND( DspCmdVmDropSuspendedState )
		CASE_DISPATCHER_COMMAND( DspCmdVmCreateSnapshot )
		CASE_DISPATCHER_COMMAND( DspCmdVmSwitchToSnapshot )
		CASE_DISPATCHER_COMMAND( DspCmdVmGetState )
		CASE_DISPATCHER_COMMAND( DspCmdVmDeleteSnapshot )
		CASE_DISPATCHER_COMMAND( DspCmdVmGetSnapshotsTree )
		CASE_DISPATCHER_COMMAND( DspCmdVmUpdateSnapshotData )
		CASE_DISPATCHER_COMMAND( DspCmdVmDevConnect )
		CASE_DISPATCHER_COMMAND( DspCmdVmDevGetState )
		CASE_DISPATCHER_COMMAND( DspCmdVmDevDisconnect )
		CASE_DISPATCHER_COMMAND( DspCmdVmDevChangeMedia )
		CASE_DISPATCHER_COMMAND( DspCmdVmDevHdCheckPassword )
		CASE_DISPATCHER_COMMAND( DspCmdVmTaskRun )
		CASE_DISPATCHER_COMMAND( DspCmdVmTaskGetState )
		CASE_DISPATCHER_COMMAND( DspCmdVmTaskCancel )
		CASE_DISPATCHER_COMMAND( DspCmdVmAnswer )
		CASE_DISPATCHER_COMMAND( DspCmdVmGetLogData )
		CASE_DISPATCHER_COMMAND( DspCmdVmGetMonitorState )
		CASE_DISPATCHER_COMMAND( DspCmdVmInstallTools )
		CASE_DISPATCHER_COMMAND( DspCmdVmMigrateCancel )
		CASE_DISPATCHER_COMMAND( DspCmdGetBackupTree )
		CASE_DISPATCHER_COMMAND( DspCmdCreateVmBackup )
		CASE_DISPATCHER_COMMAND( DspCmdRestoreVmBackup )
		CASE_DISPATCHER_COMMAND( DspCmdRemoveVmBackup )
		CASE_DISPATCHER_COMMAND( DspCmdDirVmCreate )
		CASE_DISPATCHER_COMMAND( DspCmdDirVmDelete )
		CASE_DISPATCHER_COMMAND( DspCmdDirVmClone )
		CASE_DISPATCHER_COMMAND( DspCmdDirVmCloneLinked )
		CASE_DISPATCHER_COMMAND( DspCmdDirVmMigrate )
		CASE_DISPATCHER_COMMAND( DspCmdDirVmMigrateClone )
		CASE_DISPATCHER_COMMAND( DspCmdUpdateDeviceInfo )
		CASE_DISPATCHER_COMMAND( DspCmdAttachToLostTask )
		CASE_DISPATCHER_COMMAND( DspCmdDirVmEditBegin )
		CASE_DISPATCHER_COMMAND( DspCmdDirVmEditCommit )
		CASE_DISPATCHER_COMMAND( DspCmdDirGetVmList )
		CASE_DISPATCHER_COMMAND( DspCmdDirGetVmConfig )
		CASE_DISPATCHER_COMMAND( DspCmdDirSetVmConfig )
		CASE_DISPATCHER_COMMAND( DspCmdDirCreateImage )
		CASE_DISPATCHER_COMMAND( DspCmdDirCopyImage )
		CASE_DISPATCHER_COMMAND( DspCmdDirRegVm )
		CASE_DISPATCHER_COMMAND( DspCmdDirUnregVm )
		CASE_DISPATCHER_COMMAND( DspCmdDirLockVm )
		CASE_DISPATCHER_COMMAND( DspCmdDirUnlockVm )
		CASE_DISPATCHER_COMMAND( DspCmdDirVerifyVmConfig )
		CASE_DISPATCHER_COMMAND( DspCmdStartSearchConfig )
		CASE_DISPATCHER_COMMAND( DspCmdConvertOldHdd )
		CASE_DISPATCHER_COMMAND( DspCmdUserLogin )
		CASE_DISPATCHER_COMMAND( DspCmdUserLoginLocal )
		CASE_DISPATCHER_COMMAND( DspCmdUserLoginLocalStage2 )
		CASE_DISPATCHER_COMMAND( DspCmdUserLogoff )
		CASE_DISPATCHER_COMMAND( DspCmdUserAttachEventNotification )
		CASE_DISPATCHER_COMMAND( DspCmdUserGetEvent )
		CASE_DISPATCHER_COMMAND( DspCmdUserGetProfile )
		CASE_DISPATCHER_COMMAND( DspCmdUserProfileBeginEdit )
		CASE_DISPATCHER_COMMAND( DspCmdUserProfileCommit )
		CASE_DISPATCHER_COMMAND( DspCmdGetHostCommonInfo )
		CASE_DISPATCHER_COMMAND( DspCmdHostCommonInfoBeginEdit )
		CASE_DISPATCHER_COMMAND( DspCmdHostCommonInfoCommit )
		CASE_DISPATCHER_COMMAND( DspCmdUserGetHostHwInfo )
		CASE_DISPATCHER_COMMAND( DspCmdUserPing )
		CASE_DISPATCHER_COMMAND( DspCmdUserCancelOperation )
		CASE_DISPATCHER_COMMAND( DspCmdFsGetDiskList )
		CASE_DISPATCHER_COMMAND( DspCmdFsGetCurrentDirectory )
		CASE_DISPATCHER_COMMAND( DspCmdFsGetDirectoryEntries )
		CASE_DISPATCHER_COMMAND( DspCmdFsGetFileList )
		CASE_DISPATCHER_COMMAND( DspCmdFsCreateDirectory )
		CASE_DISPATCHER_COMMAND( DspCmdFsRenameEntry )
		CASE_DISPATCHER_COMMAND( DspCmdFsRemoveEntry )
		CASE_DISPATCHER_COMMAND( DspCmdFsCanCreateFile )
		CASE_DISPATCHER_COMMAND( DspCmdFsGenerateEntryName )
		CASE_DISPATCHER_COMMAND( DspCmdDirInstallGuestOS )
		CASE_DISPATCHER_COMMAND( DspCmdGetVmInfo )
		CASE_DISPATCHER_COMMAND( DspCmdGetVmToolsInfo )
		CASE_DISPATCHER_COMMAND( DspCmdSMCGetDispatcherRTInfo )
		CASE_DISPATCHER_COMMAND( DspCmdSMCGetCommandHistoryByVm )
		CASE_DISPATCHER_COMMAND( DspCmdSMCGetCommandHistoryByUser )
		CASE_DISPATCHER_COMMAND( DspCmdSMCShutdownDispatcher )
		CASE_DISPATCHER_COMMAND( DspCmdSMCRestartDispatcher )
		CASE_DISPATCHER_COMMAND( DspCmdSMCDisconnectUser )
		CASE_DISPATCHER_COMMAND( DspCmdSMCDisconnectAllUsers )
		CASE_DISPATCHER_COMMAND( DspCmdSMCCancelUserCommand )
		CASE_DISPATCHER_COMMAND( DspCmdSMCShutdownVm )
		CASE_DISPATCHER_COMMAND( DspCmdSMCRestartVm )
		CASE_DISPATCHER_COMMAND( DspCmdSMCShutdownDispatcherInternal )
		CASE_DISPATCHER_COMMAND( DspCmdNetPrlNetworkServiceStart )
		CASE_DISPATCHER_COMMAND( DspCmdNetPrlNetworkServiceStop )
		CASE_DISPATCHER_COMMAND( DspCmdNetPrlNetworkServiceRestart )
		CASE_DISPATCHER_COMMAND( DspCmdNetPrlNetworkServiceRestoreDefaults )
		CASE_DISPATCHER_COMMAND( DspCmdGetNetServiceStatus )
		CASE_DISPATCHER_COMMAND( DspCmdAddNetAdapter )
		CASE_DISPATCHER_COMMAND( DspCmdDeleteNetAdapter )
		CASE_DISPATCHER_COMMAND( DspCmdUpdateNetAdapter )
		CASE_DISPATCHER_COMMAND( DspCmdGetHostStatistics )
		CASE_DISPATCHER_COMMAND( DspCmdVmGetStatistics )
		CASE_DISPATCHER_COMMAND( DspCmdUserUpdateLicense )
		CASE_DISPATCHER_COMMAND( DspCmdUserGetLicenseInfo )
		CASE_DISPATCHER_COMMAND( DspCmdFileTransferLogin )
		CASE_DISPATCHER_COMMAND( DspCmdFileTransferLoginResponse )
		CASE_DISPATCHER_COMMAND( DspCmdFileTransferUpload )
		CASE_DISPATCHER_COMMAND( DspCmdFileTransferUploadResponse )
		CASE_DISPATCHER_COMMAND( DspVmRequest )
		CASE_DISPATCHER_COMMAND( DspVmResponse )
		CASE_DISPATCHER_COMMAND( DspVmEvent )
		CASE_DISPATCHER_COMMAND( DspVmEventStartVNCServer )
		CASE_DISPATCHER_COMMAND( DspVmEventStopVNCServer )
		CASE_DISPATCHER_COMMAND( DspVmBinaryEvent )
		CASE_DISPATCHER_COMMAND( DspVmBinaryResponse )
		CASE_DISPATCHER_COMMAND( DspVmAnswer )
		CASE_DISPATCHER_COMMAND( DspVmGetHostHwInfo )
		CASE_DISPATCHER_COMMAND( DspVmRetHostHwInfo )
		CASE_DISPATCHER_COMMAND( DspVmSendEchoEvent )
		CASE_DISPATCHER_COMMAND( DspEvtNotifyVm )
		CASE_DISPATCHER_COMMAND( DspEvtHwChanged )
		CASE_DISPATCHER_COMMAND( DspVmAuth )
		CASE_DISPATCHER_COMMAND( DspVmGetHardDiskStates )
		CASE_DISPATCHER_COMMAND( DspVmRetHardDiskStates )
		CASE_DISPATCHER_COMMAND( DspCmdCtlGetStatus )
		CASE_DISPATCHER_COMMAND( DspCmdCtlProcessDeferredTaskFinish )
		CASE_DISPATCHER_COMMAND( DspCmdCtlApplyVmConfig )
		CASE_DISPATCHER_COMMAND( DspCmdCtlVmEditWithRename )
		CASE_DISPATCHER_COMMAND( DspCtlCommandRangeEnd )
		CASE_DISPATCHER_COMMAND( DspWsResponse )
		CASE_DISPATCHER_COMMAND( DspWsBinaryResponse )
		CASE_DISPATCHER_COMMAND( DspReplyWsStatus )
		CASE_DISPATCHER_COMMAND( DspWsCommandRangeEnd )
		CASE_DISPATCHER_COMMAND( DspCmdVmCreateUnattendedFloppy )
		CASE_DISPATCHER_COMMAND( DspCmdSubscribeToHostStatistics )
		CASE_DISPATCHER_COMMAND( DspCmdUnsubscribeFromHostStatistics )
		CASE_DISPATCHER_COMMAND( DspCmdVmSubscribeToGuestStatistics )
		CASE_DISPATCHER_COMMAND( DspCmdVmUnsubscribeFromGuestStatistics )
		CASE_DISPATCHER_COMMAND( DspCmdVmInitiateDevStateNotifications )
		CASE_DISPATCHER_COMMAND( DspCmdLookupParallelsServers )
		CASE_DISPATCHER_COMMAND( DspCmdVmUpdateSecurity )
		CASE_DISPATCHER_COMMAND( DspCmdVmSectionValidateConfig )
		CASE_DISPATCHER_COMMAND( DspCmdVmCompact )
		CASE_DISPATCHER_COMMAND( DspCmdVmCancelCompact )
		CASE_DISPATCHER_COMMAND( DspCmdVmSuspendCancel )
		CASE_DISPATCHER_COMMAND( DspCmdVmUpdateToolsSection )
		CASE_DISPATCHER_COMMAND( DspCmdUserInfoList )
		CASE_DISPATCHER_COMMAND( DspCmdUserInfo )
		CASE_DISPATCHER_COMMAND( DspCmdAllHostUsers )
		CASE_DISPATCHER_COMMAND( DspCmdPerfomanceStatistics )
		CASE_DISPATCHER_COMMAND( DspCmdVmGetSuspendedScreen )
		CASE_DISPATCHER_COMMAND( DspCmdVmRunCompressor )
		CASE_DISPATCHER_COMMAND( DspCmdVmCancelCompressor )
		CASE_DISPATCHER_COMMAND( DspCmdVmFinishCompressorInternal )
		CASE_DISPATCHER_COMMAND( DspCmdVmStartEx )
		CASE_DISPATCHER_COMMAND( DspCmdVmRestartGuest )
		CASE_DISPATCHER_COMMAND( DspCmdVmInstallUtility )
		CASE_DISPATCHER_COMMAND( DspCmdDirRestoreVm )
		CASE_DISPATCHER_COMMAND( DspCmdVmLoginInGuest )
		CASE_DISPATCHER_COMMAND( DspCmdVmGuestLogout )
		CASE_DISPATCHER_COMMAND( DspCmdVmGuestRunProgram )
		CASE_DISPATCHER_COMMAND( DspCmdVmStartVNCServer )
		CASE_DISPATCHER_COMMAND( DspCmdVmStopVNCServer )
		CASE_DISPATCHER_COMMAND( DspCmdVmGuestGetNetworkSettings )
		CASE_DISPATCHER_COMMAND( DspCmdGetVirtualNetworkList )
		CASE_DISPATCHER_COMMAND( DspCmdVmAuthWithGuestSecurityDb )
		CASE_DISPATCHER_COMMAND( DspCmdVmGuestSetUserPasswd )
		CASE_DISPATCHER_COMMAND( DspCmdAddVirtualNetwork )
		CASE_DISPATCHER_COMMAND( DspCmdUpdateVirtualNetwork )
		CASE_DISPATCHER_COMMAND( DspCmdDeleteVirtualNetwork )
		CASE_DISPATCHER_COMMAND( DspCmdConfigureGenericPci )
		CASE_DISPATCHER_COMMAND( DspCmdPrepareForHibernate )
		CASE_DISPATCHER_COMMAND( DspCmdAfterHostResume )
		CASE_DISPATCHER_COMMAND( DspCmdCtlVmStandByGuest )
		CASE_DISPATCHER_COMMAND( DspCmdSendProblemReport )
		CASE_DISPATCHER_COMMAND( DspCmdVmLock )
		CASE_DISPATCHER_COMMAND( DspCmdVmUnlock )
		CASE_DISPATCHER_COMMAND( DspCmdSetNonInteractiveSession )
		CASE_DISPATCHER_COMMAND( DspCmdVmResizeDisk )
		CASE_DISPATCHER_COMMAND( DspCmdSetSessionConfirmationMode )
		CASE_DISPATCHER_COMMAND( DspCmdStorageSetValue )
		CASE_DISPATCHER_COMMAND( DspCmdVmStorageSetValue )
		CASE_DISPATCHER_COMMAND( DspCmdSendClientStatistics )
		CASE_DISPATCHER_COMMAND( DspCmdVmChangeLogLevel )
		CASE_DISPATCHER_COMMAND( DspCmdDirReg3rdPartyVm )
		CASE_DISPATCHER_COMMAND( DspVmDevConnect )
		CASE_DISPATCHER_COMMAND( DspVmDevDisconnect )
		CASE_DISPATCHER_COMMAND( DspCmdVmChangeSid )
		CASE_DISPATCHER_COMMAND( DspEvtNetworkPrefChanged )
		CASE_DISPATCHER_COMMAND( DspCmdInstallAppliance )
		CASE_DISPATCHER_COMMAND( DspCmdVmResetUptime )
		CASE_DISPATCHER_COMMAND( DspCmdStartClusterService )
		CASE_DISPATCHER_COMMAND( DspCmdStopClusterService )
		CASE_DISPATCHER_COMMAND( DspCmdVmInternal )
		CASE_DISPATCHER_COMMAND( DspCmdGetVmVirtDevInfo )
		CASE_DISPATCHER_COMMAND( DspCmdVmConvertDisks )
		CASE_DISPATCHER_COMMAND( DspCmdVmAuthorise )
		CASE_DISPATCHER_COMMAND( DspCmdVmChangePassword )
		CASE_DISPATCHER_COMMAND( DspCmdVmEncrypt )
		CASE_DISPATCHER_COMMAND( DspCmdVmDecrypt )
		CASE_DISPATCHER_COMMAND( DspCmdUpdateNetworkClassesConfig)
		CASE_DISPATCHER_COMMAND( DspCmdGetNetworkClassesConfig )
		CASE_DISPATCHER_COMMAND( DspCmdUpdateNetworkShapingConfig )
		CASE_DISPATCHER_COMMAND( DspCmdGetNetworkShapingConfig )
		CASE_DISPATCHER_COMMAND( DspCmdRestartNetworkShaping )
		CASE_DISPATCHER_COMMAND( DspCmdRegisterIscsiStorage )
		CASE_DISPATCHER_COMMAND( DspCmdUnregisterIscsiStorage )
		CASE_DISPATCHER_COMMAND( DspCmdExtendIscsiStorage )
		CASE_DISPATCHER_COMMAND( DspCmdGetCtTemplateList )
		CASE_DISPATCHER_COMMAND( DspCmdGetDefaultVmConfig )
		CASE_DISPATCHER_COMMAND( DspVmRestoreState )
		CASE_DISPATCHER_COMMAND( DspVmStateInfo )
		CASE_DISPATCHER_COMMAND( DspCmdRemoveCtTemplate )
		CASE_DISPATCHER_COMMAND( DspCmdCopyCtTemplate )
		CASE_DISPATCHER_COMMAND( DspCmdAddIPPrivateNetwork )
		CASE_DISPATCHER_COMMAND( DspCmdRemoveIPPrivateNetwork )
		CASE_DISPATCHER_COMMAND( DspCmdUpdateIPPrivateNetwork )
		CASE_DISPATCHER_COMMAND( DspCmdGetIPPrivateNetworksList )
		CASE_DISPATCHER_COMMAND( DspCmdRefreshPlugins )
		CASE_DISPATCHER_COMMAND( DspCmdVmMount )
		CASE_DISPATCHER_COMMAND( DspCmdVmUmount )
		CASE_DISPATCHER_COMMAND( DspCmdGetPluginsList )
		CASE_DISPATCHER_COMMAND( DspCmdCtlLicenseChange )
		CASE_DISPATCHER_COMMAND( DspCmdDirVmMove )
		CASE_DISPATCHER_COMMAND( DspCmdGetVmConfigById )
		CASE_DISPATCHER_COMMAND( DspCmdCtlVmCommitDiskUnfinished )
		CASE_DISPATCHER_COMMAND( DspCmdVmSetProtection )
		CASE_DISPATCHER_COMMAND( DspCmdVmRemoveProtection )
		CASE_DISPATCHER_COMMAND( DspCmdCtlVmCollectGuestUsage )
		CASE_DISPATCHER_COMMAND( DspCmdGetCPUPoolsList )
		CASE_DISPATCHER_COMMAND( DspCmdMoveToCPUPool )
		CASE_DISPATCHER_COMMAND( DspCmdRecalculateCPUPool )
		CASE_DISPATCHER_COMMAND( DspCmdJoinCPUPool )
		CASE_DISPATCHER_COMMAND( DspCmdLeaveCPUPool )
	}
	return "Unknown";
}

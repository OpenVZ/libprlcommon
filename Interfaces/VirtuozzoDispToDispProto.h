/*
 * VirtuozzoDispToDispProto.h: Implementation of Dispatcher-Dispatcher
 * common protocol commands serializer helpers.
 *
 * Copyright (C) 1999-2016 Parallels IP Holdings GmbH
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

#ifndef VirtuozzoDispToDispProto_H
#define VirtuozzoDispToDispProto_H

#include "VirtuozzoNamespace.h"

namespace Virtuozzo
{

/**
 * Dispatcher-dispatcher common proto commands identifiers set
 */
enum IDispToDispCommands
{
	DispToDispUnknownCmd = 0,

	DispToDispAuthorizeCmd = PVE::DispToDispRangeStart + 1,
	DispToDispLogoffCmd    = PVE::DispToDispRangeStart + 2,
	DispToDispResponseCmd  = PVE::DispToDispRangeStart + 3,

	/* Generic file transfer protocol */
	FileCopyRangeStart		= PVE::DispToDispRangeStart + 400,
	FileCopyFirstRequest		= FileCopyRangeStart + 1,
	FileCopyFirstReply		= FileCopyRangeStart + 2,
	FileCopyCancelCmd		= FileCopyRangeStart + 3,
	FileCopyFileReplyCmd  		= FileCopyRangeStart + 4,
	FileCopyDirCmd			= FileCopyRangeStart + 5,
	FileCopyFileCmd			= FileCopyRangeStart + 6,
	FileCopyFileChunkCmd		= FileCopyRangeStart + 7,
	FileCopyFinishCmd		= FileCopyRangeStart + 8,
	FileCopyReply	  		= FileCopyRangeStart + 9,
	FileCopyError	  		= FileCopyRangeStart + 10,
	FileCopyRangeEnd		= PVE::DispToDispRangeStart + 499,

	VmMigrateRangeStart		= PVE::DispToDispRangeStart + 500,
	VmMigrateCheckPreconditionsCmd	= PVE::DispToDispRangeStart + 501,
	VmMigrateStartCmd		= PVE::DispToDispRangeStart + 502,
	VmMigrateCancelCmd		= PVE::DispToDispRangeStart + 503,
	VmMigrateReply			= PVE::DispToDispRangeStart + 504,
	VmMigrateCheckPreconditionsReply	= PVE::DispToDispRangeStart + 505,
	VmMigrateMemPageCmd		= PVE::DispToDispRangeStart + 506,
	VmMigrateFinishCmd		= PVE::DispToDispRangeStart + 507,
	VmMigrateDiskBlockCmd		= PVE::DispToDispRangeStart + 508,
	VmMigrateVideoMemViewCmd	= PVE::DispToDispRangeStart + 509,

	VmMigrateFreeGuestCmd		= PVE::DispToDispRangeStart + 510,
	VmMigrateFillDiskMapCmd		= PVE::DispToDispRangeStart + 511,
	VmMigrateMakeDiskMapCmd		= PVE::DispToDispRangeStart + 512,

	VmMigrateTunnelRangeStart	= PVE::DispToDispRangeStart + 513,
	VmMigrateLibvirtTunnelChunk     = VmMigrateTunnelRangeStart + 0,
	VmMigrateQemuStateTunnelChunk   = VmMigrateTunnelRangeStart + 1,
	VmMigrateConnectQemuStateCmd    = VmMigrateTunnelRangeStart + 2,
	VmMigrateQemuDiskTunnelChunk    = VmMigrateTunnelRangeStart + 3,
	VmMigrateConnectQemuDiskCmd     = VmMigrateTunnelRangeStart + 4,
	VmMigrateTunnelRangeEnd		= VmMigrateConnectQemuDiskCmd,

	VmMigrateRangeEnd		= PVE::DispToDispRangeStart + 599,

	/* Backup proto commands */
	VmBackupGetTreeCmd		= PVE::DispToDispRangeStart + 601,
	VmBackupCreateCmd		= PVE::DispToDispRangeStart + 602,
	VmBackupRestoreCmd		= PVE::DispToDispRangeStart + 603,
	VmBackupRestoreFirstReply	= PVE::DispToDispRangeStart + 604,
	VmBackupRemoveCmd		= PVE::DispToDispRangeStart + 605,
	VmBackupGetTreeReply		= PVE::DispToDispRangeStart + 606,
	VmBackupCreateFirstReply	= PVE::DispToDispRangeStart + 607,
	VmBackupCreateLocalCmd		= PVE::DispToDispRangeStart + 608,
	VmBackupAttachCmd			= PVE::DispToDispRangeStart + 609,
	VmBackupConnectSourceCmd	= PVE::DispToDispRangeStart + 610,
	VmBackupMountImage		= PVE::DispToDispRangeStart + 611,
	VmBackupRestoreImage		= PVE::DispToDispRangeStart + 612,

	ABackupProxyRangeStart			= PVE::DispToDispRangeStart + 700,
	ABackupProxyOpenDirOfFileRequest	= ABackupProxyRangeStart + 1,
	ABackupProxyOpenRequest			= ABackupProxyRangeStart + 2,
	ABackupProxyOpenResponse		= ABackupProxyRangeStart + 3,
	ABackupProxyReadRequest			= ABackupProxyRangeStart + 4,
	ABackupProxyReadResponse		= ABackupProxyRangeStart + 5,
	ABackupProxyWriteRequest		= ABackupProxyRangeStart + 6,
	ABackupProxyWriteResponse		= ABackupProxyRangeStart + 7,
	ABackupProxyFlushRequest		= ABackupProxyRangeStart + 8,
	ABackupProxyRewindRequest		= ABackupProxyRangeStart + 9,
	ABackupProxyTruncateRequest		= ABackupProxyRangeStart + 10,
	ABackupProxySizeRequest			= ABackupProxyRangeStart + 11,
	ABackupProxySizeResponse		= ABackupProxyRangeStart + 12,
	ABackupProxySeekRequest			= ABackupProxyRangeStart + 13,
	ABackupProxySeekResponse		= ABackupProxyRangeStart + 14,
	ABackupProxySeekRelRequest		= ABackupProxyRangeStart + 15,
	ABackupProxySeekRelResponse		= ABackupProxyRangeStart + 16,
	ABackupProxySeekEndRequest		= ABackupProxyRangeStart + 17,
	ABackupProxySeekEndResponse		= ABackupProxyRangeStart + 18,
	ABackupProxyWritePaddingIOCTLRequest	= ABackupProxyRangeStart + 19,
	ABackupProxyWritePaddingIOCTLResponse	= ABackupProxyRangeStart + 20,
	ABackupProxyDismountFsIOCTLRequest	= ABackupProxyRangeStart + 21,
	ABackupProxyGetVolumeBoundsIOCTLRequest	= ABackupProxyRangeStart + 22,
	ABackupProxyGetVolumeBoundsIOCTLResponse= ABackupProxyRangeStart + 23,
	ABackupProxyPreloadRegionIOCTLRequest	= ABackupProxyRangeStart + 24,
	ABackupProxyPreloadRegionIOCTLResponse	= ABackupProxyRangeStart + 25,
	ABackupProxyRenameRequest		= ABackupProxyRangeStart + 26,
	ABackupProxyGetSectorSizeRequest	= ABackupProxyRangeStart + 27,
	ABackupProxyGetSectorSizeResponse	= ABackupProxyRangeStart + 28,
	ABackupProxyResponse			= ABackupProxyRangeStart + 29,
	ABackupProxyError			= ABackupProxyRangeStart + 30,
	ABackupProxyFinishCmd			= ABackupProxyRangeStart + 31,
	ABackupProxyCancelCmd			= ABackupProxyRangeStart + 32,
	ABackupProxyProgress			= ABackupProxyRangeStart + 33,
	ABackupProxyGoodBye			= ABackupProxyRangeStart + 34,
	ABackupProxyCloseRequest		= ABackupProxyRangeStart + 35,
	ABackupQueryArchiveReply		= ABackupProxyRangeStart + 36,
	ABackupProxyRangeEnd			= PVE::DispToDispRangeStart + 799,

	CtMigrateCmd			= PVE::DispToDispRangeStart + 800,
	CopyCtTemplateCmd		= PVE::DispToDispRangeStart + 801,
	CopyCtTemplateReply		= PVE::DispToDispRangeStart + 802,
};

#define	IS_FILE_COPY_PACKAGE(type) (((type) > FileCopyRangeStart) && ((type) < FileCopyRangeEnd))
#define	IS_ABACKUP_PROXY_PACKAGE(type) (((type) > ABackupProxyRangeStart) && ((type) < ABackupProxyRangeEnd))

}//namespace Virtuozzo

#endif

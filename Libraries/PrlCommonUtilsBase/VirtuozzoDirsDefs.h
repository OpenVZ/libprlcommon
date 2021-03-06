/*
 * VirtuozzoDirsDefs.h: Virtuozzo Dirs defines.
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


#ifndef __VIRTUOZZO_DIRS_DEFS_H__
#define __VIRTUOZZO_DIRS_DEFS_H__

// FIXME This path must be used on Mac only
#define PRL_DIRS_BC_BACKUP_DIR "/Library/Virtuozzo/.bc_backup"

#define PRL_DIRS_INSTALLED_EULA_PATH "/Library/Preferences/Virtuozzo"

#define PRL_DIRS_MAC_OS_INSTALLATION_FILE "/Library/Receipts/BSD.pkg"
#define PRL_DIRS_PD_INSTALLATION_STATS_FOLDER "/var/db/Virtuozzo/Stats"

#ifdef _WIN_
	#define VM_EXECUTABLE	"/prl_vm_app.exe"
	#define VM_EXECUTABLE64	"/amd64/prl_vm_app.exe"
#else
	#define VM_EXECUTABLE	"/prl_vm_app"
	#define VM_EXECUTABLE64	"/prl_vm_app64"
#endif

#ifdef _WIN_
	#define ARCH_7_ZIP_EXECUTABLE	"7z.exe"
#elif defined (_LIN_)
	#define ARCH_7_ZIP_EXECUTABLE	"/usr/bin/prl_7z"
#else
	#define ARCH_7_ZIP_EXECUTABLE			"/7z"
	#define ARCH_7_ZIP_EXECUTABLE_ZBUILD	"../../../7z"
#endif

#ifdef _WIN_
	#define MKISO_EXECUTABLE		"prl_mkiso.exe"
#elif defined (_LIN_)
	#define MKISO_EXECUTABLE		"/usr/bin/prl_mkiso"
#else
	#define MKISO_EXECUTABLE		"/prl_mkiso"
#endif


#ifdef _WIN_
	#define VM_STARTER_EXECUTABLE	""
#else
	#define VM_STARTER_EXECUTABLE	"/prl_vm_starter"
#endif

#ifdef _WIN_
	#define DISK_TOOL_EXECUTABLE	"prl_disk_tool.exe"
#else
	#define DISK_TOOL_EXECUTABLE	"/usr/sbin/prl_disk_tool"
#endif

#ifdef _WIN_
	#define CONVERT_TOOL_EXECUTABLE	"prl_convert.exe"
#elif defined _LIN_
	#define CONVERT_TOOL_EXECUTABLE	"/usr/sbin/prl_convert"
#endif

#ifdef _LIN_
	#define UNRAR_EXECUTABLE	"/usr/bin/unrar"
#else
	#define UNRAR_EXECUTABLE	"unrar"
#endif

#ifdef _WIN_
#define PRLPROCDUMP_EXECUTABLE		"prl_procdump.exe"
#define PRLPROCDUMP_EXECUTABLE64	"amd64/prl_procdump.exe"
#endif

#endif // __VIRTUOZZO_DIRS_DEFS_H__


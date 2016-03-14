///////////////////////////////////////////////////////////////////////////////
///
/// @file Util.h
///
/// Utility functions/classes for VirtualDisk operations.
///
/// @author mperevedentsev
///
/// Copyright (c) 2016 Parallels IP Holdings GmbH
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
/// Our contact details: Parallels IP Holdings GmbH, Vordergasse 59, 8200
/// Schaffhausen, Switzerland.
///
///////////////////////////////////////////////////////////////////////////////
#ifndef __VIRTUAL_DISK_UTIL__
#define __VIRTUAL_DISK_UTIL__

#include <QString>
#include <prlsdk/PrlTypes.h>

namespace VirtualDisk
{
namespace IO
{

///////////////////////////////////////////////////////////////////////////////
// File

struct File
{
	explicit File(const QString &name):
		m_name(name), m_fd(-1)
	{
	}

	PRL_RESULT open(int flags);
	// size and offset are in BYTES
	PRL_RESULT pread(void *data, PRL_UINT64 size, PRL_UINT64 offset) const;
	PRL_RESULT pwrite(const void *data, PRL_UINT64 size, PRL_UINT64 offset) const;
	PRL_RESULT ioctl(int request, void *data) const;
	PRL_RESULT close();

private:
	QString m_name;
	int m_fd;
};

} // namespace IO
} // namespace VirtualDisk

#endif // __VIRTUAL_DISK_UTIL__

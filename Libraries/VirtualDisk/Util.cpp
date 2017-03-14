///////////////////////////////////////////////////////////////////////////////
///
/// @file Util.cpp
///
/// Utility functions/classes for VirtualDisk operations.
///
/// @author mperevedentsev
///
/// Copyright (c) 2016-2017, Parallels International GmbH
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
/// Our contact details: Parallels International GmbH, Vordergasse 59, 8200
/// Schaffhausen, Switzerland.
///
///////////////////////////////////////////////////////////////////////////////
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <QFileInfo>

#include <prlsdk/PrlErrors.h>
#include <prlsdk/PrlErrorsValues.h>

#include "../Logging/Logging.h"

#include "Util.h"

namespace VirtualDisk
{
namespace IO
{

///////////////////////////////////////////////////////////////////////////////
// File

PRL_RESULT File::open(const QString &fileName, int flags)
{
	if (!QFileInfo(fileName).exists())
	{
		WRITE_TRACE(DBG_FATAL, "File not found: '%s'", fileName.toUtf8().constData());
		return PRL_ERR_FILE_NOT_FOUND;
	}

	m_fd = ::open(fileName.toUtf8().constData(), flags);
	if (m_fd < 0)
	{
		WRITE_TRACE(DBG_FATAL, "Cannot open file '%s': %m",
		            fileName.toUtf8().constData());
		return PRL_ERR_DISK_FILE_OPEN_ERROR;
	}
	return PRL_ERR_SUCCESS;
}

PRL_RESULT File::pread(void *data, PRL_UINT64 size, PRL_UINT64 offset) const
{
	if (!data)
		return PRL_ERR_INVALID_ARG;

	if (m_fd < 0)
		return PRL_ERR_DISK_DISK_NOT_OPENED;

	size_t totalRead = 0;
	while (totalRead < size)
	{
		ssize_t currentRead = ::pread(m_fd,
		                              (char*)data + totalRead,
		                              size - totalRead,
		                              offset + totalRead);
		if (currentRead <= 0)
		{
			if (currentRead == 0)
				return PRL_ERR_DISK_READ_OUT_DISK;

			WRITE_TRACE(DBG_FATAL, "pread() failed: %m");
			return PRL_ERR_FILE_READ_ERROR;
		}

		totalRead += currentRead;
	}

	return PRL_ERR_SUCCESS;
}

PRL_RESULT File::pwrite(const void *data, PRL_UINT64 size, PRL_UINT64 offset) const
{
	if (!data)
		return PRL_ERR_INVALID_ARG;

	if (m_fd < 0)
		return PRL_ERR_DISK_DISK_NOT_OPENED;

	size_t totalWritten = 0;
	while (totalWritten < size)
	{
		ssize_t currentWritten = ::pwrite(m_fd,
		                                  (const char*)data + totalWritten,
		                                  size - totalWritten,
		                                  offset + totalWritten);
		if (currentWritten < 0)
		{
			WRITE_TRACE(DBG_FATAL, "pwrite() failed: %m");
			return PRL_ERR_FILE_WRITE_ERROR;
		}

		totalWritten += currentWritten;
	}

	return PRL_ERR_SUCCESS;
}

PRL_RESULT File::ioctl(int request, void *data) const
{
	if (::ioctl(m_fd, request, data))
	{
		WRITE_TRACE(DBG_FATAL, "ioctl() failed: %m");
		return PRL_ERR_DISK_GENERIC_ERROR;
	}
	return PRL_ERR_SUCCESS;
}

PRL_RESULT File::close()
{
	if (m_fd < 0)
		return PRL_ERR_SUCCESS;

	if (::close(m_fd))
	{
		WRITE_TRACE(DBG_FATAL, "close() failed: %m");
		return PRL_ERR_DISK_GENERIC_ERROR;
	}

	m_fd = -1;
	return PRL_ERR_SUCCESS;
}

} // namespace IO
} // namespace VirtualDisk

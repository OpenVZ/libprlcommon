/*
 * Copyright (c) 2016-2017, Parallels International GmbH
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
 * Our contact details: Parallels International GmbH, Vordergasse 59, 8200
 * Schaffhausen, Switzerland.
 */

#include "Libraries/Logging/Logging.h"
#include <Archive.h>
#include <archive_entry.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <QDir>

namespace Archive
{
///////////////////////////////////////////////////////////////////////////////
// struct Writer

Writer* Writer::create(const QString& path_)
{
	QScopedPointer<archive, Deleter> a(archive_write_new());

	archive_write_add_filter_gzip(a.data());
	archive_write_set_format_pax_restricted(a.data());
	archive_write_set_bytes_per_block(a.data(), 4096);
	if (archive_write_open_filename(a.data(), path_.toUtf8().data()) != ARCHIVE_OK)
	{
		WRITE_TRACE(DBG_FATAL, "unable to create archive file '%s'", path_.toUtf8().data());
		return NULL;
	}

	return new Writer(a.take());
}

PRL_RESULT Writer::append(const QString& file_, const QString& path_)
{
	archive* a = m_archive.data();
	char buff[8192];

	struct stat st;
	if (0 != stat(file_.toUtf8().data(), &st))
	{
		WRITE_TRACE(DBG_FATAL, "stat('%s') error (%d: '%s')",
			file_.toUtf8().data(), errno, strerror(errno));
		return PRL_ERR_FAILURE;
	}

	int fd = TEMP_FAILURE_RETRY(::open(file_.toUtf8().data(), O_RDONLY));
	if (fd < 0)
		return PRL_ERR_FAILURE;

	archive_entry* entry = archive_entry_new();
	archive_entry_copy_stat(entry, &st);
	archive_entry_set_pathname(entry, path_.toUtf8().data());
	archive_entry_set_perm(entry, 0644);
	archive_write_header(a, entry);

	int len = 0;
	while (0 < (len = TEMP_FAILURE_RETRY(::read(fd, buff, sizeof(buff)))))
	{
		if (len == archive_write_data(a, buff, len))
			continue;

		WRITE_TRACE(DBG_FATAL, "failed to write block to the archive");
		len = -1;
		break;
	}

	close(fd);
	archive_write_finish_entry(a);
	archive_entry_free(entry);
	return len < 0 ? PRL_ERR_FAILURE : PRL_ERR_SUCCESS;
}

void Writer::Deleter::cleanup(archive* ptr_)
{
	if (ptr_ == NULL)
		return;
	archive_write_close(ptr_);
	archive_write_free(ptr_);
}

///////////////////////////////////////////////////////////////////////////////
// struct Reader

int Reader::extractAll(const QString& archive_, const QString& path_)
{
	QScopedPointer<archive, Deleter> ra(archive_read_new());
	archive_read_support_filter_gzip(ra.data());
	archive_read_support_format_tar(ra.data());
	if (ARCHIVE_OK != archive_read_open_filename(ra.data(), archive_.toUtf8().data(), 10240))
	{
		WRITE_TRACE(DBG_FATAL, "unable to open archive '%s'", archive_.toUtf8().data());
		return PRL_ERR_FAILURE;
	}

	QScopedPointer<archive, Writer::Deleter> wa(archive_write_disk_new());

	int flags = ARCHIVE_EXTRACT_TIME | ARCHIVE_EXTRACT_PERM
		| ARCHIVE_EXTRACT_ACL | ARCHIVE_EXTRACT_FFLAGS;

	archive_write_disk_set_options(wa.data(), flags);
	archive_write_disk_set_standard_lookup(wa.data());

	int r = 0;
	archive_entry *entry = NULL;
	while (ARCHIVE_EOF != (r = archive_read_next_header(ra.data(), &entry)))
	{
		if (r < ARCHIVE_WARN)
		{
			WRITE_TRACE(DBG_DEBUG, "failed to read file in archive: %s\n",
				archive_error_string(ra.data()));
			return PRL_ERR_FAILURE;
		}

		archive_entry_set_pathname(entry, QDir(path_)
			.absoluteFilePath(archive_entry_pathname(entry)).toUtf8().data());

		if (archive_write_header(wa.data(), entry) < ARCHIVE_OK)
		{
			WRITE_TRACE(DBG_FATAL, "failed to create file: %s\n", archive_error_string(ra.data()));
			return PRL_ERR_FAILURE;
		}

		if ((archive_entry_size(entry) > 0) && PRL_FAILED(copy(ra.data(), wa.data())))
			return PRL_ERR_FAILURE;

		if (archive_write_finish_entry(wa.data()) < ARCHIVE_WARN)
		{
			WRITE_TRACE(DBG_FATAL, "failed to extract file from archive: %s\n",
				archive_error_string(wa.data()));
			return PRL_ERR_FAILURE;
		}
	}
	return PRL_ERR_SUCCESS;
}

int Reader::copy(archive* read_, archive* write_)
{

	int r = ARCHIVE_EOF;
	const void *buff = NULL;
	size_t size = 0;
	int64_t offset = 0;

	while (ARCHIVE_EOF != (r = archive_read_data_block(read_, &buff, &size, &offset)))
	{
		if (r != ARCHIVE_OK)
		{
			WRITE_TRACE(DBG_FATAL, "failed to read data from archive: %s",
				archive_error_string(read_));
			return PRL_ERR_FAILURE;
		}

		if (ARCHIVE_OK < archive_write_data_block(write_, buff, size, offset))
		{
			 WRITE_TRACE(DBG_FATAL, "failed to write data to archive: %s",
				archive_error_string(write_));
			return PRL_ERR_FAILURE;
		}
	}
	return PRL_ERR_SUCCESS;
}

void Reader::Deleter::cleanup(archive* ptr_)
{
	if (ptr_ == NULL)
		return;
	archive_read_close(ptr_);
	archive_read_free(ptr_);
}

} // namespace Archive

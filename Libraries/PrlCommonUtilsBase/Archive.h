/*
 * Copyright (C) 2016 Parallels IP Holdings GmbH
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

#ifndef PRL_COMMON_UTILS_ARCHIVE_CPP
#define PRL_COMMON_UTILS_ARCHIVE_CPP

#include <QString>
#include <QScopedPointer>

#include <archive.h>
#include <prlsdk/PrlErrors.h>

namespace Archive
{
///////////////////////////////////////////////////////////////////////////////
// struct Writer

struct Writer
{
	static Writer* create(const QString& path_);

	PRL_RESULT append(const QString& file_, const QString& path_);

	struct Deleter
	{
		static void cleanup(archive* ptr_);
	};

private:
	explicit Writer(archive* archive_): m_archive(archive_)
	{
	}

	QScopedPointer<archive, Deleter> m_archive;
};

///////////////////////////////////////////////////////////////////////////////
// struct Reader

struct Reader
{
	static PRL_RESULT extractAll(const QString& archive_, const QString& path_);

private:
	static PRL_RESULT copy(archive* read_, archive* write_);

	struct Deleter
	{
		static void cleanup(archive* ptr_);
	};
};

} // namespace Archive

#endif // PRL_COMMON_UTILS_ARCHIVE_CPP

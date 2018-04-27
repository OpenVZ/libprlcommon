/*
 * Copyright (c) 1999-2017, Parallels International GmbH
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

#include <fcntl.h>
#include "../Logging/Logging.h"

#include "PloopDisk.h"
#include "Qcow2Disk.h"
#include "NbdDisk.h"

namespace VirtualDisk
{
namespace Parameters
{

///////////////////////////////////////////////////////////////////////////////
// Disk

Disk::Disk():
	m_heads(0), m_cylinders(0), m_sectors(0),
	m_sizeInSectors(0), m_blockSize(0)
{
	bzero(&m_uid, sizeof(m_uid));
}

Disk::Disk(const void *data):
	m_heads(0), m_cylinders(0), m_sectors(0),
	m_sizeInSectors(0), m_blockSize(0)
{
	bzero(&m_uid, sizeof(m_uid));

	if (!data)
		return;

	PRL_DISK_PARAMETERS_PTR disk = (PRL_DISK_PARAMETERS_PTR)data;
	m_heads = disk->m_Heads;
	m_cylinders = disk->m_Cylinders;
	m_sectors = disk->m_Sectors;
	m_sizeInSectors = disk->m_SizeInSectors;
	m_blockSize = disk->m_BlockSize;
	m_name = disk->Name;
	memmove(&m_uid, &disk->Uid, sizeof(m_uid));

	for (unsigned i = 0; i < disk->m_Parts; ++i)
	{
		PRL_IMAGE_PARAMETERS_PTR pImage = (PRL_IMAGE_PARAMETERS_PTR)(disk + 1) + i;
		Image storage;
		storage.setType(pImage->Type);
		storage.setStart(pImage->uStart);
		storage.setSize(pImage->uSize);
		storage.setFilename(pImage->pFileName);
		addStorage(storage);
	}
}

PRL_UINT32 Disk::getStringSize(const QString &s)
{
	return s.toUtf8().size() + 1;
}

PRL_UINT32 Disk::getConstantSize() const
{
	return sizeof(PRL_DISK_PARAMETERS) +
		   sizeof(PRL_IMAGE_PARAMETERS) * m_storages.size();
}

PRL_UINT32 Disk::getBufferSize() const
{
	PRL_UINT32 variableData = getStringSize(m_name);
	Q_FOREACH(const Image &image, m_storages)
		variableData += getStringSize(image.getFilename());

	return getConstantSize() + variableData;
}

PRL_RESULT Disk::fillBuffer(void *data, PRL_UINT32 bufSize) const
{
	if (bufSize < getBufferSize())
		return PRL_ERR_INVALID_ARG;

	bzero(data, bufSize);

	PRL_DISK_PARAMETERS_PTR disk = (PRL_DISK_PARAMETERS_PTR)data;
	typedef QPair<char **, QString> pair_t;
	QList<pair_t> strings;

	disk->m_Heads = m_heads;
	disk->m_Cylinders = m_cylinders;
	disk->m_Sectors = m_sectors;
	disk->m_SizeInSectors = m_sizeInSectors;
	disk->m_BlockSize = m_blockSize;
	disk->m_Parts = m_storages.size();
	disk->m_Storages = (PRL_IMAGE_PARAMETERS_PTR)(disk + 1);
	memmove(&disk->Uid, &m_uid, sizeof(m_uid));
	strings << qMakePair(&disk->Name, m_name);

	for (unsigned i = 0; i < m_storages.size(); ++i)
	{
		PRL_IMAGE_PARAMETERS_PTR pImage = (PRL_IMAGE_PARAMETERS_PTR)(disk + 1) + i;
		pImage->Type = m_storages[i].getType();
		pImage->uStart = m_storages[i].getStart();
		pImage->uSize = m_storages[i].getSize();
		strings << qMakePair(&pImage->pFileName, m_storages[i].getFilename());
	}

	char *pStrings = (char*)data + getConstantSize();

	Q_FOREACH(const pair_t &pair, strings)
	{
		PRL_UINT32 size = getStringSize(pair.second);
		strncpy(pStrings, pair.second.toUtf8().constData(), size);
		*pair.first = pStrings;
		pStrings += size;
	}

	return PRL_ERR_SUCCESS;
}

} // namespace Parameters

Format* detectImageFormat(const QString &fname)
{
	if (fname.contains("nbd"))
		return new (std::nothrow) NbdDisk;

	if (!QFileInfo(fname).exists())
		return NULL;

	if (QFileInfo(Ploop::getDescriptorPath(fname)).exists())
		return new (std::nothrow) Ploop;

	return new (std::nothrow) Qcow2;
}

Format::flags_type Format::convertFlags(PRL_DISK_OPEN_FLAGS flags)
{
	int openFlags = O_DIRECT;
	// We have to read during aligned write, so do not use O_WRONLY.
	if (flags & PRL_DISK_WRITE)
		openFlags |= O_RDWR;
	else if (flags & PRL_DISK_READ)
		openFlags |= O_RDONLY;
	else
	{
		WRITE_TRACE(DBG_FATAL, "Invalid flags");
		return Error::Simple(PRL_ERR_INVALID_ARG);
	}
	return openFlags;
}

} // namespace VirtualDisk

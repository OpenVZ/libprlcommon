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
#ifndef __VIRTUAL_DISK__
#define __VIRTUAL_DISK__

#include <vector>
#include <QString>
#include <prlsdk/PrlDisk.h>
#include <boost/noncopyable.hpp>
#include <boost/variant.hpp>

#include "../PrlCommonUtilsBase/SysError.h"
#include "../PrlCommonUtilsBase/ErrorSimple.h"

class CSparseBitmap;
namespace VirtualDisk
{
namespace Parameters
{

///////////////////////////////////////////////////////////////////////////////
// Image

struct Image
{
	Image():
		m_type(PRL_DISK_INVALID), m_start(0), m_size(0)
	{
	}

	PRL_IMAGE_TYPE getType() const
	{
		return m_type;
	}

	void setType(PRL_IMAGE_TYPE type)
	{
		m_type = type;
	}

	PRL_UINT64 getStart() const
	{
		return m_start;
	}

	void setStart(PRL_UINT64 start)
	{
		m_start = start;
	}

	PRL_UINT64 getSize() const
	{
		return m_size;
	}

	void setSize(PRL_UINT64 size)
	{
		m_size = size;
	}

	const QString& getFilename() const
	{
		return m_filename;
	}

	void setFilename(const QString &filename)
	{
		m_filename = filename;
	}

private:
	PRL_IMAGE_TYPE m_type;
	PRL_UINT64 m_start;
	PRL_UINT64 m_size;
	QString m_filename;
};

///////////////////////////////////////////////////////////////////////////////
// Disk

struct Disk
{
	Disk();
	explicit Disk(const void *data);

	PRL_UINT64 getHeads() const
	{
		return m_heads;
	}

	void setHeads(PRL_UINT64 heads)
	{
		m_heads = heads;
	}

	PRL_UINT64 getCylinders() const
	{
		return m_cylinders;
	}

	void setCylinders(PRL_UINT64 cylinders)
	{
		m_cylinders = cylinders;
	}

	PRL_UINT64 getSectors() const
	{
		return m_sectors;
	}

	void setSectors(PRL_UINT64 sectors)
	{
		m_sectors = sectors;
	}

	PRL_UINT64 getSizeInSectors() const
	{
		return m_sizeInSectors;
	}

	void setSizeInSectors(PRL_UINT64 sizeInSectors)
	{
		m_sizeInSectors = sizeInSectors;
	}

	PRL_UINT64 getBlockSize() const
	{
		return m_blockSize;
	}

	void setBlockSize(PRL_UINT64 blockSize)
	{
		m_blockSize = blockSize;
	}

	const std::vector<Image>& getStorages() const
	{
		return m_storages;
	}

	void setStorages(const std::vector<Image>& storages)
	{
		m_storages = storages;
	}

	void addStorage(const Image &storage)
	{
		m_storages.push_back(storage);
	}

	const PRL_GUID& getUid() const
	{
		return m_uid;
	}

	void setUid(const PRL_GUID &uid)
	{
		memmove(&m_uid, &uid, sizeof(m_uid));
	}

	const QString& getName() const
	{
		return m_name;
	}

	void setName(const QString &name)
	{
		m_name = name;
	}

	PRL_UINT32 getBufferSize() const;
	PRL_RESULT fillBuffer(void *data, PRL_UINT32 bufSize) const;

private:
	PRL_UINT32 getConstantSize() const;

	static PRL_UINT32 getStringSize(const QString &s);

	PRL_UINT32 m_heads;
	PRL_UINT32 m_cylinders;
	PRL_UINT32 m_sectors;
	PRL_UINT64 m_sizeInSectors;
	PRL_UINT32 m_blockSize;
	std::vector<Image> m_storages;
	PRL_GUID m_uid;
	QString m_name;
};

typedef Prl::Expected<Disk, Error::Simple> disk_type;

} // namespace Parameters

namespace Policy
{

///////////////////////////////////////////////////////////////////////////////
// struct Offset

struct Offset
{
	explicit Offset(PRL_UINT64 data):
		m_data(data)
	{
	}

	PRL_UINT64 getData() const
	{
		return m_data;
	}

private:
	PRL_UINT64 m_data;
};

} // namespace Policy

typedef boost::variant<
	boost::blank,
	Policy::Offset
> policy_type;
typedef std::vector<policy_type> policyList_type;

///////////////////////////////////////////////////////////////////////////////
// struct Format

struct Format : boost::noncopyable
{
	typedef Prl::Expected<int, Error::Simple> flags_type;

	virtual ~Format() {};
	virtual PRL_RESULT open(const QString &fileName,
			const PRL_DISK_OPEN_FLAGS flags,
			const policyList_type &policies = policyList_type()) = 0;
	virtual PRL_RESULT read(void *data, PRL_UINT32 sizeBytes,
			PRL_UINT64 offSec) = 0;
	virtual PRL_RESULT write(const void *data, PRL_UINT32 sizeBytes,
			PRL_UINT64 offSec) = 0;
	virtual Parameters::disk_type getInfo(void) = 0;
	virtual PRL_RESULT close(void) = 0;
	virtual PRL_RESULT cloneState(const QString &uuid,
			const QString &target) = 0;
	virtual CSparseBitmap *getUsedBlocksBitmap(UINT32 granularity,
			PRL_RESULT &err) = 0;
	virtual CSparseBitmap *getTrackingBitmap(const QString &uuid) = 0;

protected:
	static flags_type convertFlags(PRL_DISK_OPEN_FLAGS flags);
};

Format* detectImageFormat(const QString &image);

} // namespace VirtualDisk
#endif // __VIRTUAL_DISK__

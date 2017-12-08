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
#include <unistd.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <dlfcn.h>

#include <prlsdk/PrlErrors.h>

#include <ploop/libploop.h>
#include <ploop/dynload.h>

#include <QByteArray>

#include "PloopDisk.h"
#include "Libraries/Logging/Logging.h"
#include "Libraries/VirtualDisk/SparseBitmap.h"

typedef void (* resolve_functions)(struct ploop_functions *);

namespace VirtualDisk
{

///////////////////////////////////////////////////////////////////////////////
// struct LibPloop

struct LibPloop : boost::noncopyable
{
	LibPloop() : m_Handle(NULL)
	{
		load();
	}

	~LibPloop()
	{
		if (m_Handle != NULL)
			dlclose(m_Handle);
	}

	struct ploop_functions *getFunctions()
	{
		return m_Handle ? &m_fun : NULL;
	}

	void load();

private:
	void *m_Handle;
	struct ploop_functions m_fun;
};

void LibPloop::load()
{
	if (m_Handle != NULL)
		return;

	m_Handle = dlopen("libploop.so.7", RTLD_LAZY);
	if (m_Handle == NULL)
		return;

	resolve_functions f = (resolve_functions) dlsym(m_Handle,
			"ploop_resolve_functions");
	if (f == NULL)
	{
		WRITE_TRACE(DBG_FATAL, "Failed to load ploop_resolve_functions: %s",  dlerror());
		dlclose(m_Handle);
		m_Handle = NULL;
		return;
	}

	f(&m_fun);
}

Q_GLOBAL_STATIC(LibPloop, getLibPloop)

///////////////////////////////////////////////////////////////////////////////
// struct Ploop

Ploop::Ploop() :
	m_flags(PRL_DISK_NOTHING), m_di(NULL), m_wasMmounted(false)
{
	m_ploop = getLibPloop()->getFunctions();
}

Ploop::~Ploop()
{
	close();
}

QString Ploop::getDescriptorPath(const QString &fileName)
{
	return fileName + "/DiskDescriptor.xml";
}

PRL_RESULT Ploop::umount()
{
	if (!m_wasMmounted)
		return PRL_ERR_SUCCESS;

	if (m_ploop->umount_image(m_di))
	{
		WRITE_TRACE(DBG_FATAL, "ploop_mount_image: %s",
				m_ploop->get_last_error());
		return PRL_ERR_FAILURE;

	}
	m_wasMmounted = false;

	return PRL_ERR_SUCCESS;
}

PRL_RESULT Ploop::mount()
{
	PRL_RESULT rc;
	char dev[PATH_MAX];

	if (m_ploop == NULL)
		return PRL_ERR_UNINITIALIZED;

	if (m_wasMmounted)
		return PRL_ERR_SUCCESS;

	rc = m_ploop->get_dev(m_di, dev, sizeof(dev));
	if (rc == -1)
	{
		WRITE_TRACE(DBG_FATAL, "ploop_get_dev: %s",
				m_ploop->get_last_error());
		return PRL_ERR_FAILURE;
	}

	if (rc)
	{
		struct ploop_mount_param p = ploop_mount_param();

		m_ploop->set_component_name(m_di, "prl-e1871f4a");
		if (m_flags & PRL_DISK_READ)
			p.ro = 1;

		if (m_ploop->mount_image(m_di, &p))
		{
			WRITE_TRACE(DBG_FATAL, "ploop_mount_image: %s",
					m_ploop->get_last_error());

			return PRL_ERR_FAILURE;
		}

		snprintf(dev, sizeof(dev), "%s", p.device);
		m_wasMmounted = true;
	}

	rc = m_file.open(dev, O_DIRECT | (m_flags & PRL_DISK_READ ? O_RDONLY : O_RDWR));
	if (rc)
		WRITE_TRACE(DBG_FATAL, "Failed to open %s", dev);

	return rc;
}

PRL_RESULT Ploop::open(const QString &fileName,
		const PRL_DISK_OPEN_FLAGS flags,
		const policyList_type &policies)
{
	Q_UNUSED(policies);


	if (m_di != NULL)
		return PRL_ERR_INVALID_ARG;

	if (m_ploop == NULL)
		return PRL_ERR_UNINITIALIZED;

	if (m_ploop->open_dd(&m_di, getDescriptorPath(fileName).toUtf8().constData()))
	{
		WRITE_TRACE(DBG_FATAL, "ploop_open_dd: %s",
				m_ploop->get_last_error());
		return PRL_ERR_FAILURE;
	}

	if (m_ploop->read_dd(m_di))
	{
		close();
		WRITE_TRACE(DBG_FATAL, "ploop_read_dd: %s",
				m_ploop->get_last_error());
		return PRL_ERR_FAILURE;
	}

	m_flags = flags;

	return PRL_ERR_SUCCESS;
}

PRL_RESULT Ploop::create(const QString &fileName,
		const Parameters::Disk &params)
{
	if (m_ploop == NULL)
		return PRL_ERR_UNINITIALIZED;

	if (fileName.isEmpty())
		return PRL_ERR_INVALID_ARG;

	struct ploop_create_param x = ploop_create_param();

	QString f = fileName + "/root.hds";
	QByteArray image(f.toUtf8().constData());
	x.image = image.data();
	x.size = params.getSizeInSectors();
	x.blocksize = params.getBlockSize();

	if (m_ploop->create_image(&x)) {
		WRITE_TRACE(DBG_FATAL, "ploop_create_image: %s",
				m_ploop->get_last_error());

		return PRL_ERR_FAILURE;
	}

	return PRL_ERR_SUCCESS;
}

PRL_RESULT Ploop::read(void *data, PRL_UINT32 sizeBytes,
		PRL_UINT64 offSec)
{
	if (m_di == NULL)
		return PRL_ERR_UNINITIALIZED;

	if (!m_wasMmounted && mount())
		return PRL_ERR_FAILURE;

	return m_file.pread(data, sizeBytes, offSec * 512);
}

PRL_RESULT Ploop::write(const void *data, PRL_UINT32 sizeBytes,
		PRL_UINT64 offSec)
{
	if (m_di == NULL)
		return PRL_ERR_UNINITIALIZED;

	if (!m_wasMmounted && mount())
		return PRL_ERR_FAILURE;

	return m_file.pwrite(data, sizeBytes, offSec * 512);
}

Parameters::disk_type Ploop::getInfo(void)
{
	if (m_di == NULL)
		return Error::Simple(PRL_ERR_UNINITIALIZED);

	Parameters::Disk disk;

	disk.setHeads(m_di->heads);
	disk.setCylinders(m_di->cylinders);
	disk.setSectors(m_di->sectors);

	disk.setSizeInSectors(m_di->size);
	disk.setBlockSize(m_di->blocksize);

	Parameters::Image image;
	image.setType(m_di->mode == PLOOP_EXPANDED_MODE ?
				PRL_IMAGE_COMPRESSED : PRL_IMAGE_PLAIN);
	image.setStart(0);
	image.setSize(m_di->size);

	disk.addStorage(image);

	return disk;
}

PRL_RESULT Ploop::close(void)
{
	if (m_ploop == NULL)
		return PRL_ERR_UNINITIALIZED;

	if (m_di == NULL)
		return PRL_ERR_SUCCESS;

	/* release device first */
	m_file.close();

	m_ploop->close_dd(m_di);
	m_di = NULL;

	return PRL_ERR_SUCCESS;
}

CSparseBitmap *Ploop::getUsedBlocksBitmap(UINT32 granularity,
		PRL_RESULT &err)
{
	if (m_ploop == NULL || m_di == NULL)
		return NULL;

	UINT64 sectors = m_di->size;
	CSparseBitmap *res = CSparseBitmap::Create(sectors, granularity, err);
	if (res == NULL)
		return NULL;

	err = res->SetAll();
	if (PRL_FAILED(err)) {
		delete res;
		return NULL;
	}

	return res;
}

CSparseBitmap *Ploop::getTrackingBitmap()
{
	PRL_RESULT err;
	UINT64 sectors = m_di->size;
	CSparseBitmap *res = CSparseBitmap::Create(sectors, 1 << 7, err);
	if (res == NULL)
		return NULL;

	err = res->SetAll();
	if (PRL_FAILED(err)) {
		delete res;
		return NULL;
	}

	return res;
}

PRL_RESULT Ploop::cloneState(const QString &uuid, const QString &target)
{
	if (m_ploop == NULL || m_di == NULL || m_ploop->clone_dd == NULL)
		return PRL_ERR_UNINITIALIZED;

	if (m_ploop->clone_dd(m_di, uuid.toUtf8().constData(),
				target.toUtf8().constData()))
	{
		WRITE_TRACE(DBG_FATAL, "ploop_clone_dd(%s): %s",
				target.toUtf8().constData(),
				m_ploop->get_last_error());
		return PRL_ERR_FAILURE;
	}

	return PRL_ERR_SUCCESS;
}

} // namespace VirtualDisk

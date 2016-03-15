/*
 * Copyright (C) 1999-2016 Parallels IP Holdings GmbH
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

#include "PloopDisk.h"
#include "Libraries/Logging/Logging.h"
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
	m_di(NULL), m_wasMmounted(false)
{
	m_ploop = getLibPloop()->getFunctions();
}

Ploop::~Ploop()
{
	close();
}

QString Ploop::getDescriptorPath(const QString &fileName) const
{
	return fileName + "/DiskDescriptor.xml";
}

PRL_RESULT Ploop::open(const QString &fileName,
		const PRL_DISK_OPEN_FLAGS flags)
{
	char dev[PATH_MAX];
	PRL_RESULT rc;

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

	rc = m_ploop->get_dev(m_di, dev, sizeof(dev));
	if (rc == -1)
	{
		WRITE_TRACE(DBG_FATAL, "ploop_get_dev: %s",
				m_ploop->get_last_error());
		close();
		return PRL_ERR_FAILURE;
	}

	if (rc)
	{
		struct ploop_mount_param p = ploop_mount_param();

		if (m_ploop->mount_image(m_di, &p))
		{
			WRITE_TRACE(DBG_FATAL, "ploop_mount_image: %s",
					m_ploop->get_last_error());

			close();
			return PRL_ERR_FAILURE;
		}

		snprintf(dev, sizeof(dev), "%s", p.device);
		m_wasMmounted = true;
	}

	int f = O_DIRECT | (flags & PRL_DISK_READ ? O_RDONLY : O_RDWR);

	rc = m_file.open(dev, f);
	if (rc)
		close();

	return rc;
}

PRL_RESULT Ploop::read(void *data, PRL_UINT32 sizeBytes,
		PRL_UINT64 offSec)
{
	if (m_di == NULL)
		return PRL_ERR_UNINITIALIZED;

	return m_file.pread(data, sizeBytes, offSec * 512);
}

PRL_RESULT Ploop::write(const void *data, PRL_UINT32 sizeBytes,
		PRL_UINT64 offSec)
{
	if (m_di == NULL)
		return PRL_ERR_UNINITIALIZED;

	return m_file.pwrite(data, sizeBytes, offSec * 512);
}

PRL_RESULT Ploop::close(void)
{
	if (m_ploop == NULL)
		return PRL_ERR_UNINITIALIZED;

	if (m_di == NULL)
		return PRL_ERR_SUCCESS;

	/* release device first */
	m_file.close();

	if (m_wasMmounted)
	{
		if (m_ploop->umount_image(m_di))
		{
			WRITE_TRACE(DBG_FATAL, "ploop_mount_image: %s",
					m_ploop->get_last_error());
		}
		m_wasMmounted = false;

	}

	m_ploop->close_dd(m_di);
	m_di = NULL;

	return PRL_ERR_SUCCESS;
}

} // namespace VirtualDisk

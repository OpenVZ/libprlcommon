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

#include <pcs-core/nbd_clnt.h>
#include <pcs-core/pcs_process.h>

//#include <nbd/libnbd.h>
//#include <nbd/dynload.h>

enum {SECTOR_SIZE = 512};

extern "C" {
struct nbd_functions {
	struct nbd_client * (*nbd_client_init) (void);
	int  (*nbd_client_connect) (struct nbd_client *clnt, const char *addr, const char *exp_name);
	int  (*nbd_client_disconnect) (struct nbd_client *clnt);
	void (*nbd_client_getsize) (struct nbd_client *clnt, u32 *blksize, u64 *size);
	int  (*nbd_client_read) (struct nbd_client *clnt, u64 offs, void *buf, u32 size);
	int  (*nbd_client_write) (struct nbd_client *clnt, u64 offs, const void *buf, u32 size);
	int  (*nbd_client_blk_status) (struct nbd_client *clnt, const char *meta_ctx, u64 offs, u32 size, nbd_blk_status_cb cb, void *cb_arg);
	void (*nbd_client_fini) (struct nbd_client *clnt);
};
}

#include "NbdDisk.h"
#include "Libraries/Logging/Logging.h"

typedef void (* resolve_functions)(struct nbd_functions *);

namespace VirtualDisk
{

///////////////////////////////////////////////////////////////////////////////
// struct LibNbd

struct LibNbd : boost::noncopyable
{
	LibNbd() : m_Handle(NULL)
	{
		load();
	}

	~LibNbd()
	{
		if (m_Handle != NULL)
			dlclose(m_Handle);
	}

	struct nbd_functions *getFunctions()
	{
		return m_Handle ? &m_func : NULL;
	}

	void load();

private:
	void *m_Handle;
	struct nbd_functions m_func;
	struct pcs_process m_proc;
};

void LibNbd::load()
{
	if (m_Handle != NULL)
		return;

	m_Handle = dlopen("libnbd.so", RTLD_LAZY);
	if (m_Handle == NULL)
		return;

	m_func.nbd_client_init = (struct nbd_client *(*)(void))
		dlsym(m_Handle, "nbd_client_init");
	m_func.nbd_client_fini = (void (*)(nbd_client*))
		dlsym(m_Handle, "nbd_client_fini");
	m_func.nbd_client_connect = (int (*)(nbd_client*, const char*, const char*))
		dlsym(m_Handle, "nbd_client_connect");
	m_func.nbd_client_disconnect = (int (*)(nbd_client*))
		dlsym(m_Handle, "nbd_client_disconnect");
	m_func.nbd_client_getsize = (void (*)(nbd_client*, u32*, u64*))
		dlsym(m_Handle, "nbd_client_getsize");
	m_func.nbd_client_read = (int (*)(nbd_client*, u64, void*, u32))
		dlsym(m_Handle, "nbd_client_read");
	m_func.nbd_client_write = (int (*)(nbd_client*, u64, const void*, u32))
		dlsym(m_Handle, "nbd_client_write");

	if (m_func.nbd_client_init == NULL ||
	    m_func.nbd_client_fini == NULL ||
	    m_func.nbd_client_connect == NULL ||
	    m_func.nbd_client_disconnect == NULL ||
	    m_func.nbd_client_getsize == NULL ||
	    m_func.nbd_client_read == NULL ||
	    m_func.nbd_client_write == NULL)
	{
		WRITE_TRACE(DBG_FATAL, "Failed to load nbd_resolve_functions: %s",  dlerror());
		dlclose(m_Handle);
		m_Handle = NULL;
		return;
	}

}

Q_GLOBAL_STATIC(LibNbd, getLibNbd)

///////////////////////////////////////////////////////////////////////////////
// struct NbdDisk

NbdDisk::NbdDisk() :
	m_clnt(NULL), m_connected(false)
{
	m_nbd = getLibNbd()->getFunctions();
}

NbdDisk::~NbdDisk()
{
	close();
}

PRL_RESULT NbdDisk::open(const QString &fileName,
		const PRL_DISK_OPEN_FLAGS flags,
		const policyList_type &policies)
{
	Q_UNUSED(policies);
	Q_UNUSED(flags);

	int rc;

	if (m_clnt != NULL)
		return PRL_ERR_INVALID_ARG;

	if (m_nbd == NULL)
		return PRL_ERR_UNINITIALIZED;

	m_fileName = fileName;

	if (!fileName.startsWith("nbd://"))
		return PRL_ERR_INVALID_ARG;

	QStringList f = fileName.split("/");
	if (f.size() != 4)
		return PRL_ERR_INVALID_ARG;

	QString addr = f[2];
	QString name = f[3];

	if (addr.isEmpty() || name.isEmpty())
		return PRL_ERR_INVALID_ARG;

	m_clnt = m_nbd->nbd_client_init();
	if (m_clnt == NULL)
		return PRL_ERR_FAILURE;

	if ((rc = m_nbd->nbd_client_connect(m_clnt, qPrintable(addr), qPrintable(name))) != 0)
	{
		WRITE_TRACE(DBG_FATAL, "nbd_client_connect: %d", rc);
		return PRL_ERR_FAILURE;
	}

	m_connected = true;

	return 0;
}


PRL_RESULT NbdDisk::read(void *data, PRL_UINT32 sizeBytes,
		PRL_UINT64 offSec)
{
	if (m_clnt == NULL)
		return PRL_ERR_UNINITIALIZED;

	return m_nbd->nbd_client_read(m_clnt, offSec * 512, data, sizeBytes);
}

PRL_RESULT NbdDisk::write(const void *data, PRL_UINT32 sizeBytes,
		PRL_UINT64 offSec)
{
	if (m_clnt == NULL)
		return PRL_ERR_UNINITIALIZED;

	return m_nbd->nbd_client_write(m_clnt, offSec * 512, data, sizeBytes);
}

Parameters::disk_type NbdDisk::getInfo(void)
{
	if (m_clnt == NULL)
		return Error::Simple(PRL_ERR_UNINITIALIZED);

	Parameters::Disk disk;

	u64 size;
	u32 blksize;

	m_nbd->nbd_client_getsize(m_clnt, &blksize, &size);

	disk.setSizeInSectors(size / SECTOR_SIZE);
	disk.setBlockSize(blksize / SECTOR_SIZE);

	Parameters::Image image;
	image.setType(PRL_IMAGE_TRY_GUESS);
	image.setStart(0);
	image.setSize(size);
	image.setFilename(m_fileName);

	disk.addStorage(image);

	return disk;
}

PRL_RESULT NbdDisk::close(void)
{
	if (m_nbd == NULL)
		return PRL_ERR_UNINITIALIZED;

	if (m_clnt == NULL)
		return PRL_ERR_SUCCESS;

	if (m_connected)
	{
		m_nbd->nbd_client_disconnect(m_clnt);
		m_connected = false;
	}

	m_nbd->nbd_client_fini(m_clnt);

	m_clnt = NULL;

	return PRL_ERR_SUCCESS;
}

PRL_RESULT NbdDisk::cloneState(const QString &uuid, const QString &target)
{
	Q_UNUSED(uuid);
	Q_UNUSED(target);
	return PRL_ERR_UNIMPLEMENTED;
}

CSparseBitmap *NbdDisk::getUsedBlocksBitmap(UINT32 granularity, PRL_RESULT &err)
{
	Q_UNUSED(granularity);
	Q_UNUSED(err);
	return NULL;
}

CSparseBitmap *NbdDisk::getTrackingBitmap()
{
	return NULL;
}

} // namespace VirtualDisk

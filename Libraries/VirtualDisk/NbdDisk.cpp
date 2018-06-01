/*
 * Copyright (c) 2018 Virtuozzo International GmbH.  All rights reserved.
 *
 * This file is part of OpenVZ. OpenVZ is free software;
 * you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation;
 * either version 2 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 * Our contact details: Virtuozzo International GmbH, Vordergasse 59, 8200
 * Schaffhausen, Switzerland.
 */
#include <dlfcn.h>

#include <pcs-core/nbd_clnt.h>

#include "NbdDisk.h"
#include "Libraries/Logging/Logging.h"
#include "Libraries/VirtualDisk/SparseBitmap.h"
#include "Libraries/Std/BitOps.h"

enum { 
	SECTOR_SIZE = 512,
	DEFAULT_GRANULARITY = 128,
};

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
};

void LibNbd::load()
{
	if (m_Handle != NULL)
		return;

	m_Handle = dlopen("libpcs_nbd.so", RTLD_LAZY);
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
	m_func.nbd_client_blk_status = (int (*) (struct nbd_client*, const char*, u64, u32, nbd_blk_status_cb, void*))
		dlsym(m_Handle, "nbd_client_blk_status");

	if (m_func.nbd_client_init == NULL ||
	    m_func.nbd_client_fini == NULL ||
	    m_func.nbd_client_connect == NULL ||
	    m_func.nbd_client_disconnect == NULL ||
	    m_func.nbd_client_getsize == NULL ||
	    m_func.nbd_client_read == NULL ||
	    m_func.nbd_client_write == NULL ||
	    m_func.nbd_client_blk_status == NULL)
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
	m_clnt(NULL)
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

	m_url = fileName;

	if (!m_url.isValid() || m_url.scheme() != "nbd")
		return PRL_ERR_INVALID_ARG;

	QRegExp rx("\\{(.*)\\}");
	rx.indexIn(m_url.path());
	m_uuid = rx.cap(1);

	m_clnt = m_nbd->nbd_client_init();
	if (m_clnt == NULL)
		return PRL_ERR_FAILURE;

	if ((rc = m_nbd->nbd_client_connect(m_clnt,
		qPrintable(m_url.authority()), qPrintable(m_url.path().mid(1)))) != 0)
	{
		WRITE_TRACE(DBG_FATAL, "nbd_client_connect: %d", rc);
		m_nbd->nbd_client_fini(m_clnt);
		m_clnt = NULL;
		return PRL_ERR_FAILURE;
	}

	return PRL_ERR_SUCCESS;
}


PRL_RESULT NbdDisk::read(void *data, PRL_UINT32 sizeBytes,
		PRL_UINT64 offSec)
{
	if (m_clnt == NULL)
		return PRL_ERR_UNINITIALIZED;

	u64 bytes = 0;
	while (bytes < sizeBytes) {
		int rc = m_nbd->nbd_client_read(m_clnt, offSec * SECTOR_SIZE + bytes,
			(char*)data + bytes, sizeBytes - bytes);
		if (rc < 0) {
			WRITE_TRACE(DBG_FATAL, "nbd_client_read: %d", rc);
			return PRL_ERR_FAILURE;
		}
		bytes += rc;
	}

	return PRL_ERR_SUCCESS;
}

PRL_RESULT NbdDisk::write(const void *data, PRL_UINT32 sizeBytes,
		PRL_UINT64 offSec)
{
	if (m_clnt == NULL)
		return PRL_ERR_UNINITIALIZED;

	u64 bytes = 0;
	while (bytes < sizeBytes) {
		int rc = m_nbd->nbd_client_write(m_clnt, offSec * SECTOR_SIZE + bytes,
			(char*)data + bytes, sizeBytes - bytes);
		if (rc < 0) {
			WRITE_TRACE(DBG_FATAL, "nbd_client_write: %d", rc);
			return PRL_ERR_FAILURE;
		}
		bytes += rc;
	}

	return PRL_ERR_SUCCESS;
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
	image.setFilename(m_url.toString());

	disk.addStorage(image);

	return disk;
}

PRL_RESULT NbdDisk::close(void)
{
	if (m_nbd == NULL)
		return PRL_ERR_UNINITIALIZED;

	if (m_clnt == NULL)
		return PRL_ERR_SUCCESS;

	m_nbd->nbd_client_disconnect(m_clnt);

	m_nbd->nbd_client_fini(m_clnt);

	m_clnt = NULL;

	return PRL_ERR_SUCCESS;
}

static void get_bitmap_cb(u64 offs, u32 size, u32 flags, void *arg)
{
	CSparseBitmap *b =  (CSparseBitmap *) arg;
	
	//WRITE_TRACE(DBG_FATAL, "get_bitmap_cb offs %ld size %d flags %x", (long)offs, (int)size, (int)flags);

	/* skip zero-allocated holes */
	if (flags == (NBD_STATE_HOLE|NBD_STATE_ZERO))
		return;

	b->SetRange(offs/SECTOR_SIZE, (offs + size)/SECTOR_SIZE);
}

CSparseBitmap * NbdDisk::getBitmap(
	const char *metactx, UINT32 granularity, const Uuid &uuid, PRL_RESULT &err)
{
	if (m_clnt == NULL) {
		err = PRL_ERR_FAILURE;
		return NULL;
	}

	u64 size;
	u32 blksize;
	m_nbd->nbd_client_getsize(m_clnt, &blksize, &size);

	err = PRL_ERR_SUCCESS;
	CSparseBitmap *res = CSparseBitmap::Create(size/SECTOR_SIZE, granularity, uuid, err);

	if (res == NULL)
		return NULL;

	u64 offs = 0;
	while (offs < size) {
		u32 len = qMin<u64>(size - offs, blksize);
		int rc = m_nbd->nbd_client_blk_status(m_clnt, metactx, offs, len, get_bitmap_cb, res);
		if (rc < 0) {
			delete res;
			err = PRL_ERR_FAILURE;
			return NULL;
		}
		offs += rc;
	}

	return res;
}

CSparseBitmap *NbdDisk::getUsedBlocksBitmap(UINT32 granularity, PRL_RESULT &err)
{
	return getBitmap("base:allocation", granularity, Uuid(), err);
}

CSparseBitmap *NbdDisk::getTrackingBitmap(const QString& uuid)
{
	PRL_RESULT err;
	QString name = QString("qemu-dirty-bitmap:") + uuid;
	return getBitmap(qPrintable(name), DEFAULT_GRANULARITY, uuid, err);
}

PRL_RESULT NbdDisk::cloneState(const QString &uuid, const QString &target)
{
	Q_UNUSED(uuid);
	Q_UNUSED(target);
	return PRL_ERR_UNIMPLEMENTED;
}

} // namespace VirtualDisk

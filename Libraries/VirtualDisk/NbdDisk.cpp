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
#include <errno.h>

#include "NbdDisk.h"
#include "SparseBitmap.h"

#include "Libraries/Logging/Logging.h"
#include "Libraries/Std/BitOps.h"

extern "C" {

typedef void (*nbd_blk_status_cb)(PRL_UINT64 offs, PRL_UINT32 size, PRL_UINT32 flags, void *arg);

struct nbd_client;
struct pcs_process;

struct nbd_functions {
	struct nbd_client * (*nbd_client_init) (void);
	int  (*nbd_client_connect) (struct nbd_client *clnt, const char *addr, const char *exp_name);
	int  (*nbd_client_disconnect) (struct nbd_client *clnt);
	void (*nbd_client_getsize) (struct nbd_client *clnt, PRL_UINT32 *blksize, PRL_UINT64 *size);
	int  (*nbd_client_read) (struct nbd_client *clnt, PRL_UINT64 offs, void *buf, PRL_UINT32 size);
	int  (*nbd_client_write) (struct nbd_client *clnt, PRL_UINT64 offs, const void *buf, PRL_UINT32 size);
	PRL_INT64  (*nbd_client_blk_status) (struct nbd_client *clnt, const char *meta_ctx,
		PRL_UINT64 offs, PRL_UINT32 size, nbd_blk_status_cb cb, void *cb_arg);
	void (*nbd_client_fini) (struct nbd_client *clnt);
};

struct pcs_functions {
	int  (*pcs_process_alloc) (struct pcs_process **);
	int  (*pcs_process_fini) (struct pcs_process *);
	int  (*pcs_process_start) (struct pcs_process *, const char *name);
	void (*pcs_process_terminate) (struct pcs_process *);
	int  (*pcs_process_wait) (struct pcs_process *);
	void (*pcs_move_to_coroutine) (struct pcs_process *);
	void (*pcs_move_from_coroutine) (void);
	void (*pcs_free) (void*);
};

}

namespace VirtualDisk
{

///////////////////////////////////////////////////////////////////////////////
// struct NbdClient

struct NbdClient : boost::noncopyable
{
	NbdClient *getNbdClient(void) { return isLoaded() ? this : NULL; }

	NbdClient() : m_clnt(NULL), m_proc(NULL), loaded(false)
	{
		load();
	}

	void load();
	bool isLoaded() const { return loaded; }

	// nbd_client wrappers
	int  init(void);
	void fini(void);
	int  connect(const char *addr, const char *exp_name);
	int  disconnect(void);
	void getsize(PRL_UINT32 *blksize, PRL_UINT64 *size);
	int  read(PRL_UINT64 offs, void *buf, PRL_UINT32 size);
	int  write(PRL_UINT64 offs, const void *buf, PRL_UINT32 size);
	PRL_INT64 get_blk_status(const char *meta_ctx, PRL_UINT64 offs,
		PRL_UINT32 size, nbd_blk_status_cb cb, void *cb_arg);

	// pcs_io wrappers
	int  pcs_init(void);
	void pcs_fini(void);

	void move_to_coroutine(void)
	{
		m_pcs.pcs_move_to_coroutine(m_proc);
	}

	void move_from_coroutine(void)
	{
		m_pcs.pcs_move_from_coroutine();
	}


private:
	struct nbd_functions m_nbd;
	struct pcs_functions m_pcs;

	struct nbd_client  *m_clnt;
	struct pcs_process *m_proc;

	bool loaded;
};

void NbdClient::load()
{
	// resolve libpcs_nbd symbols
	const char *dlname = "libpcs_nbd.so.1";

	void *h = dlopen(dlname, RTLD_LAZY);
	if (h == NULL) {
		WRITE_TRACE(DBG_FATAL, "Failed to load %s: %s", dlname, strerror(errno));
		return;
	}

	m_nbd.nbd_client_init = (struct nbd_client *(*)(void))
		dlsym(h, "nbd_client_init");
	m_nbd.nbd_client_fini = (void (*)(nbd_client*))
		dlsym(h, "nbd_client_fini");
	m_nbd.nbd_client_connect = (int (*)(nbd_client*, const char*, const char*))
		dlsym(h, "nbd_client_connect");
	m_nbd.nbd_client_disconnect = (int (*)(nbd_client*))
		dlsym(h, "nbd_client_disconnect");
	m_nbd.nbd_client_getsize = (void (*)(nbd_client*, PRL_UINT32*, PRL_UINT64*))
		dlsym(h, "nbd_client_getsize");
	m_nbd.nbd_client_read = (int (*)(nbd_client*, PRL_UINT64, void*, PRL_UINT32))
		dlsym(h, "nbd_client_read");
	m_nbd.nbd_client_write = (int (*)(nbd_client*, PRL_UINT64, const void*, PRL_UINT32))
		dlsym(h, "nbd_client_write");
	m_nbd.nbd_client_blk_status = (PRL_INT64 (*) (struct nbd_client*, const char*,
			PRL_UINT64, PRL_UINT32, nbd_blk_status_cb, void*))
		dlsym(h, "nbd_client_blk_status");

	if (m_nbd.nbd_client_init == NULL ||
	    m_nbd.nbd_client_fini == NULL ||
	    m_nbd.nbd_client_connect == NULL ||
	    m_nbd.nbd_client_disconnect == NULL ||
	    m_nbd.nbd_client_getsize == NULL ||
	    m_nbd.nbd_client_read == NULL ||
	    m_nbd.nbd_client_write == NULL ||
	    m_nbd.nbd_client_blk_status == NULL)
	{
		WRITE_TRACE(DBG_FATAL, "Failed to resolve %s functions: %s", dlname, dlerror());
		dlclose(h);
		return;
	}

	// resolve libpcs_io symbols
	dlname = "libpcs_io.so.4";

	h = dlopen(dlname, RTLD_LAZY);
	if (h == NULL) {
		WRITE_TRACE(DBG_FATAL, "Failed to load %s: %s", dlname, strerror(errno));
		return;
	}

	m_pcs.pcs_process_alloc =  (int (*)(struct pcs_process **))
		dlsym(h, "pcs_process_alloc");
	m_pcs.pcs_process_fini = (int (*)(struct pcs_process *))
		dlsym(h, "pcs_process_fini");
	m_pcs.pcs_process_start = (int (*)(struct pcs_process *, const char *name))
		dlsym(h, "pcs_process_start");
	m_pcs.pcs_process_terminate = (void (*)(struct pcs_process *))
		dlsym(h, "pcs_process_terminate");
	m_pcs.pcs_process_wait = (int (*)(struct pcs_process *))
		dlsym(h, "pcs_process_wait");
	m_pcs.pcs_move_to_coroutine = (void (*)(struct pcs_process *))
		dlsym(h, "pcs_move_to_coroutine");
	m_pcs.pcs_move_from_coroutine = (void (*) (void))
		dlsym(h, "pcs_move_from_coroutine");
	m_pcs.pcs_free = (void (*)(void*))
		dlsym(h, "__pcs_free");

	if (m_pcs.pcs_process_alloc == NULL ||
	    m_pcs.pcs_process_fini == NULL ||
	    m_pcs.pcs_process_start == NULL ||
	    m_pcs.pcs_process_terminate == NULL ||
	    m_pcs.pcs_process_wait == NULL ||
	    m_pcs.pcs_move_to_coroutine == NULL ||
	    m_pcs.pcs_move_from_coroutine == NULL ||
	    m_pcs.pcs_free == NULL)
	{
		WRITE_TRACE(DBG_FATAL, "Failed to resolve %s functions: %s", dlname, dlerror());
		dlclose(h);
		return;
	}

	loaded = true;
}

int NbdClient::pcs_init(void)
{
	int rc = m_pcs.pcs_process_alloc(&m_proc);
	if (rc) {
		WRITE_TRACE(DBG_FATAL, "%s: %d", __PRETTY_FUNCTION__, rc);
		return rc;
	}

	rc = m_pcs.pcs_process_start(m_proc, "VzNbdClient");
	if (rc) {
		WRITE_TRACE(DBG_FATAL, "%s: %d", __PRETTY_FUNCTION__, rc);
		m_pcs.pcs_process_fini(m_proc);
		m_pcs.pcs_free(m_proc);
		m_proc = NULL;
		return rc;
	}

	return 0;
}

void NbdClient::pcs_fini(void)
{
	if (m_proc) {
		m_pcs.pcs_process_terminate(m_proc);
		m_pcs.pcs_process_wait(m_proc);
		m_pcs.pcs_process_fini(m_proc);
		m_pcs.pcs_free(m_proc);
		m_proc = NULL;
	}
}

int NbdClient::init(void)
{
	m_clnt = m_nbd.nbd_client_init();
	return m_clnt == NULL;
}

void NbdClient::fini(void)
{
	if (m_clnt) {
		move_to_coroutine();
		m_nbd.nbd_client_fini(m_clnt);
		move_from_coroutine();

		m_clnt = NULL;
	}
}

int NbdClient::connect(const char *addr, const char *exp_name)
{
	move_to_coroutine();
	int rc = m_nbd.nbd_client_connect(m_clnt, addr, exp_name);
	move_from_coroutine();

	if (rc)
		WRITE_TRACE(DBG_FATAL, "%s: %d", __PRETTY_FUNCTION__, rc);
	return rc;
}

int NbdClient::disconnect(void)
{
	move_to_coroutine();
	int rc = m_nbd.nbd_client_disconnect(m_clnt);
	move_from_coroutine();

	if (rc)
		WRITE_TRACE(DBG_FATAL, "%s: %d", __PRETTY_FUNCTION__, rc);
	return rc;
}

void NbdClient::getsize(PRL_UINT32 *blksize, PRL_UINT64 *size)
{
	move_to_coroutine();
	m_nbd.nbd_client_getsize(m_clnt, blksize, size);
	move_from_coroutine();
}

int NbdClient::read(PRL_UINT64 offs, void *buf, PRL_UINT32 size)
{
	move_to_coroutine();
	int rc = m_nbd.nbd_client_read(m_clnt, offs, buf, size);
	move_from_coroutine();

	if (rc)
		WRITE_TRACE(DBG_FATAL, "%s: %d", __PRETTY_FUNCTION__, rc);
	return rc;
}

int NbdClient::write(PRL_UINT64 offs, const void *buf, PRL_UINT32 size)
{
	move_to_coroutine();
	int rc = m_nbd.nbd_client_write(m_clnt, offs, buf, size);
	move_from_coroutine();

	if (rc)
		WRITE_TRACE(DBG_FATAL, "%s: %d", __PRETTY_FUNCTION__, rc);
	return rc;
}

PRL_INT64 NbdClient::get_blk_status(const char *meta_ctx, PRL_UINT64 offs,
	PRL_UINT32 size, nbd_blk_status_cb cb, void *cb_arg)
{
	move_to_coroutine();
	PRL_INT64 rc = m_nbd.nbd_client_blk_status(m_clnt, meta_ctx, offs, size, cb, cb_arg);
	move_from_coroutine();

	if (rc < 0)
		WRITE_TRACE(DBG_FATAL, "%s: %lld", __PRETTY_FUNCTION__, rc);
	return rc;
}

Q_GLOBAL_STATIC(NbdClient, getNbdClient)

///////////////////////////////////////////////////////////////////////////////
// struct NbdDisk

NbdDisk::NbdDisk() : m_nbd(getNbdClient())
{
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

	if (m_nbd == NULL) {
		WRITE_TRACE(DBG_FATAL, "libpcs_nbd.so initialization failed");
		return PRL_ERR_UNINITIALIZED;
	}

	m_url = fileName;

	if (!m_url.isValid() || m_url.scheme() != "nbd") {
		WRITE_TRACE(DBG_FATAL, "Invalid url %s", qPrintable(fileName));
		return PRL_ERR_INVALID_ARG;
	}

	QRegExp rx("\\{(.*)\\}");
	rx.indexIn(m_url.path());
	m_uuid = rx.cap(1);

	if (m_nbd->pcs_init()) {
		close();
		return PRL_ERR_FAILURE;
	}

	if (m_nbd->init()) {
		close();
		return PRL_ERR_FAILURE;
	}

	if (m_nbd->connect(qPrintable(m_url.authority()), qPrintable(m_url.path().mid(1)))) {
		close();
		return PRL_ERR_FAILURE;
	}

	return PRL_ERR_SUCCESS;
}


PRL_RESULT NbdDisk::read(void *data, PRL_UINT32 sizeBytes,
		PRL_UINT64 offSec)
{
	PRL_UINT64 bytes = 0;
	while (bytes < sizeBytes) {
		int rc = m_nbd->read(offSec * SECTOR_SIZE + bytes,
			(char*)data + bytes, sizeBytes - bytes);
		if (rc < 0)
			return PRL_ERR_FAILURE;
		bytes += rc;
	}

	return PRL_ERR_SUCCESS;
}

PRL_RESULT NbdDisk::write(const void *data, PRL_UINT32 sizeBytes,
		PRL_UINT64 offSec)
{
	PRL_UINT64 bytes = 0;
	while (bytes < sizeBytes) {
		int rc = m_nbd->write(offSec * SECTOR_SIZE + bytes,
			(char*)data + bytes, sizeBytes - bytes);
		if (rc < 0)
			return PRL_ERR_FAILURE;
		bytes += rc;
	}

	return PRL_ERR_SUCCESS;
}

Parameters::disk_type NbdDisk::getInfo(void)
{
	Parameters::Disk disk;

	PRL_UINT64 size;
	PRL_UINT32 blksize;
	m_nbd->getsize(&blksize, &size);

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
	m_nbd->disconnect();
	m_nbd->fini();
	m_nbd->pcs_fini();

	return PRL_ERR_SUCCESS;
}

template <class T>
void NbdDisk::Bitmap::setRange(PRL_UINT64 offs, PRL_UINT32 size, PRL_UINT32 flags, void *arg)
{
	std::pair<T*, CSparseBitmap*> *a =
		static_cast<std::pair<T*, CSparseBitmap*>* > (arg);

	//WRITE_TRACE(DBG_FATAL, "get_bitmap_cb offs %ld size %d flags %x", (long)offs, (int)size, (int)flags);

	/* filter range with unwanted flags */
	if ((*a->first)(flags))
		return;

	a->second->SetRange(offs/SECTOR_SIZE, (offs + size)/SECTOR_SIZE);
}

template <class T>
PRL_RESULT NbdDisk::Bitmap::operator()(T policy, int granularity)
{
	reset();

	PRL_UINT64 size;
	PRL_UINT32 blksize;
	m_nbd->getsize(&blksize, &size);

	PRL_RESULT err = PRL_ERR_SUCCESS;
	QScopedPointer<CSparseBitmap> bitmap(CSparseBitmap::Create(
		size/SECTOR_SIZE, granularity, policy.getUuid(), err));
	if (bitmap.isNull())
		return err;

	PRL_UINT64 offs = 0;
	while (offs < size) {
		PRL_UINT32 len = qMin<PRL_UINT64>(size - offs, DEFAULT_BLK_STATUS_RANGE);
		std::pair<T*, CSparseBitmap*> a(&policy, bitmap.data());

		//WRITE_TRACE(DBG_FATAL, "nbd_client_blk_status %ld size %d", (long)offs, (int)len);

		PRL_INT64 rc = m_nbd->get_blk_status(
			policy.getName(), offs, len, &setRange<T>, &a);
		if (rc < 0)
			return PRL_ERR_FAILURE;
		offs += rc;
	}

	reset(bitmap.take());

	return err;
}

CSparseBitmap *NbdDisk::getUsedBlocksBitmap(UINT32 granularity, PRL_RESULT &err)
{
	NbdDisk::Bitmap bitmap(m_nbd);
	err = bitmap(Allocation(), granularity);
	return bitmap.take();
}

CSparseBitmap *NbdDisk::getTrackingBitmap(const QString& uuid)
{
	NbdDisk::Bitmap bitmap(m_nbd);
	bitmap(Dirty(uuid));
	return bitmap.take();
}

PRL_RESULT NbdDisk::cloneState(const QString &uuid, const QString &target)
{
	Q_UNUSED(uuid);
	Q_UNUSED(target);
	return PRL_ERR_UNIMPLEMENTED;
}

} // namespace VirtualDisk

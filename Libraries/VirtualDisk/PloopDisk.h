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
#ifndef __VIRTUAL_DISK_PLOOP__
#define __VIRTUAL_DISK_PLOOP__

#include <QString>

#include "VirtualDisk.h"
#include "Util.h"

namespace VirtualDisk
{
///////////////////////////////////////////////////////////////////////////////
// struct Ploop

struct Ploop : Format
{
	Ploop();
	~Ploop();

	static QString getDescriptorPath(const QString &fileName);

	virtual PRL_RESULT open(const QString &fileName,
			const PRL_DISK_OPEN_FLAGS flags,
			const policyList_type &policies = policyList_type());
	virtual PRL_RESULT read(void *data, PRL_UINT32 sizeBytes,
			PRL_UINT64 offSec);
	virtual PRL_RESULT write(const void *data, PRL_UINT32 sizeBytes,
			PRL_UINT64 offSec);
	virtual Parameters::disk_type getInfo(void);
	virtual PRL_RESULT close(void);
	PRL_RESULT create(const QString &fileName,
			const Parameters::Disk &params);

private:
	struct ploop_disk_images_data *m_di;
	struct ploop_functions *m_ploop;
	IO::File m_file;
	bool m_wasMmounted;
};

} // namespace VirtualDisk
#endif // __VIRTUAL_DISK_PLOOP__

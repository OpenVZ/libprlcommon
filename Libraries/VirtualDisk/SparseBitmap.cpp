/* Copyright (c) 2017 Virtuozzo International GmbH.  All rights reserved.
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

#include <prlsdk/PrlErrorsValues.h>
#include <prlsdk/PrlErrors.h>
#include "Libraries/Std/BitOps.h"
#include "Libraries/Std/sparse_bitmap.h"
#include "SparseBitmap.h"
#include <errno.h>

/* Constructors and assignment are private, use static Create instead */
CSparseBitmap::~CSparseBitmap()
{
	sp_bitmap_destroy(m_Bitmap);
}

CSparseBitmap *CSparseBitmap::Create(UINT64 size, UINT32 granularity,
		const Uuid &uid, UINT32 waitParts, PRL_RESULT &err)
{
	CSparseBitmap *res = new(std::nothrow) CSparseBitmap();

	if (res == NULL) {
		err = PRL_ERR_OUT_OF_MEMORY;
		return NULL;
	}

	err = res->Init(size, granularity, uid, waitParts);
	if (PRL_FAILED(err)) {
		delete res;
		return NULL;
	}

	return res;
}

CSparseBitmap *CSparseBitmap::Create(UINT64 size, UINT32 granularity,
		const Uuid &uid, PRL_RESULT &err)
{
	return Create(size, granularity, uid, 0, err);
}

CSparseBitmap *CSparseBitmap::Create(UINT64 size, UINT32 granularity,
		UINT32 waitParts, PRL_RESULT &err)
{
	return Create(size, granularity, Uuid(), waitParts, err);
}

CSparseBitmap *CSparseBitmap::Create(UINT64 size, UINT32 granularity,
		PRL_RESULT &err)
{
	return Create(size, granularity, Uuid(), 0, err);
}

bool CSparseBitmap::CheckUid(const Uuid &uid) const
{
	return uid == m_Uid;
}

UINT32 CSparseBitmap::GetGranularity() const
{
	return 1lu << m_GranularityBits;
}

const Uuid & CSparseBitmap::GetUid() const
{
	return m_Uid;
}

void CSparseBitmap::SetUid(const Uuid &uid)
{
	m_Uid = uid;
}

UINT64 CSparseBitmap::GetSize() const
{
	return m_Size;
}

PRL_RESULT CSparseBitmap::SetAll()
{
	return ToPrlResult(sp_bitmap_set_all(m_Bitmap));
}

PRL_RESULT CSparseBitmap::SetRange(UINT64 begin, UINT64 end)
{
	return ToPrlResult(sp_bitmap_set_range(m_Bitmap, End(end), Begin(begin)));
}

PRL_RESULT CSparseBitmap::ClearRange(UINT64 begin, UINT64 end)
{
	return ToPrlResult(sp_bitmap_clear_range(m_Bitmap, End(end), Begin(begin)));
}

PRL_RESULT CSparseBitmap::ClearAll()
{
	return ToPrlResult(sp_bitmap_clear_all(m_Bitmap));
}

PRL_RESULT CSparseBitmap::SetBit(UINT64 pos)
{
	return ToPrlResult(sp_bitmap_set(m_Bitmap, Begin(pos)));
}

bool CSparseBitmap::IsSet(UINT64 pos) const
{
	return sp_bitmap_is_set(m_Bitmap, Begin(pos)) > 0;
}

bool CSparseBitmap::IsSetRange(UINT64 begin, UINT64 end) const
{
	return sp_bitmap_is_set_range(m_Bitmap, End(end), Begin(begin)) > 0;
}

bool CSparseBitmap::IsClearRange(UINT64 begin, UINT64 end) const
{
	return sp_bitmap_is_clear_range(m_Bitmap, End(end), Begin(begin)) > 0;
}

PRL_RESULT CSparseBitmap::Merge(const CSparseBitmap &bitmap, bool use_new_uid)
{
	PRL_RESULT res = ToPrlResult(sp_bitmap_merge(m_Bitmap, bitmap.m_Bitmap));
	if (PRL_FAILED(res))
		return res;

	if (use_new_uid)
		SetUid(bitmap.GetUid());

	return 0;
}

/* This function is used for loading bitmap parts from
 * storages to global cdisk bitmap.
 * In normal case, when granularity == this.GetGranularity() and block_size
 * mod (granularity * 8) == 0, the fast way would be used with memcpy'ing
 * internal bitmap data. Else, there would be slow MERGING of each set bit
 * from source */
PRL_RESULT CSparseBitmap::AssignRange(UINT8 *buf, UINT32 granularity,
		UINT64 begin, UINT64 end)
{
	if (granularity != GetGranularity())
		return PRL_ERR_INVALID_PARAM;

	if (((begin | end) & (GetGranularity() * 8 - 1)) == 0) {
		// Normal way
		return ToPrlResult(sp_bitmap_write_aligned_range(m_Bitmap, buf, End(end), Begin(begin)));
	}

	for (UINT64 bit = begin; bit < end; bit += granularity) {
		if (BMAP_GET(buf, (bit - begin) / granularity))
			SetRange(bit, bit + granularity);
	}

	return PRL_ERR_SUCCESS;
}

PRL_RESULT CSparseBitmap::GetRange(UINT8 *buf, UINT32 granularity,
		UINT64 begin, UINT64 end) const
{
	if (granularity != GetGranularity())
		return PRL_ERR_INVALID_PARAM;

	if (((begin | end) & (GetGranularity() * 8 - 1)) == 0) {
		// Normal way
		return ToPrlResult(sp_bitmap_read_aligned_range(m_Bitmap, buf, End(end), Begin(begin)));
	}

	for (UINT64 bit = begin; bit < end; bit += granularity) {
		if (IsSet(bit)) {
			BMAP_SET(buf, (bit - begin) / granularity);
			BMAP_SET(buf, (bit + granularity - 1 - begin) / granularity);
		}
	}

	return PRL_ERR_SUCCESS;
}

bool CSparseBitmap::IsValid() const
{
	return m_WaitParts == 0;
}

bool CSparseBitmap::IsLoading() const
{
	return m_WaitParts > 0;
}

void CSparseBitmap::PartComplete()
{
	if (m_WaitParts > 0)
		m_WaitParts--;
}

PRL_RESULT CSparseBitmap::Init(UINT64 size, UINT32 granularity,
		const Uuid &uid, UINT32 waitParts)
{
	if (granularity == 0 || (granularity & (granularity -1)) != 0)
		return PRL_ERR_INVALID_ARG;

	m_GranularityBits = BitFindLowestSet(granularity);
	m_Size = size;
	m_Uid = uid;
	m_WaitParts = waitParts;
	m_Bitmap = sp_bitmap_create((m_Size + granularity - 1) >> m_GranularityBits);

	if (m_Bitmap == NULL)
		return PRL_ERR_OUT_OF_MEMORY;

	return PRL_ERR_SUCCESS;
}

PRL_RESULT CSparseBitmap::ToPrlResult(int ret)
{
	switch (-ret) {
	case 0:
		return PRL_ERR_SUCCESS;
	case EINVAL:
		return PRL_ERR_INVALID_ARG;
	case ENOMEM:
		return PRL_ERR_OUT_OF_MEMORY;
	}

	return PRL_ERR_UNEXPECTED;
}

UINT64 CSparseBitmap::End(UINT64 end) const
{
	return ((end - 1) >> m_GranularityBits) + 1;
}

UINT64 CSparseBitmap::Begin(UINT64 begin) const
{
	return begin >> m_GranularityBits;
}

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
#ifndef CSPARCEBITMAP_H
#define CSPARCEBITMAP_H

#include <QScopedPointer>
#include <prlsdk/PrlTypes.h>

class Uuid;
struct sp_bitmap;
class CSparseBitmap {
public:
	/* Constructors and assignment are private, use static Create instead */
	~CSparseBitmap();

	static CSparseBitmap *Create(UINT64 size, UINT32 granularity,
		const Uuid &uid, UINT32 waitParts, PRL_RESULT &err);
	static CSparseBitmap *Create(UINT64 size, UINT32 granularity,
		const Uuid &uid, PRL_RESULT &err);
	static CSparseBitmap *Create(UINT64 size,
		UINT32 granularity, UINT32 waitParts, PRL_RESULT &err);
	static CSparseBitmap *Create(UINT64 size,
		UINT32 granularity, PRL_RESULT &err);
	bool CheckUid(const Uuid &uid) const;
	UINT32 GetGranularity() const;
	const Uuid & GetUid() const;
	void SetUid(const Uuid &uid);
	UINT64 GetSize() const;
	PRL_RESULT SetAll();
	PRL_RESULT SetRange(UINT64 begin, UINT64 end);
	PRL_RESULT ClearRange(UINT64 begin, UINT64 end);
	PRL_RESULT ClearAll();
	PRL_RESULT SetBit(UINT64 pos);
	bool IsSet(UINT64 pos) const;
	bool IsSetRange(UINT64 begin, UINT64 end) const;
	bool IsClearRange(UINT64 begin, UINT64 end) const;
	PRL_RESULT Merge(const CSparseBitmap &bitmap, bool use_new_uid);

	/* This function is used for loading bitmap parts from
	 * storages to global cdisk bitmap.
	 * In normal case, when granularity == this.GetGranularity() and block_size
	 * mod (granularity * 8) == 0, the fast way would be used with memcpy'ing
	 * internal bitmap data. Else, there would be slow MERGING of each set bit
	 * from source */
	PRL_RESULT AssignRange(UINT8 *buf, UINT32 granularity, UINT64 begin, UINT64 end);
	PRL_RESULT GetRange(UINT8 *buf, UINT32 granularity, UINT64 begin, UINT64 end) const;
	bool IsValid() const;
	bool IsLoading() const;
	void PartComplete();

private:
	CSparseBitmap();
	CSparseBitmap(const CSparseBitmap &);
	CSparseBitmap & operator=(const CSparseBitmap &);

	PRL_RESULT Init(UINT64 size, UINT32 granularity, const Uuid &uid, UINT32 waitParts);
	static PRL_RESULT ToPrlResult(int ret);
	UINT64 End(UINT64 end) const;
	UINT64 Begin(UINT64 begin) const;
private:
	sp_bitmap *m_Bitmap;
	UINT32 m_GranularityBits;
	UINT64 m_Size;
	QScopedPointer<Uuid> m_Uid;
	UINT32 m_WaitParts;
};

#endif /* CSPARCEBITMAP_H */

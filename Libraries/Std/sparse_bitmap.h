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
#ifndef _SPARCE_BITMAP_H_
#define _SPARCE_BITMAP_H_

#include "Interfaces/VirtuozzoTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

struct sp_bitmap
{
	UINT32 size;
	UINT32 size_pages;
	UINT64*	data[1];
};

/** Allocate and initialize sparce bitmap, data is zeroed */
struct sp_bitmap *sp_bitmap_create(UINT32 size);

/** Destroy sparce bitmap and release all memory */
void sp_bitmap_destroy(struct sp_bitmap *);

/** Set all bits */
int sp_bitmap_set_all(struct sp_bitmap *bmap);

/** Clear all bits */
int sp_bitmap_clear_all(struct sp_bitmap *bmap);

int sp_bitmap_set(struct sp_bitmap *bmap, UINT32 idx);
int sp_bitmap_clear(struct sp_bitmap *bmap, UINT32 idx);
int sp_bitmap_is_set(struct sp_bitmap *bmap, UINT32 idx);

int sp_bitmap_merge(struct sp_bitmap *dest, struct sp_bitmap *src);

int sp_bitmap_read_aligned_range(struct sp_bitmap *bmap, UINT8 *buf,
		UINT32 bmapSize, UINT32 startBit);
int sp_bitmap_write_aligned_range(struct sp_bitmap *bmap, UINT8 *buf,
		UINT32 bmapSize, UINT32 startBit);
/**
 * @brief
 *	Set all bits in range. Affected bits -
 *	from startBit to bmapSize-1 inclusive.
 *
 * @warning Function return success (zero)
 * if bmapSize is zero.
 *
 * @param bmap	pointer to a sparsed bitmap
 * @param bmapSize	size of a bitmap (in bits!)
 * @param startBit	position to start from (in bits!)
 *
 * @return
 *	\li	0	if success,
 *	\li	-EINVAL	if bmap is NULL, startBit or bmapSize
 *				are greate than whole bitmap size.
 *	\li	-ENOMEM	if no memory for new page allocation.
 */
int sp_bitmap_set_range(struct sp_bitmap *bmap,
		UINT32 bmapSize,
		UINT32 startBit);

/**
 * @brief
 *	Clear all bits in range. Affected bits -
 *	from startBit to bmapSize-1 inclusive.
 *
 * @warning Function return success (zero)
 * if bmapSize is zero.
 *
 * @param bmap	pointer to a sparsed bitmap
 * @param bmapSize	size of a bitmap (in bits!)
 * @param startBit	position to start from (in bits!)
 *
 * @return
 *	\li	0	if success,
 *	\li	-EINVAL	if bmap is NULL, startBit or bmapSize
 *				are greate than whole bitmap size.
 *	\li	-ENOMEM	if no memory for new page allocation.
 */
int sp_bitmap_clear_range(struct sp_bitmap *bmap,
		UINT32 bmapSize,
		UINT32 startBit);

/**
 * @brief
 *	Check are all bits in range are set. Checked bits -
 *	from startBit to bmapSize-1 inclusive.
 *
 * @warning Function return fail (zero)
 * if bmapSize is zero.
 *
 * @param bmap	pointer to a sparsed bitmap
 * @param bmapSize	size of a bitmap (in bits!)
 * @param startBit	position to start from (in bits!)
 *
 * @return
 *	\li	1	if all bits set in range,
 *	\li	0	if at least one bit is cleared,
 *	\li	-EINVAL	if bmap is NULL, startBit or bmapSize
 *				are greate than whole bitmap size.
 */
int sp_bitmap_is_set_range(struct sp_bitmap *bmap,
		UINT32 bmapSize,
		UINT32 startBit);

/**
 * @brief
 *	Check are all bits in range are cleared. Checked bits -
 *	from startBit to bmapSize-1 inclusive.
 *
 * @warning Function return fail (zero)
 * if bmapSize is zero.
 *
 * @param bmap	pointer to a sparsed bitmap
 * @param bmapSize	size of a bitmap (in bits!)
 * @param startBit	position to start from (in bits!)
 *
 * @return
 *	\li	1	if all bits clear in range,
 *	\li	0	if at least one bit is set,
 *	\li	-EINVAL	if bmap is NULL, startBit or bmapSize
 *				are greate than whole bitmap size of
 *				if bmapSize is zero.
 */

int sp_bitmap_is_clear_range(struct sp_bitmap *bmap,
		UINT32 bmapSize,
		UINT32 startBit);


LONG64 sp_bitmap_find_first_clear(struct sp_bitmap *bmap,
		UINT32 bmapSize,
		UINT32 startBit);

LONG64 sp_bitmap_find_last_set(struct sp_bitmap *bmap,
		UINT32 bmapSize,
		UINT32 startBit);

#ifdef __cplusplus
}
#endif

#endif /* _SPARCE_BITMAP_H_ */

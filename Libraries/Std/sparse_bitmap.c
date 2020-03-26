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
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include "Libraries/Std/BitOps.h"
#include <Interfaces/VirtuozzoAlloc.h>
#include "Libraries/Std/sparse_bitmap.h"

#ifndef	PAGE_SHIFT
#	define PAGE_SHIFT	12
#endif

#define SP_BMAP_CLEAR_PAGE	((UINT64*)0u)
#define SP_BMAP_SET_PAGE	((UINT64*)1u)

#define SP_BMAP_SHIFT			(PAGE_SHIFT + 3)
#define SP_BMAP_MASK			((1u << SP_BMAP_SHIFT) - 1)

#define	PAGE_SIZE_BITS			(PAGE_SIZE << 3U)
#define	PAGE_SIZE_64U			(PAGE_SIZE >> 3U)

// idx - bit index inside whole bitmap
#define	PAGE_OFFSET_BITS(idx)	(idx & SP_BMAP_MASK)
#define	PAGE_INDEX(idx)			(idx >> SP_BMAP_SHIFT)

/** Allocate and initialize sparce bitmap, data is zeroed */
struct sp_bitmap *sp_bitmap_create(UINT32 size)
{
	UINT32 entries;
	struct sp_bitmap *bmap;
	UINT32 i;

	if (0 == size)
		return NULL;

	entries = (size + SP_BMAP_MASK) >> SP_BMAP_SHIFT;
	if (0 == entries)	// overflow
		return NULL;

	bmap = malloc(sizeof(struct sp_bitmap) + (entries - 1) * sizeof(UINT32*));
	if (bmap == NULL)
		return NULL;

	bmap->size = size;
	bmap->size_pages = entries;

	for (i = 0; i < entries; i++)
		bmap->data[i] = SP_BMAP_CLEAR_PAGE;

	return bmap;
}

/** Destroy sparce bitmap and release all memory */
void sp_bitmap_destroy(struct sp_bitmap *bmap)
{
	UINT32 i;

	if (bmap == NULL)
		return;

	for (i = 0; i < bmap->size_pages; i++) {
		if (bmap->data[i] == SP_BMAP_CLEAR_PAGE)
			continue;
		if (bmap->data[i] == SP_BMAP_SET_PAGE)
			continue;
		prl_vfree(bmap->data[i]);
	}
	free(bmap);
}

int sp_bitmap_set_all(struct sp_bitmap *bmap)
{
	UINT32 i;

	if (NULL == bmap)
		return -EINVAL;

	for (i = 0; i < bmap->size_pages; i++) {
		if (bmap->data[i] == SP_BMAP_SET_PAGE)
			continue;
		if (bmap->data[i] == SP_BMAP_CLEAR_PAGE) {
			bmap->data[i] = SP_BMAP_SET_PAGE;
			continue;
		}
		// mixed
		prl_vfree(bmap->data[i]);
		bmap->data[i] = SP_BMAP_SET_PAGE;
	}
	return 0;
}

int sp_bitmap_clear_all(struct sp_bitmap *bmap)
{
	UINT32 i;

	if (NULL == bmap)
		return -EINVAL;

	for (i = 0; i < bmap->size_pages; i++) {
		if (bmap->data[i] == SP_BMAP_CLEAR_PAGE)
			continue;
		if (bmap->data[i] == SP_BMAP_SET_PAGE) {
			bmap->data[i] = SP_BMAP_CLEAR_PAGE;
			continue;
		}
		// mixed
		prl_vfree(bmap->data[i]);
		bmap->data[i] = SP_BMAP_CLEAR_PAGE;
	}
	return 0;
}

static int sp_bitmap_alloc_page(struct sp_bitmap *bmap,
									UINT32 page_idx,
									int fill)
{
	UINT64*	page = prl_valloc(PAGE_SIZE);
	if (page == NULL)
		return -ENOMEM;
	//
	memset(page, fill, PAGE_SIZE);
	bmap->data[page_idx] = page;
	return 0;
}

static void sp_bitmap_sparse_page(struct sp_bitmap *bmap,
								 UINT32 page_idx,
								 UINT64* val)
{
	if (bmap->data[page_idx] != val) {
		prl_vfree(bmap->data[page_idx]);
		bmap->data[page_idx] = val;
	}
}

int sp_bitmap_set(struct sp_bitmap *bmap, UINT32 idx)
{
	UINT32 page_idx = PAGE_INDEX(idx);

	if ((bmap == NULL) || (idx >= bmap->size))
		return -EINVAL;

	if (bmap->data[page_idx] == SP_BMAP_SET_PAGE)
		return 0;        /* Page with all 1, nothing to do */

	if (bmap->data[page_idx] == SP_BMAP_CLEAR_PAGE) {
		if (sp_bitmap_alloc_page(bmap, page_idx, 0) != 0)
			return -ENOMEM;

		BMAP_SET(bmap->data[page_idx], idx & SP_BMAP_MASK);
		return 0;
	}

	BMAP_SET(bmap->data[page_idx], idx & SP_BMAP_MASK);
	if (BitFindNextClear64((UINT64*)bmap->data[page_idx], PAGE_SIZE_BITS, 0) >= 0)
		return 0;

	sp_bitmap_sparse_page(bmap, page_idx, SP_BMAP_SET_PAGE);
	return 0;
}

int sp_bitmap_merge(struct sp_bitmap *dest, struct sp_bitmap *src)
{
	UINT32 i;

	if (dest == NULL || src == NULL || dest->size != src->size)
		return -EINVAL;

	for (i = 0; i < dest->size_pages; i++) {
		if (src->data[i] == SP_BMAP_CLEAR_PAGE)
			continue;
		if (dest->data[i] == SP_BMAP_SET_PAGE)
			continue;

		if (src->data[i] == SP_BMAP_SET_PAGE) {
			if (dest->data[i] != SP_BMAP_CLEAR_PAGE)
				prl_vfree(dest->data[i]);

			dest->data[i] = SP_BMAP_SET_PAGE;
			continue;
		}

		if (dest->data[i] == SP_BMAP_CLEAR_PAGE) {
			if (sp_bitmap_alloc_page(dest, i, 0) != 0)
				return -ENOMEM;

			memcpy(dest->data[i], src->data[i], PAGE_SIZE);
		} else {
			BMAP_MERGE(dest->data[i], src->data[i], PAGE_SIZE_BITS);
			if (BitFindNextClear64((UINT64*)dest->data[i], PAGE_SIZE_BITS, 0) < 0)
				sp_bitmap_sparse_page(dest, i, SP_BMAP_SET_PAGE);
		}
	}

	return 0;
}

int sp_bitmap_clear(struct sp_bitmap *bmap, UINT32 idx)
{
	UINT32 page_idx = PAGE_INDEX(idx);

	if ((bmap == NULL) || (idx >= bmap->size))
		return -EINVAL;

	if (bmap->data[page_idx] == SP_BMAP_CLEAR_PAGE)
		return 0;	/* Page with all 0, nothing to do */

	if (bmap->data[page_idx] == SP_BMAP_SET_PAGE) {
		if (sp_bitmap_alloc_page(bmap, page_idx, 0xff) != 0)
			return -ENOMEM;
		BMAP_CLR(bmap->data[page_idx], idx & SP_BMAP_MASK);
		return 0;
	}

	BMAP_CLR(bmap->data[page_idx], idx & SP_BMAP_MASK);
	if (BitFindNextSet64((UINT64*)bmap->data[page_idx], PAGE_SIZE_BITS, 0) >= 0)
		return 0;

	sp_bitmap_sparse_page(bmap, page_idx, SP_BMAP_CLEAR_PAGE);
	return 0;
}

// offs - bit index inside bufer (page)
#define	UINT64_SHIFT			6u
#define	UINT64_BMASK			0x3Fu
#define	UINT64_ENTRY_IDX(offs)	(offs >> UINT64_SHIFT)
#define	UINT64_BIT_IDX(offs)		(offs & UINT64_BMASK)
#define	UINT64_BIT_MASK(offs)		((UINT64)1 << UINT64_BIT_IDX(offs))

int sp_bitmap_is_set(struct sp_bitmap *bmap, UINT32 idx)
{
	UINT32 page_idx = PAGE_INDEX(idx);
	const UINT64* page;
	UINT32	page_offset_bits, entry_idx;
	UINT64	bit_mask, val;

	if ((bmap == NULL) || (idx >= bmap->size))
		return -EINVAL;

	if (bmap->data[page_idx] == SP_BMAP_CLEAR_PAGE)
		return 0;	/* Page with all 0, nothing to do */

	if (bmap->data[page_idx] == SP_BMAP_SET_PAGE)
		return 1;	/* Page with all 1, nothing to do */

	page = (const UINT64*) bmap->data[page_idx];
	page_offset_bits = PAGE_OFFSET_BITS(idx);
	entry_idx = UINT64_ENTRY_IDX(page_offset_bits);
	bit_mask = UINT64_BIT_MASK(page_offset_bits);

	val = page[entry_idx];
	return !!(val & bit_mask);
}

static void fill_range(UINT64* buff,
					   UINT32 sizeBits,
					   UINT32 startBit)
{
	UINT32	idx = BIT2POS64(startBit);
	UINT32	offset = startBit & BIT_MASK_UINT64;
	UINT64	mask;

	sizeBits -= startBit;

	while (sizeBits > 0) {

		UINT32 bitCount = BITS_PER_UINT64;
		if (sizeBits + offset < bitCount)
			bitCount = sizeBits + offset;

		if (0 == offset && BITS_PER_UINT64 == bitCount) {
			buff[idx] = BIT_ALL_SET64;
			goto next;
		}

		mask = (BIT_ALL_SET64 << offset)
				& (BIT_ALL_SET64 >> (BITS_PER_UINT64 - bitCount));
		buff[idx] |= mask;

next:
		idx++;
		sizeBits -= bitCount - offset;
		offset = 0;
	}
}

// buff size is already PAGE_SIZE_BITS
static void clear_range(UINT64 *buff,
						UINT32 sizeBits,
						UINT32 startBit)
{
	UINT32	idx = BIT2POS64(startBit);
	UINT32	offset = startBit & BIT_MASK_UINT64;
	UINT64	mask;

	sizeBits -= startBit;

	while (sizeBits > 0) {

		UINT32 bitCount = BITS_PER_UINT64;
		if (sizeBits + offset < bitCount)
			bitCount = sizeBits + offset;

		if (0 == offset && BITS_PER_UINT64 == bitCount) {
			buff[idx] = ~BIT_ALL_SET64;
			goto next;
		}

		mask = (BIT_ALL_SET64 << offset)
				& (BIT_ALL_SET64 >> (BITS_PER_UINT64 - bitCount));
		buff[idx] &= ~mask;

next:
		idx++;
		sizeBits -= bitCount - offset;
		offset = 0;
	}
}

static int range_sanity_check(struct sp_bitmap *bmap,
							  UINT32 bmapSize,
							  UINT32 startBit)
{
	if (NULL == bmap
		|| startBit >= bmap->size
		|| bmapSize > bmap->size)
		return -1;

	return 0;
}

// Real buff size is already PAGE_SIZE_BITS
static int page_will_set(UINT64 *page,
						 UINT32 sizeBits,
						 UINT32 startBit)
{
	if (0 != startBit && -1 != BitFindNextClear64(page, startBit, 0))
		return 0;
	if (sizeBits != PAGE_SIZE_BITS && -1 != BitFindNextClear64(page, PAGE_SIZE_BITS, sizeBits))
		return 0;
	return 1;
}

static int page_will_clear(UINT64 *page,
						   UINT32 sizeBits,
						   UINT32 startBit)
{
	if (0 != startBit && -1 != BitFindNextSet64(page, startBit, 0))
		return 0;
	if (sizeBits != PAGE_SIZE_BITS && -1 != BitFindNextSet64(page, PAGE_SIZE_BITS, sizeBits))
		return 0;
	return 1;
}

static int write_page(struct sp_bitmap *bmap, UINT32 pageIdx, UINT32 offset, UINT32 size, UINT8 *buf)
{
	UINT64 *page = bmap->data[pageIdx];

	if (page == SP_BMAP_CLEAR_PAGE || page == SP_BMAP_SET_PAGE) {
		if (sp_bitmap_alloc_page(bmap, pageIdx, 0) != 0)
			return -ENOMEM;

		page = bmap->data[pageIdx];
	}

	memcpy((UINT8 *)page + offset, buf, size);

	if (BitFindNextClear64(page, PAGE_SIZE_BITS, 0) < 0)
		sp_bitmap_sparse_page(bmap, pageIdx, SP_BMAP_SET_PAGE);
	else if (BitFindNextSet64(page, PAGE_SIZE_BITS, 0) < 0)
		sp_bitmap_sparse_page(bmap, pageIdx, SP_BMAP_CLEAR_PAGE);

	return 0;
}

static int read_page(struct sp_bitmap *bmap, UINT32 pageIdx, UINT32 offset, UINT32 size, UINT8 *buf)
{
	UINT64 *page = bmap->data[pageIdx];

	if (page == SP_BMAP_CLEAR_PAGE)
		memset(buf, 0, size);
	else if (page == SP_BMAP_SET_PAGE)
		memset(buf, 0xff, size);
	else
		memcpy(buf, (UINT8 *)page + offset, size);

	return 0;
}

int sp_bitmap_rw_aligned_range(struct sp_bitmap *bmap, UINT8 *buf,
							   UINT32 bmapSize, UINT32 startBit,
							   int (*fn)(struct sp_bitmap *, UINT32, UINT32, UINT32, UINT8 *))
{
	UINT32 start, size, off, pageIdx;
	UINT8 *end;

	if (0 != range_sanity_check(bmap, bmapSize, startBit))
		return -EINVAL;

	if (0 == bmapSize)
		return 0;

	// byte-aligned range:
	if ((bmapSize | startBit) & 7)
		return -EINVAL;

	start = startBit >> 3;
	off = start & (PAGE_SIZE - 1);
	size = PAGE_SIZE - off;
	end = buf + ((bmapSize - startBit) >> 3);
	pageIdx = PAGE_INDEX(startBit);

	while (buf < end) {
		int ret;

		if (buf + size > end)
			size = end - buf;

		ret = fn(bmap, pageIdx, off, size, buf);
		if (ret != 0)
			return ret;

		buf += size;
		pageIdx++;
		off = 0;
		size = PAGE_SIZE;
	}

	return 0;
}

int sp_bitmap_read_aligned_range(struct sp_bitmap *bmap, UINT8 *buf,
								 UINT32 bmapSize, UINT32 startBit)
{
	return sp_bitmap_rw_aligned_range(bmap, buf, bmapSize, startBit, read_page);
}

int sp_bitmap_write_aligned_range(struct sp_bitmap *bmap, UINT8 *buf,
								  UINT32 bmapSize, UINT32 startBit)
{
	return sp_bitmap_rw_aligned_range(bmap, buf, bmapSize, startBit, write_page);
}

int sp_bitmap_set_range(struct sp_bitmap *bmap,
						UINT32 bmapSize,
						UINT32 startBit)
{
	UINT32 off = PAGE_OFFSET_BITS(startBit);
	UINT32 pageIdx = PAGE_INDEX(startBit);

	if (0 != range_sanity_check(bmap, bmapSize, startBit))
		return -EINVAL;

	if (0 == bmapSize)
		return 0;

	bmapSize -= startBit;

	while (bmapSize > 0) {
		UINT64 *page = bmap->data[pageIdx];
		UINT32 check = PAGE_SIZE_BITS;

		if (bmapSize + off < check)
			check = bmapSize + off;

		if (page == SP_BMAP_SET_PAGE)
			goto ok;

		// page is clear or mixed
		if (off == 0 && check == PAGE_SIZE_BITS) {
			if (page == SP_BMAP_CLEAR_PAGE) {
				bmap->data[pageIdx] = SP_BMAP_SET_PAGE;
				goto ok;
			}
			// whole mixed page will set
			sp_bitmap_sparse_page(bmap, pageIdx, SP_BMAP_SET_PAGE);
			goto ok;
		}
		// here we process first or last or single page.
		if (page == SP_BMAP_CLEAR_PAGE) {
			if (0 != sp_bitmap_alloc_page(bmap, pageIdx, 0))
				return -ENOMEM;
			page = bmap->data[pageIdx];
		} else if (page_will_set(page, check, off)) {
			sp_bitmap_sparse_page(bmap, pageIdx, SP_BMAP_SET_PAGE);
			goto ok;
		}

		fill_range(page, check, off);

ok:
		bmapSize -= check - off;
		off = 0;
		pageIdx++;
	}
	return 0;
}

int sp_bitmap_clear_range(struct sp_bitmap *bmap,
						  UINT32 bmapSize,
						  UINT32 startBit)
{
	UINT32 off = PAGE_OFFSET_BITS(startBit);
	UINT32 pageIdx = PAGE_INDEX(startBit);

	if (0 != range_sanity_check(bmap, bmapSize, startBit))
		return -EINVAL;

	if (0 == bmapSize)
		return 0;

	bmapSize -= startBit;

	while (bmapSize > 0) {
		UINT64 *page = bmap->data[pageIdx];
		UINT32 check = PAGE_SIZE_BITS;

		if (bmapSize + off < check)
			check = bmapSize + off;

		if (page == SP_BMAP_CLEAR_PAGE)
			goto ok;
		// page is clear or mixed
		if (off == 0 && check == PAGE_SIZE_BITS) {
			if (page == SP_BMAP_SET_PAGE) {
				bmap->data[pageIdx] = SP_BMAP_CLEAR_PAGE;
				goto ok;
			}
			// whole mixed page will set
			sp_bitmap_sparse_page(bmap, pageIdx, SP_BMAP_SET_PAGE);
			goto ok;
		}
		// here we process first or last or single page.
		if (page == SP_BMAP_SET_PAGE) {
			if (0 != sp_bitmap_alloc_page(bmap, pageIdx, 0xff))
				return -ENOMEM;
			page = bmap->data[pageIdx];
		} else if (page_will_clear(page, check, off)) {
			sp_bitmap_sparse_page(bmap, pageIdx, SP_BMAP_CLEAR_PAGE);
			goto ok;
		}
		// clear _check_ bits starting from _off_
		clear_range(page, check, off);

ok:
		bmapSize -= check - off;
		off = 0;
		pageIdx++;
	}
	return 0;
}

int sp_bitmap_is_set_range(struct sp_bitmap *bmap,
							UINT32 bmapSize,
							UINT32 startBit)
{
	UINT32 off = PAGE_OFFSET_BITS(startBit);
	UINT32 pageIdx = PAGE_INDEX(startBit);

	if (0 != range_sanity_check(bmap, bmapSize, startBit))
		return -EINVAL;

	if (0 == bmapSize)
		return 0;

	bmapSize -= startBit;

	while (bmapSize > 0) {
		UINT64 *page = bmap->data[pageIdx];

		UINT32 check = PAGE_SIZE_BITS;
		if (bmapSize + off < check)
			check = bmapSize + off;

		if (page == SP_BMAP_SET_PAGE)
			goto ok;

		if (page == SP_BMAP_CLEAR_PAGE)
			return 0;

		if (off == 0 && check == PAGE_SIZE_BITS)
			return 0;

		if (BitFindNextClear64(page, check, off) != -1)
			return 0;

ok:
		bmapSize -= check - off;
		off = 0;
		pageIdx++;
	}
	return 1;
}

int sp_bitmap_is_clear_range(struct sp_bitmap *bmap,
							 UINT32 bmapSize,
							 UINT32 startBit)
{
	UINT32 off = PAGE_OFFSET_BITS(startBit);
	UINT32 pageIdx = PAGE_INDEX(startBit);

	if (0 != range_sanity_check(bmap, bmapSize, startBit))
		return -EINVAL;

	if (0 == bmapSize)
		return 0;

	bmapSize -= startBit;
	while (bmapSize > 0) {
		UINT64 *page = bmap->data[pageIdx];
		UINT32 check = PAGE_SIZE_BITS;

		if (bmapSize + off < check)
			check = bmapSize + off;

		if (page == SP_BMAP_CLEAR_PAGE)
			goto ok;

		if (page == SP_BMAP_SET_PAGE)
			return 0;

		if (off == 0 && check == PAGE_SIZE_BITS)
			return 0;

		if (BitFindNextSet64(page, check, off) != -1)
			return 0;

ok:
		bmapSize -= check - off;
		off = 0;
		pageIdx++;
	}
	return 1;
}

LONG64 sp_bitmap_find_first_clear(struct sp_bitmap *bmap,
								  UINT32 bmapSize,
								  UINT32 startBit)
{
	UINT32 off = PAGE_OFFSET_BITS(startBit);
	UINT32 pageIdx = PAGE_INDEX(startBit);

	if (0 != range_sanity_check(bmap, bmapSize, startBit))
		return -EINVAL;

	if (0 == bmapSize)
		return -1;

	bmapSize -= startBit;

	while (bmapSize > 0) {
		UINT64 *page = bmap->data[pageIdx];
		UINT32 check = PAGE_SIZE_BITS;
		LONG64 theBit;

		if (bmapSize + off < check) {
			check = bmapSize + off;
		}

		if (page == SP_BMAP_SET_PAGE) {
			bmapSize -= check - off;
			off = 0;
			pageIdx++;
			continue;
		}

		if (page == SP_BMAP_CLEAR_PAGE) {
			return (pageIdx << SP_BMAP_SHIFT);
		}

		theBit = BitFindNextClear64(page, check, off);

		if (theBit != -1) {
			theBit += (pageIdx << SP_BMAP_SHIFT);
		}

		return theBit;

	}

	return -1;
}

#define	PAGE_LAST_BIT(idx)	(((idx + 1) << SP_BMAP_SHIFT) - 1)

LONG64 sp_bitmap_find_last_set(struct sp_bitmap *bmap,
							   UINT32 bmapSize,
							   UINT32 startBit)
{
	UINT32 tailOff = PAGE_OFFSET_BITS(bmapSize);
	UINT32 pageIdx = PAGE_INDEX(bmapSize);
	UINT32 startIdx = PAGE_INDEX(startBit);
	UINT32 headOff = PAGE_OFFSET_BITS(startBit);
	UINT64* page = NULL;
	LONG64 theBit;
	UINT32	size = PAGE_SIZE_BITS;
	UINT32	start = 0;

	if (0 != range_sanity_check(bmap, bmapSize, startBit))
		return -EINVAL;

	if (0 == bmapSize)
		return -1;

	// buffer tail
	if (tailOff != 0) {
		page = bmap->data[pageIdx];
		if (SP_BMAP_SET_PAGE == page) {
			return PAGE_LAST_BIT(pageIdx);
		}

		size = tailOff;

		if (pageIdx == startIdx) {
			goto head;
		}

		if (SP_BMAP_CLEAR_PAGE != page) {
			// [0, tailOff)
			theBit = BitFindLastSet64(page, size, start);
			if (theBit != -1) {
				return theBit + (pageIdx << SP_BMAP_SHIFT);
			}
		}
	}

	// buffer body
	pageIdx--;

	while (pageIdx > startIdx) {
		page = bmap->data[pageIdx];
		if (SP_BMAP_SET_PAGE == page) {
			return PAGE_LAST_BIT(pageIdx);
		}
		if (SP_BMAP_CLEAR_PAGE != page) {
			start = 0;
			size = PAGE_SIZE_BITS;
			goto found;
		}
		pageIdx--;
	}

	// buffer head
	page = bmap->data[pageIdx];

	if (SP_BMAP_SET_PAGE == page) {
		return PAGE_LAST_BIT(pageIdx);
	}
	size = PAGE_SIZE_BITS;

head:
	// if come from buffer tail: size = tailOff
	// if come from body: size = PAGE_SIZE_BITS
	if (SP_BMAP_CLEAR_PAGE == page) {
		return -1;
	}

	start = headOff;

found:
	theBit = BitFindLastSet64(page, size, start);
	if (theBit != -1) {
		theBit += (pageIdx << SP_BMAP_SHIFT);
	}

	return theBit;
}

///////////////////////////////////////////////////////////////////////////////
///
/// @file Pause.h
///
/// @brief CpuPause operation
///
/// @author Denis V. Lunev <den@virtuozzo.com>
///
/// Copyright (c) 2006-2017, Parallels International GmbH
/// Copyright (c) 2017-2019 Virtuozzo International GmbH, All rights reserved.
///
/// This file is part of Virtuozzo Core Libraries. Virtuozzo Core
/// Libraries is free software; you can redistribute it and/or modify it
/// under the terms of the GNU Lesser General Public License as published
/// by the Free Software Foundation; either version 2.1 of the License, or
/// (at your option) any later version.
///
/// This library is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
/// Lesser General Public License for more details.
///
/// You should have received a copy of the GNU Lesser General Public
/// License along with this library.  If not, see
/// <http://www.gnu.org/licenses/> or write to Free Software Foundation,
/// 51 Franklin Street, Fifth Floor Boston, MA 02110, USA.
///
/// Our contact details: Virtuozzo International GmbH, Vordergasse 59, 8200
/// Schaffhausen, Switzerland.
///
///////////////////////////////////////////////////////////////////////////////


#ifndef __PAUSE_H__
#define __PAUSE_H__

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) && !defined(_KERNEL_)
#include <intrin.h>
#pragma intrinsic(_mm_pause)
#endif

static __always_inline void CpuPause(void)
{
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
	_mm_pause();
#elif defined(SUPPORT_ASM_MS)
	__asm pause
#else
	asm volatile("pause": : :"memory");
#endif
}

#endif

/*
 * PrlAssert.h
 *
 * Copyright (c) 1999-2017, Parallels International GmbH
 * Copyright (c) 2017-2019 Virtuozzo International GmbH. All rights reserved.
 *
 * This file is part of Virtuozzo SDK. Virtuozzo SDK is free
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
 * Our contact details: Virtuozzo International GmbH, Vordergasse 59, 8200
 * Schaffhausen, Switzerland; http://www.virtuozzo.com/.
 */

#ifndef H_PrlAssert_H
#define H_PrlAssert_H

#include <QtGlobal>
#include "../Logging/Logging.h"

/************************************************************************/
/*    PRL_ASSERT is Q_ASSERT + log message in RELEASE and DEBUG  mode   */
/*    PRL_ASSERT(x)  ==  WRITE_TRACE( x ) + Q_ASSERT(x)                 */
/*                                                                      */
/*    NOTE: Please, save this functionality PRL_ASSERT   !!!            */
/************************************************************************/

#define PRL_ASSERT( x )    \
     do {  \
		if( !(x) ) \
		{ \
			WRITE_TRACE( DBG_FATAL, "ASSERT( %s ) occured in %s:%d [%s]", #x, __FILE__, __LINE__ , __FUNCTION__ ); \
			Q_ASSERT( #x == NULL ); \
		}\
     }while(0)

#endif

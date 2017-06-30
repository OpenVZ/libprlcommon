#include "Cancellation.h"

///////////////////////////////////////////////////////////////////////////////
///
/// @file Cancellation.cpp
///
/// Primitives for cancellation.
///
/// @author shrike
///
/// Copyright (c) 2005-2017, Parallels International GmbH
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
/// Our contact details: Parallels International GmbH, Vordergasse 59, 8200
/// Schaffhausen, Switzerland.
///
///////////////////////////////////////////////////////////////////////////////

namespace Cancellation
{
///////////////////////////////////////////////////////////////////////////////
// class Sink

Sink::Sink(QMutex& mutex_, QWaitCondition& condition_):
	m_mutex(&mutex_), m_condition(&condition_)
{
}

void Sink::do_()
{
	QMutexLocker g(m_mutex);
	m_condition->wakeAll();
}

///////////////////////////////////////////////////////////////////////////////
// class Token

void Token::signal()
{
	if (0 < m_is.fetchAndStoreAcquire(1))
		return;

	emit cancel();
}

} // namespace Cancellation


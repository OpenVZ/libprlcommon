#ifndef CANCELLATION_H
#define CANCELLATION_H

///////////////////////////////////////////////////////////////////////////////
///
/// @file Cancellation.h
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

#include <QMutex>
#include <QObject>
#include <QAtomicInt>
#include <QWaitCondition>

namespace Cancellation
{
///////////////////////////////////////////////////////////////////////////////
// class Sink

class Sink: public QObject
{
	Q_OBJECT

public:
	Sink(QMutex& , QWaitCondition& );

public slots:
	void do_();

private:
	QMutex* m_mutex;
	QWaitCondition* m_condition;
};

///////////////////////////////////////////////////////////////////////////////
// class Token

class Token: public QObject
{
	Q_OBJECT

public:
	bool isCancelled() const
	{
		return 0 < m_is.operator int();
	}
	void signal();

signals:
	void cancel();

private:
	QAtomicInt m_is;
};

} // namespace Cancellation

#endif // CANCELLATION_H


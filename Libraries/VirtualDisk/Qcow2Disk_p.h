///////////////////////////////////////////////////////////////////////////////
///
/// @file Qcow2Disk_p.h
///
/// VirtualDisk qcow2 private.
///
/// @author shrike
///
/// Copyright (c) 2020 Virtuozzo International GmbH, All rights reserved.
///
/// This file is part of Virtuozzo SDK. Virtuozzo SDK is free
/// software; you can redistribute it and/or modify it under the terms
/// of the GNU General Public License as published by the Free Software
/// Foundation; either version 2 of the License, or (at your option) any
/// later version.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program; if not, write to the Free Software
/// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
/// 02110-1301, USA.
///
/// Our contact details: Virtuozzo International GmbH, Vordergasse 59, 8200
/// Schaffhausen, Switzerland.
///
///////////////////////////////////////////////////////////////////////////////
#ifndef __VIRTUAL_DISK_QCOW2_P__
#define __VIRTUAL_DISK_QCOW2_P__

#include <memory>
#include "Util.h"
#include <QFuture>
#include <QProcess>
#include <QElapsedTimer>
#include <QSharedPointer>
#include <QFutureInterface>
#include <boost/mpl/at.hpp>
#include <boost/mpl/set.hpp>
#include <boost/variant.hpp>
#include <boost/mpl/void.hpp>
#include <prlsdk/PrlErrors.h>
#include "../Logging/Logging.h"
#include "../PrlCommonUtilsBase/SysError.h"

class QFile;
class QTimer;

namespace VirtualDisk
{
namespace Nbd
{
struct Process;
struct Frontend;

typedef QFuture<PRL_RESULT> future_type;
typedef QFutureInterface<PRL_RESULT> feedbackChannel_type;
typedef QSharedPointer<feedbackChannel_type> token_type;

///////////////////////////////////////////////////////////////////////////////
// struct Script

struct Script
{       
	typedef QList<quint32> portList_type;

	// NB. this one is required for Qt metadata
	Script()
	{
	}
	Script(const QString& image_, const portList_type& portList_);

	Script& addArgument(const QString& one_);
	Script& addArguments(const QStringList& bunch_);
	void operator()(Process& target_) const;

private:
	QString m_image;
	QString m_logUuid;
	portList_type m_portList;
	QStringList m_commandLine;
};

namespace Machine
{
///////////////////////////////////////////////////////////////////////////////
// struct Carcass forward declaration

struct Carcass;

} // namespace Machine

namespace State
{
///////////////////////////////////////////////////////////////////////////////
// struct Retired

struct Retired
{
};

///////////////////////////////////////////////////////////////////////////////
// struct Retirable

struct Retirable
{
	Retired retire() const
	{
		return Retired();
	}
};

///////////////////////////////////////////////////////////////////////////////
// struct Terminal

struct Terminal: Retirable
{
	void adopt(const token_type& value_)
	{
		m_token = value_;
	}

protected:
	const token_type& getToken() const
	{
		return m_token;
	}

private:
	token_type m_token;
};

///////////////////////////////////////////////////////////////////////////////
// struct Stopping

struct Stopping: Terminal
{
	typedef QSharedPointer<Machine::Carcass> machine_type;

	explicit Stopping(const machine_type& machine_): m_machine(machine_)
	{
	}

	Retired retire();

private:
	machine_type m_machine;
};

///////////////////////////////////////////////////////////////////////////////
// struct Terminating

struct Terminating: Stopping
{
	template<class T>
	explicit Terminating(const QSharedPointer<T>& machine_): Stopping(machine_)
	{
		if (!machine_.isNull())
			machine_->terminate();
	}
};

///////////////////////////////////////////////////////////////////////////////
// struct Wakeup

struct Wakeup
{
	Wakeup(): m_timer()
	{
	}

	void bind(QObject& target_);
	void cancel();

private:
	QTimer* m_timer;
};

} // namespace State

namespace Visitor
{
///////////////////////////////////////////////////////////////////////////////
// struct Quit

template<class S, class T>
struct Quit: boost::static_visitor<S>
{
	template<class U>
	typename
	boost::enable_if
	<
		boost::is_same<typename boost::mpl::at<T, U>::type, boost::mpl::void_>,
		S
	>::type operator()(U& value_) const
	{
		return S(value_.retire());
	}
	template<class V>
	typename
	boost::disable_if
	<
		boost::is_same<typename boost::mpl::at<T, V>::type, boost::mpl::void_>,
		S
	>::type operator()(const V& value_) const
	{
		WRITE_TRACE(DBG_FATAL, "Unexpected quit event inside the state");
		return value_;
	}
};

///////////////////////////////////////////////////////////////////////////////
// struct Start

template<class T>
struct Start: boost::static_visitor<Prl::Expected<T, PRL_RESULT> >
{
	explicit Start(const T& starting_): m_starting(starting_)
	{
	}

	template<class U>
	Prl::Expected<T, PRL_RESULT> operator()(const U& ) const
	{
		return PRL_ERR_OPERATION_PENDING;
	}
	Prl::Expected<T, PRL_RESULT> operator()(const boost::blank& ) const
	{
		return m_starting;
	}

private:
	T m_starting;
};

} // namespace Visitor

namespace Machine
{
///////////////////////////////////////////////////////////////////////////////
// struct Carcass

struct Carcass: QObject
{
	virtual ~Carcass();

	virtual void terminate() = 0;

public slots:
	virtual void reactFinish(int, QProcess::ExitStatus) = 0;

	virtual void reactStarted() = 0;

	virtual void reactTimeout() = 0;

private:
	Q_OBJECT
};

///////////////////////////////////////////////////////////////////////////////
// struct Flavor forward declaration

template<class T>
struct Flavor;

///////////////////////////////////////////////////////////////////////////////
// struct Generic

template<class T>
struct Generic: Carcass
{
	template<class U>
	Generic(const Script& script_, T& state_, U flavor_);
	~Generic()
	{
		if (NULL != m_engine.get())
			m_engine->disconnect(this);
	}

	void terminate()
	{
		m_flavor.terminate(m_engine);
	}

	void reactFinish(int, QProcess::ExitStatus)
	{
		typedef typename Flavor<T>::unexpectedQuit_type unexpected_type;

		this->terminate();
		m_state = boost::apply_visitor(Visitor::Quit<T, unexpected_type>(), m_state);
	}

	void reactStarted()
	{
		m_state = boost::apply_visitor(m_flavor.reactStarted(), m_state);
	}

	void reactTimeout()
	{
		m_state = boost::apply_visitor(m_flavor.reactTimeout(), m_state);
	}

protected:
	T& m_state;
	Flavor<T> m_flavor;

private:
	std::auto_ptr<QProcess> m_engine;
};

} // namespace Machine

namespace Export
{
struct Machine;
typedef QSharedPointer<Machine> machineCarrier_type;

namespace State
{
///////////////////////////////////////////////////////////////////////////////
// struct Disconnecting

struct Disconnecting: Nbd::State::Terminal
{
	explicit Disconnecting(const machineCarrier_type& machine_): m_machine(machine_)
	{
	}

	Nbd::State::Stopping stop() const;

	Nbd::State::Terminating terminate(PRL_RESULT reason_) const;

private:
	machineCarrier_type m_machine;
};

///////////////////////////////////////////////////////////////////////////////
// struct Running

struct Running: Nbd::State::Retirable
{
	explicit Running(const machineCarrier_type& machine_): m_machine(machine_)
	{
	}

	Disconnecting disconnect() const;

	static PRL_RESULT disconnect(const QString& device_);

private:
	machineCarrier_type m_machine;
};

///////////////////////////////////////////////////////////////////////////////
// struct Steady

struct Steady
{
	Steady(const machineCarrier_type& machine_, const token_type& token_);

	Running run() const;

	Nbd::State::Retired retire();

	Nbd::State::Terminating terminate() const;

	Prl::Expected<size_t, PRL_RESULT> probe();

	Steady retry() const;

private:
	token_type m_token;
	QElapsedTimer m_timer;
	Nbd::State::Wakeup m_retry;
	machineCarrier_type m_machine;
};

///////////////////////////////////////////////////////////////////////////////
// struct Starting

struct Starting: private Steady
{
	Starting(const machineCarrier_type& machine_, const token_type& token_):
		Steady(machine_, token_)
	{
	}

	Steady steady() const
	{
		return *this;
	}

	Nbd::State::Retired retire()
	{
		Steady::terminate();
		return Nbd::State::Retired();
	}
};

} // namespace State

typedef boost::variant
	<
		boost::blank,
		State::Starting,
		State::Steady,
		State::Running,
		State::Disconnecting,
		Nbd::State::Stopping,
		Nbd::State::Terminating,
		Nbd::State::Retired
	> state_type;

namespace Visitor
{
///////////////////////////////////////////////////////////////////////////////
// struct Stop

struct Stop: boost::static_visitor<Prl::Expected<State::Disconnecting, PRL_RESULT> >
{
	explicit Stop(const token_type& token_): m_token(token_)
	{
	}

	template<class U>
	result_type operator()(const U& ) const
	{
		return PRL_ERR_OPERATION_PENDING;
	}

	result_type operator()(const State::Running& value_) const
	{
		State::Disconnecting output(value_.disconnect());
		output.adopt(m_token);

		return output;
	}

private:
	token_type m_token;
};

///////////////////////////////////////////////////////////////////////////////
// struct Started

struct Started: boost::static_visitor<state_type>
{
	template<class T>
	result_type operator()(const T& value_) const
	{
		WRITE_TRACE(DBG_FATAL, "Unexpected started event inside the state");
		return value_;
	}
	result_type operator()(const State::Starting& value_) const;
	result_type visit(State::Steady& value_) const;
};

///////////////////////////////////////////////////////////////////////////////
// struct Timeout

struct Timeout: boost::static_visitor<state_type>
{
	template<class T>
	result_type operator()(const T& value_) const
	{
		WRITE_TRACE(DBG_FATAL, "Unexpected timeout event inside the state");
		return value_;
	}
	result_type operator()(State::Starting& value_) const
	{
		return result_type(value_.retire());
	}
	result_type operator()(State::Steady& value_) const
	{
		return Started().visit(value_);
	}
};

///////////////////////////////////////////////////////////////////////////////
// struct Completed

struct Completed: boost::static_visitor<state_type>
{
	explicit Completed(PRL_RESULT status_): m_status(status_)
	{
	}

	template<class T>
	result_type operator()(const T& value_) const
	{
		WRITE_TRACE(DBG_FATAL, "Unexpected completed event inside the state");
		return value_;
	}
	result_type operator()(State::Disconnecting& value_) const;

private:
	PRL_RESULT m_status;
};

} // namespace Visitor
} // namespace Export

namespace Persistent
{
typedef QSharedPointer<Nbd::Machine::Carcass> machineCarrier_type;

namespace State
{
///////////////////////////////////////////////////////////////////////////////
// struct Running

struct Running: Nbd::State::Retirable
{
	explicit Running(const machineCarrier_type& machine_): m_machine(machine_)
	{
	}

	Nbd::State::Terminating terminate() const
	{
		return Nbd::State::Terminating(m_machine);
	}

private:
	machineCarrier_type m_machine;
};

///////////////////////////////////////////////////////////////////////////////
// struct Linger

struct Linger
{
	Linger(const machineCarrier_type& machine_, const token_type& token_);

	Running expire();

	Nbd::State::Retired retire();

private:
	token_type m_token;
	Nbd::State::Wakeup m_delay;
	machineCarrier_type m_machine;
};

///////////////////////////////////////////////////////////////////////////////
// struct Starting

struct Starting
{
	Starting(const machineCarrier_type& machine_, const token_type& token_):
		m_token(token_), m_machine(machine_)
	{
	}

	Linger linger() const
	{
		return Linger(m_machine, m_token);
	}

	Nbd::State::Retired retire() const;

private:
	token_type m_token;
	machineCarrier_type m_machine;
};

} // namespace State

typedef boost::variant
	<
		boost::blank,
		State::Starting,
		State::Linger,
		State::Running,
		Nbd::State::Terminating,
		Nbd::State::Retired
	> state_type;

namespace Visitor
{
///////////////////////////////////////////////////////////////////////////////
// struct Stop

struct Stop: boost::static_visitor<Prl::Expected<Nbd::State::Terminating, PRL_RESULT> >
{
	explicit Stop(const token_type& token_): m_token(token_)
	{
	}

	template<class U>
	result_type operator()(const U& ) const
	{
		return PRL_ERR_OPERATION_PENDING;
	}

	result_type operator()(const State::Running& value_) const
	{
		Nbd::State::Terminating output(value_.terminate());
		output.adopt(m_token);

		return output;
	}

private:
	token_type m_token;
};

///////////////////////////////////////////////////////////////////////////////
// struct Started

struct Started: boost::static_visitor<state_type>
{
	template<class T>
	result_type operator()(const T& value_) const
	{
		WRITE_TRACE(DBG_FATAL, "Unexpected started event inside the state");
		return value_;
	}
	result_type operator()(const State::Starting& value_) const
	{
		return value_.linger();
	}
};

///////////////////////////////////////////////////////////////////////////////
// struct Timeout

struct Timeout: boost::static_visitor<state_type>
{
	template<class T>
	result_type operator()(const T& value_) const
	{
		WRITE_TRACE(DBG_FATAL, "Unexpected timeout event inside the state");
		return value_;
	}
	result_type operator()(State::Linger& value_) const
	{
		return value_.expire();
	}
	result_type operator()(const State::Starting& value_) const
	{
		return value_.retire();
	}
};

} // namespace Visitor
} // namespace Persistent

namespace Machine
{
///////////////////////////////////////////////////////////////////////////////
// struct Flavor<Persistent::state_type>

template<>
struct Flavor<Persistent::state_type>
{
	typedef boost::mpl::set
		<
			boost::blank,
			Persistent::State::Starting,
			Nbd::State::Retired
		> unexpectedQuit_type;

	void terminate(std::auto_ptr<QProcess>& nbd_);

	Persistent::Visitor::Started reactStarted() const
	{
		return Persistent::Visitor::Started();
	}

	Persistent::Visitor::Timeout reactTimeout() const
	{
		return Persistent::Visitor::Timeout();
	}
};

///////////////////////////////////////////////////////////////////////////////
// struct Flavor<Export::state_type>

template<>
struct Flavor<Export::state_type>
{
	typedef boost::mpl::set
		<
			boost::blank,
			Export::State::Starting,
			Nbd::State::Retired
		> unexpectedQuit_type;

	explicit Flavor(const QSharedPointer<QFile>& guard_): m_guard(guard_)
	{
	}

	void terminate(std::auto_ptr<QProcess>& nbd_);

	Export::Visitor::Started reactStarted() const
	{
		return Export::Visitor::Started();
	}

	Export::Visitor::Timeout reactTimeout() const
	{
		return Export::Visitor::Timeout();
	}

private:
	QSharedPointer<QFile> m_guard;
};

} // namespace Machine

namespace Export
{
///////////////////////////////////////////////////////////////////////////////
// struct Machine

struct Machine: Nbd::Machine::Generic<state_type>
{
	Machine(Script script_, state_type& state_, QSharedPointer<QFile> guard_);

	QString getDevice() const;
	Prl::Expected<size_t, PRL_RESULT> probe() const;

public slots:
	void reactCompleted();

private:
	Q_OBJECT

	QWeakPointer<QFile> m_guard;
};

} // namespace Export

namespace Backend
{
///////////////////////////////////////////////////////////////////////////////
// struct Base

struct Base: QObject
{
	Q_INVOKABLE virtual future_type stop() = 0;
	Q_INVOKABLE virtual future_type start(const Script& script_) = 0;

private:
	Q_OBJECT
};

///////////////////////////////////////////////////////////////////////////////
// struct Flavor forward declaration

template<class T>
struct Flavor;

///////////////////////////////////////////////////////////////////////////////
// struct Generic

template<class T>
struct Generic: Base
{
	template<class U>
	explicit Generic(U flavor_): m_flavor(flavor_)
	{
	}

	future_type stop()
	{
		typedef typename flavor_type::stopVisitor_type visitor_type;

		token_type t(new feedbackChannel_type());
		t->reportStarted();
		typename visitor_type::result_type x =
			boost::apply_visitor(m_flavor.stop(t), m_state);
		if (x.isFailed())
			t->reportFinished(&x.error());
		else
			m_state = x.value();

		return t->future();
	}

	future_type start(const Script& script_)
	{
		typedef typename flavor_type::startVisitor_type visitor_type;

		token_type t(new feedbackChannel_type());
		t->reportStarted();
		typename visitor_type::result_type x =
			boost::apply_visitor(m_flavor.start(script_, t, m_state), m_state);
		if (x.isFailed())
			t->reportFinished(&x.error());
		else
			m_state = x.value();

		return t->future();
	}

private:
	typedef Flavor<T> flavor_type;

	T m_state;
	flavor_type m_flavor;
};

///////////////////////////////////////////////////////////////////////////////
// struct Flavor<Persistent::state_type>

template<>
struct Flavor<Persistent::state_type>
{
	typedef Persistent::state_type state_type;
	typedef Persistent::Visitor::Stop stopVisitor_type;
	typedef Nbd::Visitor::Start<Persistent::State::Starting> startVisitor_type;

	stopVisitor_type stop(const token_type& token_)
	{
		return stopVisitor_type(token_);
	}
	startVisitor_type start(Script script_, const token_type& token_, state_type& state_);
};

///////////////////////////////////////////////////////////////////////////////
// struct Flavor<Export::state_type>

template<>
struct Flavor<Export::state_type>
{
	typedef Export::state_type state_type;
	typedef Export::Visitor::Stop stopVisitor_type;
	typedef Nbd::Visitor::Start<Export::State::Starting> startVisitor_type;

	explicit Flavor(QFile* guard_): m_guard(guard_)
	{
	}

	stopVisitor_type stop(const token_type& token_)
	{
		return stopVisitor_type(token_);
	}
	startVisitor_type start(const Script& script_, const token_type& token_, Export::state_type& state_);

private:
	std::auto_ptr<QFile> m_guard;
};

} // namespace Backend

///////////////////////////////////////////////////////////////////////////////
// struct Frontend

struct Frontend: QObject
{
	Frontend();

	PRL_RESULT stop();
	PRL_RESULT bind(const QString& device_);
	PRL_RESULT start(const Script& script_);

	const QString& getDevice() const
	{
		return m_device;
	}

private:
	QObject* m_nbd;
	QString m_device;
	std::auto_ptr<QFile> m_guard;
};

} // namespace Nbd
} // namespace VirtualDisk

#endif // __VIRTUAL_DISK_QCOW2_P__


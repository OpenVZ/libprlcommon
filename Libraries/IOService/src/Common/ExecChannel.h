/*
 * ExecChannel.h
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
 * Schaffhausen, Switzerland.
 */


////////////////////////////////////////////////////////////////////////////////

#ifndef __EXECCHANNEL_H__
#define __EXECCHANNEL_H__

#include <QObject>
#include <QThread>

#include "../../Interfaces/VirtuozzoTypes.h"
#include "Channel.h"
#include "../../PrlDataSerializer/CPrlStringDataSerializer.h"
#include "../../PrlUuid/Uuid.h"

#include <prlsdk/PrlErrors.h>
#include <prlsdk/PrlEnums.h>
#include <prlsdk/PrlCommandsFlags.h>

using namespace IOService;

namespace IOService {

/**
* Helper class used in transfer operation.
*/
class ExecChannel: public Channel
{
	Q_OBJECT
public:

	ExecChannel(const QString& sessionId,
			const QString& vmId,
			bool isLocal,
			const QString& host,
			quint32 port,
			PRL_SECURITY_LEVEL securityLevel,
			bool useUnixSockets = false );

	virtual ~ExecChannel();

	void init();

	bool sendAuthRequest();
	IOSendJob::Handle sendPackage ( IOPackage::Type,
			const void*, quint32 size );
	IOSendJob::Handle sendPackage ( const SmartPtr<IOPackage> );


private slots:
	/**
	 * Handles state changes from IO client
	 */
	void innerHandleStateChange( IOSender::State );

	/**
	 * Handles package from IO client
	 */
	void innerHandlePackage( const SmartPtr<IOPackage> p );
signals:
	/**
	 * Emits when desktop receives a package.
	 * @note: Signal emission works in _not_ a main thread.
	 *        So be careful if connection to this signal is a DirectConnection.
	 */
	void onPackageReceived ( const SmartPtr<IOPackage> );

	/**
	 * Emits when desktop receives a response package.
	 * @note: Signal emission works in _not_ a main thread.
	 *        So be careful if connection to this signal is a DirectConnection.
	 */
	void onResponsePackageReceived ( IOSendJob::Handle,
			const SmartPtr<IOPackage> );

private:
	QString m_sessionId;
	QString m_vmId;
	volatile bool m_authenticated;
};

};
#endif


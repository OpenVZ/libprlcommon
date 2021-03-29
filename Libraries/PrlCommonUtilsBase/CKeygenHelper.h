///////////////////////////////////////////////////////////////////////////////
///
/// @file CKeygenHelper.h
///
/// Helper to generate ssh-compatiable RSA keys
///
/// @author alexander.alekseev
///
/// Copyright (c) 2021 Virtuozzo International GmbH, All rights reserved.
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

#pragma once

#include "prlsdk/PrlTypes.h"
#include "SysError.h"
#include "ErrorSimple.h"
#include <QString>

class CKeygenHelper
{
public:
	CKeygenHelper() = delete;

	/**
	 * Convert OpenSSH key from file to .pem format
	 *
	 * @return PEM key string or error
	 */
	static Prl::Expected<QString, Error::Simple> GetPemKey(const QFileInfo& openssh_key_file);
private:
	/**
	 * Get ssh-keygen args list to convert public key to .pem format
	 */
	static QStringList GetConvertToPemArgs(const QFileInfo& openssh_key_file);

	/**
	 * Call ssh-keygen with given args and capture stdout
	 *
	 * @param keygen_args Args to pass to ssh-keygen
	 * @param out Reference to QByteArray to store process stdout
	 * @return Sucess or error dependent on process exit code
	 */
	static Error::Simple CallKeygen(const QStringList& keygen_args, QString& out);
};

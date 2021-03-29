///////////////////////////////////////////////////////////////////////////////
///
/// @file CKeygenHelper.cpp
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

#include "CKeygenHelper.h"
#include "prlsdk/PrlErrors.h"
#include "../Interfaces/VirtuozzoQt.h"
#include "../Logging/Logging.h"
#include "../HostUtils/HostUtils.h"
#include <QFile>
#include <QDir>
#include <QStringList>

Prl::Expected<QString, Error::Simple> CKeygenHelper::GetPemKey(const QFileInfo& openssh_key_file)
{
	Error::Simple ret(PRL_ERR_SUCCESS);
	QString pem_key;
	if ((ret = CallKeygen(GetConvertToPemArgs(openssh_key_file), pem_key)).code() != PRL_ERR_SUCCESS)
		return ret;
	return pem_key;
}

QStringList CKeygenHelper::GetConvertToPemArgs(const QFileInfo& openssh_key_file)
{
	return QStringList{
		"-f", openssh_key_file.absoluteFilePath(),
		"-e",
		"-m", "pem"
	};
}

Error::Simple CKeygenHelper::CallKeygen(const QStringList& keygen_args, QString& out)
{
	WRITE_TRACE(DBG_INFO, "Keygen args: %s", QSTR2UTF8(keygen_args.join(" ")));
	if (!HostUtils::RunCmdLineUtility((QStringList{"ssh-keygen"} << keygen_args).join(" "), out))
		return Error::Simple(PRL_ERR_KEYGEN_TOOL_EXECUTED_WITH_ERROR);
	return Error::Simple(PRL_ERR_SUCCESS);
}

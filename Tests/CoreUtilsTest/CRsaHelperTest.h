/////////////////////////////////////////////////////////////////////////////
///
/// Copyright (c) 2021 Virtuozzo International GmbH, All rights reserved.
///
/// This file is part of Virtuozzo Core. Virtuozzo Core is free
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
/// @file
///		CRsaHelperTest.h
///
/// @author
///		alexander.alekseev
///
/// @brief
///		Class for openssl RSA wrapper testing
///
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Libraries/PrlCommonUtilsBase/CRsaHelper.hpp"
#include <QObject>

class CRsaHelperTest: public QObject
{
	Q_OBJECT

private slots:
	void init();
	void cleanup();

private slots:
	void TestCryptographyCycle();
	void TestCryptographyCycleWithFiles();
	void TestIsAuthorized();

private:
	CRsaHelper m_Rsa;
	QString m_Public_Key;
	QString m_Private_Key;
	static const QString S_SAMPLE_STRING;
};

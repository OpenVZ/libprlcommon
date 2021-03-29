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
///		CRsaHelperTest.cpp
///
/// @author
///		alexander.alekseev
///
/// @brief
///		Class for openssl RSA wrapper testing
///
/////////////////////////////////////////////////////////////////////////////

#include "CRsaHelperTest.h"
#include <QtTest/QtTest>

void CRsaHelperTest::init()
{
	m_Rsa = CRsaHelper(QDir::currentPath());

	QFile public_key_file(".vz/keys/id_rsa.pub");
	public_key_file.open(QIODevice::ReadOnly);
	m_Public_Key = public_key_file.readAll();

	QFile private_key_file(".vz/keys/id_rsa");
	private_key_file.open(QIODevice::ReadOnly);
	m_Private_Key = private_key_file.readAll();
}

void CRsaHelperTest::cleanup() {}

const QString CRsaHelperTest::S_SAMPLE_STRING =
	"This is a sample string to verify that cryptography cycle works fine";

void CRsaHelperTest::TestCryptographyCycle()
{
	auto enc = m_Rsa.Encrypt(S_SAMPLE_STRING, m_Public_Key);
	QVERIFY(enc.isSucceed());
	auto dec = m_Rsa.Decrypt(enc.value(), m_Private_Key);
	QVERIFY(dec.isSucceed());
	QVERIFY(dec.value() == S_SAMPLE_STRING);
}

void CRsaHelperTest::TestCryptographyCycleWithFiles()
{
	auto enc = m_Rsa.Encrypt(S_SAMPLE_STRING);
	QVERIFY(enc.isSucceed());
	auto dec = m_Rsa.Decrypt(enc.value());
	QVERIFY(dec.isSucceed());
	QVERIFY(dec.value() == S_SAMPLE_STRING);
}

void CRsaHelperTest::TestIsAuthorized()
{
	QFile authorized(".vz/keys/id_rsa.pub");
	QFile not_authorized(".vz/keys/id_rsa_unauthorized.pub");

	QVERIFY(authorized.open(QIODevice::ReadOnly));
	QVERIFY(not_authorized.open(QIODevice::ReadOnly));

	QVERIFY(m_Rsa.isAuthorized(authorized.readAll()));
	QVERIFY(!m_Rsa.isAuthorized(not_authorized.readAll()));
}

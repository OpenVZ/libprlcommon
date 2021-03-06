/////////////////////////////////////////////////////////////////////////////
///
///	@file IOTCPControlBlockStatTest.h
///
///	This file is the part of Virtuozzo IO service tests suite.
///	Tests ssl helper library
///
///	@author ipasechnik
///
/// Copyright (c) 1999-2017, Parallels International GmbH
/// Copyright (c) 2017-2019 Virtuozzo International GmbH, All rights reserved.
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
/////////////////////////////////////////////////////////////////////////////

#include "SslTest.h"
#include <QTest>

using namespace IOService;

void SslHelperTest::init()
{
	QCOMPARE(SSL_library_init(), 1);
	SSL_load_error_strings();

	SmartPtr<BIGNUM> e = SmartPtr<BIGNUM>(BN_new(), BN_free);
	BN_set_word(e.get(), 65537);
	m_rsa  = RSA_new();
	RSA_generate_key_ex(m_rsa, 2048, e.get(), NULL);
	m_pk  = SmartPtr<EVP_PKEY>(EVP_PKEY_new(), EVP_PKEY_free);
	m_cert  = SmartPtr<X509>(X509_new(), X509_free);

	EVP_PKEY_assign_RSA(m_pk.get(), m_rsa);

	X509_NAME *name = X509_get_subject_name(m_cert.get());

	X509_set_version(m_cert.get(), 3);

	ASN1_INTEGER_set(X509_get_serialNumber(m_cert.get()), 11);

	X509_gmtime_adj(X509_get_notBefore(m_cert.get()), (long) 60 * 60 * 24  *(-2));

	X509_gmtime_adj(X509_get_notAfter(m_cert.get()), (long) 60 * 60 * 24 * 5);

	X509_set_pubkey(m_cert.get(), m_pk.get());

	X509_NAME_add_entry_by_txt(name, "emailAddress" , MBSTRING_ASC,
	                           (const unsigned char*) "testname", -1, -1, 0);

	X509_NAME_add_entry_by_NID(name, NID_role , MBSTRING_ASC,
	                           (unsigned char*) "Server", -1, -1, 0);

	X509_sign(m_cert.get(), m_pk.get(), EVP_sha1());

}

void SslHelperTest::cleanup()
{

}

void SslHelperTest::testCSRCreation()
{
	IOCredentials credentials;

	QVERIFY (!credentials.isValid());
	QVERIFY (generateCredentials("test@test.test", "Server", credentials, 2048, 11, 365));
	QVERIFY (credentials.isValid());

}

void SslHelperTest::testRsaConvertions()
{
	QByteArray initialArr = SSLHelper::RSAToQByteArray(m_rsa);
	SmartPtr<RSA> rsa (SSLHelper::QByteArrayToRSA(initialArr), RSA_free);
	QByteArray finalArr = SSLHelper::RSAToQByteArray(m_rsa);

	QCOMPARE(initialArr, finalArr);

	QCOMPARE(RSA_check_key(m_rsa), 1);

	QCOMPARE(RSA_check_key(rsa.get()), 1);

}

void SslHelperTest::testX509Convertions()
{
	QByteArray initialArr = SSLHelper::X509ToQByteArray(m_cert.get());
	SmartPtr<X509> cert (SSLHelper::QByteArrayToX509(initialArr), X509_free);
	QByteArray finalArr = SSLHelper::X509ToQByteArray(m_cert.get());

	QVERIFY (cert.isValid());

	QCOMPARE(initialArr, finalArr);

	QCOMPARE(X509_subject_name_cmp(m_cert.get(),cert.get()), 0);
}

void SslHelperTest::testX509Role()
{
	IOCertificate certificate;
	certificate.fromByteArray( SSLHelper::X509ToQByteArray(m_cert.get()));

	QCOMPARE(certificate.getRole(), QString("Server"));
}


QTEST_MAIN(SslHelperTest)

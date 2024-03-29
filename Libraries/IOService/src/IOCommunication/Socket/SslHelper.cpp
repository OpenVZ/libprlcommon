/*
 * SslHelper.cpp
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


#include "SslHelper.h"
#include <Libraries/OpenSSL/OpenSSL.h>
#include <Libraries/Logging/Logging.h>

// OpenSSL headers
#ifndef OPENSSL_NO_ENGINE
#include <openssl/engine.h>
#endif
#include <openssl/err.h>
#include <openssl/rand.h>

#include <QStringList>

namespace IOService {

const char* SSLHelper::s_serverSessionIdContext = "VirtuozzoServer";

/* These DH parameters have been generated as follows:
 *    $ openssl dhparam -C -noout 512
 *    $ openssl dhparam -C -noout 1024
 *    $ openssl dhparam -C -noout -dsaparam 1024
 * (The third function has been renamed to avoid name conflicts.)
 */

static DH * get_dhXXX(unsigned char * dhXXX_p, int pLen, unsigned char * dhXXX_g, int gLen)
{
	BIGNUM *p, *g;
	DH* dh = DH_new();

	if (dh == NULL)
		return NULL;

	p = BN_bin2bn(dhXXX_p, pLen, NULL);
	g = BN_bin2bn(dhXXX_g, gLen, NULL);

	if (p == NULL || g == NULL) {
		DH_free(dh);
		BN_free(p);
		BN_free(g);
		return NULL;
	}

#if  OPENSSL_VERSION_NUMBER >= 0x10100000L
	DH_set0_pqg(dh, p, NULL, g);
#else
	dh->p = p;
	dh->g = g;
#endif

	return dh;
}

static DH * get_dh2048()
{
		static unsigned char dh2048_p[]={
				0x9C,0x34,0xDA,0xEF,0xEE,0xC1,0x73,0x98,0xAE,0xEE,0x4E,0xFB,
				0xB8,0x61,0x85,0x04,0x57,0x11,0xCD,0xB7,0xBC,0x74,0xC9,0x95,
				0xB1,0xF5,0x7D,0x81,0x7D,0x9F,0xA8,0xDE,0x85,0xFE,0x1E,0x4F,
				0x04,0xFD,0x84,0xC8,0x27,0xD3,0x8D,0x57,0xAD,0x04,0xC9,0xF9,
				0x5B,0x65,0x9D,0xAD,0x2B,0xF0,0x51,0xDB,0xA5,0x46,0xE2,0x59,
				0x7A,0x49,0x29,0xBB,0xB0,0x72,0x13,0xD2,0xF6,0xDF,0xCD,0x5B,
				0x2A,0xFD,0xEB,0x08,0x1C,0x77,0x97,0xCE,0x9C,0x31,0x4C,0xE2,
				0x04,0x2D,0x25,0x1F,0xB5,0x0D,0xAC,0xE3,0xE8,0x60,0x0A,0xEC,
				0xD4,0xB7,0x9B,0x4E,0xCD,0x9E,0xFF,0x1A,0x69,0xF0,0x78,0xE7,
				0x7B,0x65,0xAE,0xAF,0x08,0x0D,0xB6,0x15,0x9C,0x77,0x05,0xD9,
				0xF6,0xAF,0x93,0x38,0x64,0x17,0x8F,0x2C,0xAF,0x79,0xEE,0x20,
				0x3C,0x4C,0xFC,0x82,0xA4,0x3F,0x99,0xBB,0xD8,0xB3,0xA1,0xCF,
				0x11,0x55,0x0C,0x3D,0xF9,0x8C,0x5A,0x20,0x69,0xEC,0x68,0x28,
				0x23,0x5B,0x6F,0x77,0xD3,0x8E,0x97,0xEC,0x46,0x85,0xB4,0x0C,
				0x4F,0x87,0xF4,0x8D,0x04,0x7E,0x48,0x10,0x9B,0x1B,0xDB,0x58,
				0xE8,0xC0,0x69,0xCE,0xF8,0xAA,0xA1,0x2C,0x17,0x04,0x45,0xA7,
				0xA8,0x84,0x95,0xBC,0xDC,0xFD,0xF3,0x97,0x60,0x3E,0x07,0xA4,
				0xBA,0x4D,0x6F,0xB1,0x50,0x92,0x16,0x0D,0x80,0xA9,0xF6,0x01,
				0xE1,0x55,0x83,0x4A,0x43,0x6E,0x3D,0x4B,0x08,0x0D,0x24,0xCF,
				0xD7,0x18,0x6B,0x05,0xDD,0x26,0x77,0x81,0xA6,0xE8,0x3C,0xD7,
				0x2C,0xA0,0xE9,0x32,0xD8,0xCB,0x86,0xD7,0x90,0xA0,0x6F,0xD1,
				0x6F,0x90,0x23,0x73,
				};
		static unsigned char dh2048_g[]={
				0x02,
				};
	return get_dhXXX(dh2048_p, sizeof(dh2048_p), dh2048_g, sizeof(dh2048_g));
}

static DH * get_dh4096()
{
		static unsigned char dh4096_p[]={
				0xBD,0x24,0xCE,0xF5,0xA7,0x38,0x56,0x92,0x1A,0x78,0xFF,0xF0,
				0x5B,0xC3,0x5B,0x46,0x99,0x90,0x9C,0x4E,0xA0,0xB9,0xE0,0x4E,
				0x38,0x82,0xBE,0xBC,0xBE,0x13,0xDD,0xAF,0x1A,0xFF,0x8A,0x9F,
				0xF7,0xC5,0x52,0x52,0x0A,0xD8,0x58,0x0E,0x3E,0x1F,0x15,0x23,
				0xDE,0xD3,0x27,0xBB,0x67,0x68,0x77,0x80,0xBA,0x09,0x4C,0xE5,
				0x0A,0xC7,0x24,0xB4,0xD1,0x8F,0x8D,0xA9,0x3D,0x69,0x41,0x27,
				0x7A,0x5B,0x24,0x58,0xF8,0x1C,0x8C,0xEF,0x09,0x4F,0xC5,0xFB,
				0x16,0xDE,0x33,0x8A,0x75,0x3E,0x69,0x56,0xFC,0xE5,0x21,0x34,
				0xD7,0x6A,0x73,0xA7,0x65,0x6A,0x77,0x1D,0x3B,0xF1,0x6B,0x69,
				0x9C,0x60,0x15,0x59,0x25,0xB1,0x0B,0x4D,0xF7,0x42,0x89,0xF3,
				0xA8,0xF4,0x61,0xDC,0x1E,0x1E,0xD5,0x0A,0xD3,0xC9,0x29,0x8D,
				0x7E,0x9D,0xC8,0xA5,0x76,0x55,0x2D,0x23,0x9F,0x4D,0x3C,0x61,
				0x52,0x3C,0x26,0xE6,0x98,0x75,0xDF,0x68,0x20,0xD4,0x34,0xB0,
				0xF2,0x03,0xF7,0x4A,0xD1,0x2E,0xCA,0xF0,0x7D,0x3E,0xBA,0xFA,
				0x20,0x81,0xF2,0x74,0x14,0x9C,0x96,0xB5,0x9E,0xFB,0xE7,0xFB,
				0x9C,0x3B,0xC9,0x1C,0x27,0x55,0x44,0x0E,0x9D,0x72,0x8D,0xE3,
				0x96,0x6E,0x1B,0xA6,0xD3,0xB8,0x16,0x6E,0x2A,0x74,0xEE,0x41,
				0x62,0x38,0xBE,0xEF,0xAE,0x6D,0x42,0xCE,0x3C,0x19,0xD8,0x3F,
				0x25,0x61,0x7B,0x38,0x01,0x10,0xAA,0xF7,0x3C,0xD6,0x08,0x98,
				0x3E,0xE1,0xE1,0x34,0x52,0x4C,0xD0,0x51,0xC6,0x75,0x38,0x59,
				0x01,0x69,0x69,0x47,0x72,0x19,0x83,0xA4,0x5A,0x30,0x46,0x6D,
				0xCD,0xDE,0xD7,0x44,0x90,0xCF,0xF9,0x9D,0x09,0x50,0x41,0x5E,
				0x22,0xFF,0x19,0xF8,0x79,0xD3,0x82,0x40,0x06,0x30,0x5B,0x36,
				0x5D,0x9F,0x2A,0x7D,0x90,0x71,0x8F,0xBB,0x81,0xDD,0x7A,0x6A,
				0xD9,0xF2,0x1C,0x85,0x2E,0xC5,0x30,0x3D,0x8D,0x45,0xF9,0xB7,
				0xE1,0x82,0xFF,0x1A,0x29,0x01,0x62,0x22,0x1A,0x0D,0xAF,0x74,
				0x9C,0xFC,0x89,0x49,0xF1,0x0A,0x62,0xBC,0xCD,0xEB,0xDB,0x3B,
				0xF0,0xEA,0xDB,0x5C,0x54,0x8B,0x2F,0x08,0x01,0xAE,0xD5,0xF1,
				0x8A,0x49,0xBE,0x5B,0xA9,0x6B,0x1F,0xC5,0xD2,0x94,0xEC,0xF7,
				0x5B,0x2A,0xFD,0x28,0xF8,0x44,0xAD,0xCB,0xC1,0x81,0xC0,0x75,
				0xB3,0xE1,0xA4,0xA0,0x9F,0xB1,0x93,0xAD,0xAC,0xC0,0xBB,0x67,
				0x25,0x3F,0x33,0xD2,0x18,0x50,0x38,0x11,0xF0,0x71,0x48,0xEC,
				0x87,0x2B,0x33,0xFC,0x1B,0x34,0x28,0x66,0xAF,0x9A,0xC8,0x7E,
				0x02,0xA3,0xA1,0x45,0xBF,0xB2,0xCF,0x51,0xA9,0xA1,0xDF,0x9F,
				0xDB,0xD8,0xA8,0x41,0x9F,0x1D,0x4F,0x85,0xC5,0xE9,0xF4,0x4F,
				0x1D,0x2F,0x5F,0xB8,0x55,0x3E,0x40,0xEE,0xAF,0x7F,0x51,0x04,
				0xCB,0x61,0x2A,0xDA,0xD4,0x6F,0xB6,0xD1,0x1D,0xD5,0x57,0x95,
				0x04,0xB5,0x10,0xE1,0x37,0x79,0xE1,0x28,0x0B,0x36,0x0F,0xC0,
				0xCD,0xDA,0x36,0xCF,0x38,0x17,0xAC,0x15,0x37,0xA7,0xEF,0x98,
				0xFB,0xA6,0xC6,0x2D,0x20,0x77,0x72,0xC7,0xD0,0xE1,0xA9,0xFE,
				0x59,0x68,0x0F,0xBA,0x12,0x7F,0x45,0xF5,0x0F,0xD5,0xCE,0xFC,
				0x4D,0xF3,0xDC,0x4F,0x23,0xDF,0x0B,0x40,0x81,0x69,0x0D,0x1F,
				0xF1,0x78,0x3D,0x3A,0xAB,0x43,0xA2,0xE3,
				};
		static unsigned char dh4096_g[]={
				0x02,
				};
	return get_dhXXX(dh4096_p, sizeof(dh4096_p), dh4096_g, sizeof(dh4096_g));
}

typedef QHash< int, SmartPtr<DH> > DHMap;
Q_GLOBAL_STATIC(QMutex, dhCallbackMutex)
Q_GLOBAL_STATIC(DHMap, dhCallbackMap)

DH* SSLHelper::DHCallback ( SSL*, int, int keyLength )
{
	// Lock
	QMutexLocker locker( dhCallbackMutex() );

	DHMap* dhMap = dhCallbackMap();

	if ( dhMap->contains(keyLength) ) {
		SmartPtr<DH> dhPtr = dhMap->value(keyLength);
		return dhPtr.getImpl();
	}

	DH* dh = 0;
	switch( keyLength ) {
	case 2048:
		dh = get_dh2048();
		break;
	case 4096:
		dh = get_dh4096();
		break;
	}
	if ( dh == 0 )
	{
		dh = DH_new();
		DH_generate_parameters_ex(dh, keyLength, DH_GENERATOR_2, NULL);
	}
	if ( dh != 0 )
		dhMap->insert(keyLength, SmartPtr<DH>(dh, DH_free));

	return dh;
}

bool SSLHelper::InitSSLContext(const IOCredentials& credentials)
{
	const size_t ErrBuffSize = 256;
	char errBuff[ ErrBuffSize ];

	// Create client context
	{
		s_clientSSLCtx = SSL_CTX_new( TLS_client_method() );
		if ( ! s_clientSSLCtx ) {
			ERR_error_string_n( ERR_get_error(), errBuff, ErrBuffSize );
			WRITE_TRACE(DBG_FATAL,
			            "Can't create client SSL context (SSL error: %s)",
			            errBuff);
			goto error;
		}

		SSL_CTX_set_quiet_shutdown(s_clientSSLCtx, 1);

		// Setup anonymous DH cipher
#if OPENSSL_VERSION_NUMBER < 0x10100000L
		if (credentials.isValid()) {
			SSL_CTX_set_cipher_list(s_clientSSLCtx, "RSA:ADH:!eNULL:@STRENGTH");
		}else {
			SSL_CTX_set_cipher_list(s_clientSSLCtx, "ADH:!eNULL:@STRENGTH");
		}
#else
		if (credentials.isValid()) {
			SSL_CTX_set_cipher_list(s_clientSSLCtx, "RSA:ADH:!eNULL:@STRENGTH:@SECLEVEL=0");
		}else {
			SSL_CTX_set_cipher_list(s_clientSSLCtx, "ADH:!eNULL:@STRENGTH:@SECLEVEL=0");
		}
#endif
	}

	// Create server context
	{
		s_serverSSLCtx = SSL_CTX_new( TLS_server_method() );
		if ( ! s_serverSSLCtx ) {
			ERR_error_string_n( ERR_get_error(), errBuff, ErrBuffSize );
			WRITE_TRACE(DBG_FATAL,
			            "Can't create server SSL context (SSL error: %s)",
			            errBuff);
			goto error;
		}

		SSL_CTX_set_quiet_shutdown(s_serverSSLCtx, 1);

		// Setup anonymous DH cipher
#if OPENSSL_VERSION_NUMBER < 0x10100000L
		if (credentials.isValid())
			SSL_CTX_set_cipher_list(s_serverSSLCtx, "RSA:ADH:!eNULL:@STRENGTH");
		else
			SSL_CTX_set_cipher_list(s_serverSSLCtx, "ADH:!eNULL:@STRENGTH");
#else
		if (credentials.isValid())
			SSL_CTX_set_cipher_list(s_serverSSLCtx, "RSA:ADH:!eNULL:@STRENGTH:@SECLEVEL=0");
		else
			SSL_CTX_set_cipher_list(s_serverSSLCtx, "ADH:!eNULL:@STRENGTH:@SECLEVEL=0");
#endif

		SSL_CTX_set_tmp_dh_callback(s_serverSSLCtx, DHCallback);
		SSL_CTX_set_session_id_context(
		            s_serverSSLCtx,
		            reinterpret_cast<const unsigned char*>(
		                s_serverSessionIdContext),
		            (unsigned)::strlen(s_serverSessionIdContext));
	}

	 //Init context with local credentials
	if ( credentials.isValid() && ( !SetLocalCredentials(credentials) )) {

		ERR_error_string_n( ERR_get_error(), errBuff, ErrBuffSize );
		WRITE_TRACE(DBG_FATAL,
		            "Can't init SSL context (SSL error: %s)",
		            errBuff);
		goto error;
	}

	return true;

error:
   DeinitSSLContext();
   return false;
}

void SSLHelper::DeinitSSLContext()
{
	if (s_localCert)
		X509_free(s_localCert);
	s_localCert = 0;

	if ( s_serverSSLCtx )
		SSL_CTX_free( s_serverSSLCtx );
	if ( s_clientSSLCtx )
		SSL_CTX_free( s_clientSSLCtx );
	s_serverSSLCtx = 0;
	s_clientSSLCtx = 0;
}

SSL_CTX* SSLHelper::GetClientSSLContext ()
{
	Q_ASSERT(s_clientSSLCtx);
	return s_clientSSLCtx;
}

SSL_CTX* SSLHelper::GetServerSSLContext ()
{
	Q_ASSERT(s_serverSSLCtx);
	return s_serverSSLCtx;
}

X509 * SSLHelper::QByteArrayToX509(const QByteArray& pem)
{
	return OpenSSL::ArrayToX509((void*)pem.data(), pem.size());
}

QByteArray SSLHelper::RSAToQByteArray(RSA * rsa)
{
	SmartPtr<BIO> bio  (BIO_new(BIO_s_mem()), BIO_free) ;

	if (!bio.isValid())
		return QByteArray();

	if (!PEM_write_bio_RSAPrivateKey(bio.get(), rsa, (const EVP_CIPHER *)0, NULL, 0, 0, 0))
		return QByteArray();

	char *data;

	long size = BIO_get_mem_data(bio.get(), &data);

	return QByteArray(data, size);
}

RSA * SSLHelper::QByteArrayToRSA(const QByteArray& byteArray)
{
	return OpenSSL::ArrayToRSA((void*)byteArray.data(), byteArray.size());
}

QByteArray SSLHelper::X509_REQToQByteArray(X509_REQ * req)
{
	SmartPtr<BIO > out (BIO_new(BIO_s_mem()), BIO_free);

	if (!out.isValid())
		return QByteArray();

	if (!PEM_write_bio_X509_REQ(out.get(), req))
		return QByteArray();

	char *data;

	long size = BIO_get_mem_data(out.get(), &data);

	return  QByteArray(data, size);
}

QByteArray SSLHelper::X509ToQByteArray(X509 * x)
{
	SmartPtr<BIO > out( BIO_new(BIO_s_mem()), BIO_free);

	if (!out.isValid())
		return QByteArray();

	if (!PEM_write_bio_X509(out.get(), x))
		return QByteArray();

	char *data;

	long size = BIO_get_mem_data(out.get(), &data);

	return  QByteArray(data, size);
}

int verify_callback(int preverify_ok, X509_STORE_CTX *ctx)
{
	if (preverify_ok)
		return preverify_ok;

	char   incomingCertDescription[256] = {0};
	int     err, depth;

	X509* err_cert = X509_STORE_CTX_get_current_cert(ctx);
	err = X509_STORE_CTX_get_error(ctx);
	depth = X509_STORE_CTX_get_error_depth(ctx);

	X509_STORE_CTX_get_ex_data(ctx, SSL_get_ex_data_X509_STORE_CTX_idx());
	X509_NAME_oneline(X509_get_subject_name(err_cert), incomingCertDescription, (int)sizeof(incomingCertDescription));

	Q_ASSERT(err);

	WRITE_TRACE(DBG_FATAL, "Verify error :num=%d:%s:depth=%d:%s\n",
		err, X509_verify_cert_error_string(err), depth, incomingCertDescription);

	if (err == X509_V_ERR_CERT_NOT_YET_VALID)
		return 1;

	return 0; // error
}

bool SSLHelper::SSL_CTX_set_credentials(SSL_CTX* ctx, const IOCredentials& cert)
{
	SmartPtr< X509> peerCert (QByteArrayToX509(cert.certificate.toByteArray()), X509_free);

	if (!peerCert.isValid())
		return false;

	if (!SSL_CTX_use_certificate(ctx, peerCert.get()))
		return false;

	SmartPtr< RSA> rsa(QByteArrayToRSA(cert.privKey), RSA_free);

	if (!rsa.isValid())
		return false;

	if (!SSL_CTX_use_RSAPrivateKey(ctx, rsa.get()))
		return false;

	foreach(const IOCertificate& chaninCertificate, cert.certificatesChain)
	{
		X509* chainCert =  QByteArrayToX509(chaninCertificate.toByteArray());

		if (!chainCert)
			return false;

		if (SSL_CTX_add_extra_chain_cert(ctx, chainCert) != 1)
		{
			return false;
		}
	}
	return true;
}

bool SSLHelper::postHandshakeCheck(SSL *ssl, bool rehandshake)
{
	QString cipher = GetConnectionChipher(ssl);

	if (!isValidCipher(cipher))
		return false;

	if (!isNeedPostConnectionCheck(cipher) || rehandshake)
	{
		return true;
	}

	WRITE_TRACE(DBG_FATAL, "Perfoming post connection check \n");
	QByteArray incomingCertString(256, 0);
	QByteArray localCertString(256, 0);

	SmartPtr< X509> pnt ( SSL_get_peer_certificate(ssl), X509_free);

	if (!pnt.isValid()) {
		WRITE_TRACE(DBG_FATAL, "Unable to get peer certifiate\n");
		return false;
	}

	if (!SSLHelper::s_localCert)
	{
		WRITE_TRACE(DBG_FATAL, "Unable to get local certificate\n");
		return false;
	}

	X509_NAME* peerSubjectName = X509_get_subject_name(pnt.get());
	X509_NAME* localSubjectName = X509_get_subject_name(SSLHelper::s_localCert);

	if (!localSubjectName || !peerSubjectName)
	{
		WRITE_TRACE(DBG_FATAL, "Post connection check failed: missing subject attributes \n");
		return false;
	}

	X509_NAME_oneline(peerSubjectName, incomingCertString.data(), incomingCertString.size());
	X509_NAME_oneline(localSubjectName, localCertString.data(), localCertString.size());

	WRITE_TRACE(DBG_DEBUG, "Peer Certificate: %s\n", incomingCertString.constData());
	WRITE_TRACE(DBG_DEBUG, "Local certificate: %s\n", localCertString.constData());

	int localNameIndex(X509_NAME_get_index_by_NID(localSubjectName, NID_commonName, -1));
	int peerNameIndex(X509_NAME_get_index_by_NID(peerSubjectName, NID_commonName, -1));

	if ((localNameIndex == -1) || (peerNameIndex == -1)) {
		WRITE_TRACE(DBG_INFO, "Post connection check failed: missing attributes");
		return false;
	}

	X509_NAME_ENTRY * localNameEntry = X509_NAME_get_entry(localSubjectName, localNameIndex);

	X509_NAME_ENTRY * peerNameEntry = X509_NAME_get_entry(peerSubjectName, peerNameIndex);

	if (!localNameEntry || !peerNameEntry) {
		WRITE_TRACE(DBG_INFO, "Post connection check failed: missing attributes");
		return false;
	}

	ASN1_STRING * localNameAsn1String = X509_NAME_ENTRY_get_data(localNameEntry);

	ASN1_STRING * peerNameAsn1String = X509_NAME_ENTRY_get_data(peerNameEntry);

	if (!localNameAsn1String || !peerNameAsn1String) {
		WRITE_TRACE(DBG_INFO, "Post connection check failed: Non valid certificate");
		return false;
	}

	if (ASN1_STRING_cmp(localNameAsn1String, peerNameAsn1String) != 0) {
		WRITE_TRACE(DBG_INFO, "Post connection check failed: Subject names mistmatching");
		return false;
	}

	return true;
}

bool SSLHelper::SetLocalCredentials(const IOCredentials& credentials)
{
	WRITE_TRACE(DBG_INFO, "Perfoming credentials setting");

	if (!SSL_CTX_set_credentials(s_clientSSLCtx, credentials))
		return false;

	if (!SSL_CTX_set_credentials(s_serverSSLCtx, credentials))
		return false;

	s_localCert = QByteArrayToX509(credentials.certificate.toByteArray());

	return (s_localCert);
}

bool SSLHelper::verifyCertificate(const IOCredentials& credentials, const QByteArray& caCertificate)
{
	SmartPtr< X509> endpointCert (QByteArrayToX509(credentials.certificate.toByteArray()), X509_free);

	if (!endpointCert.isValid())
		return false;

	SmartPtr< X509> rootCert;

	rootCert = SmartPtr< X509> (QByteArrayToX509( caCertificate), X509_free);

	SmartPtr< X509> topLevelChainCert = endpointCert;

	foreach( const IOCertificate& cert, credentials.certificatesChain)
	{
		SmartPtr< X509> tempCert (QByteArrayToX509(cert.toByteArray()), X509_free);

		if (!tempCert.isValid())
			return false;

		if (X509_verify(topLevelChainCert.get(), X509_get_pubkey(tempCert.get())) == 1)
		{
			topLevelChainCert = tempCert;
		}
		else
		{
			return false;
		}
	}


	if (!rootCert.isValid())
		return false;

	return  (X509_verify(topLevelChainCert.get(), X509_get_pubkey(rootCert.get())) == 1);
}

bool SSLHelper::isValidCipher(const QString& cipherName)
{
	return  ((cipherName == SSL_TXT_DH) || (cipherName == SSL_TXT_RSA));
}

bool SSLHelper::isNeedPostConnectionCheck(const QString& cipherName)
{
	return  (cipherName == SSL_TXT_RSA );
}

bool SSLHelper::isTrustedChannelCipher(const QString& cipherName)
{
	return  (cipherName == SSL_TXT_RSA);
}

QString SSLHelper::GetConnectionChipher(SSL* ssl)
{
	char buff[256];

	const char* desc = SSL_CIPHER_description(SSL_get_current_cipher(ssl), buff, sizeof(buff));

	if (!desc)
		return QString();

	QStringList cipherList = QString(desc).simplified ().split(" ");

	if (cipherList.size() < SSLCipherDescriptionSize)
		return QString();

	QString Kx = cipherList[SSLKeyExchangePosition];

	WRITE_TRACE(DBG_DEBUG, "Key exchange :%s", qPrintable(Kx));

	QStringList kxList = Kx.split("=");

	if (kxList.size() < 2)
		return QString();

	return kxList[1];
}

void SSLHelper::printCiphers(SSL* ssl)
{
	int prio(0);
	do{
		const char *name = SSL_get_cipher_list(ssl, prio);
		if (!name)
			break;

		++ prio;
		WRITE_TRACE(DBG_INFO, "Cipher :%s", name);
	}while(true);
}

} // IOService

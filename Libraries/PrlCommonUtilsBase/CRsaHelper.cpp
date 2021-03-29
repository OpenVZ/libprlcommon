///////////////////////////////////////////////////////////////////////////////
///
/// @file CRsaHelper.cpp
///
/// Openssl RSA cryptography wrapper
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

#include <type_traits>
#include "CRsaHelper.hpp"
#include "../Logging/Logging.h"
#include <openssl/rand.h>
#include <openssl/conf.h>
#include <openssl/pem.h>


CRsaHelper::CRsaHelper(const QDir& home /*= CFileHelper::homePath()*/) :
	m_RsaPrivateKeyFile(home.filePath(".vz/keys/id_rsa")),
	m_RsaPublicKeyFile(home.filePath(".vz/keys/id_rsa.pub")),
	m_AuthorizedKeysFile(home.filePath(".vz/keys/authorized_keys")) {}

Prl::Expected<QString, Error::Simple> CRsaHelper::getOpenSshPublicKey()
{
	QFile file(m_RsaPublicKeyFile.absoluteFilePath());
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return Error::Simple(PRL_ERR_READ_FAILED);
	return QString(file.readAll()).trimmed();
}

Prl::Expected<QString, Error::Simple>
CRsaHelper::Encrypt(const QString& data)
{
	return Encrypt(data, m_RsaPublicKeyFile);
}

Prl::Expected<QString, Error::Simple>
CRsaHelper::EncryptWithPemKey(const QString& data, const QString& pem_key)
{
	return RsaEncryptor(pem_key)(data);
}

Prl::Expected<QString, Error::Simple> CRsaHelper::Decrypt(const QString& data)
{
	return Decrypt(data, m_RsaPrivateKeyFile);
}

bool CRsaHelper::isAuthorized(const QString& public_key)
{
	// return false also in case file could not be read.
	QFile authorized_keys(m_AuthorizedKeysFile.absoluteFilePath());
	if (!authorized_keys.open(QIODevice::ReadOnly))
		return false;
	return !public_key.isEmpty() &&
		   authorized_keys.readAll().split('\n').contains(public_key.trimmed().toUtf8());
}

Prl::Expected<QFileInfo, Error::Simple>
CRsaHelper::WriteKey(const QString& key)
{
	QTemporaryFile key_file(QDir::temp().filePath("id_rsaXXXXXX.pub"));
	key_file.setAutoRemove(false);
	if (!key_file.open())
	{
		key_file.remove();
		WRITE_TRACE(DBG_FATAL, "Error opening key file %s", qPrintable(key_file.fileName()));
		return Error::Simple(PRL_ERR_FILE_WRITE_ERROR);
	}
	if (key_file.write(key.toUtf8()) == -1)
	{
		key_file.remove();
		WRITE_TRACE(DBG_FATAL, "Error writing key to file %s", qPrintable(key_file.fileName()));
		return Error::Simple(PRL_ERR_FILE_WRITE_ERROR);
	}
	return QFileInfo(key_file.fileName());
}

Prl::Expected<QFileInfo, Error::Simple>
CRsaHelper::WriteKey(const QFileInfo& key)
{
	Q_UNUSED(key);
	return Error::Simple(PRL_ERR_SUCCESS);
}

// Rsa Encryptor

CRsaHelper::RsaEncryptor::RsaEncryptor(const QString& key)
{
	auto key_data = key.toUtf8();
	QSharedPointer<BIO> buf(BIO_new_mem_buf(reinterpret_cast<void*>(key_data.data()), key_data.size()), BIO_free);
	RSA* rsa_ = NULL;
	if (PEM_read_bio_RSAPublicKey(buf.data(), &rsa_, NULL, NULL) == NULL)
		WRITE_TRACE(DBG_FATAL, "Invalid public key");
	m_Rsa = QSharedPointer<RSA>(rsa_, RSA_free);
}

CRsaHelper::RsaEncryptor::RsaEncryptor(const QFileInfo& key_file)
{
	auto path = qPrintable(key_file.absoluteFilePath());
	QSharedPointer<FILE> fp(fopen(path, "rb"), &fclose);
	if (fp.isNull())
	{
		WRITE_TRACE(DBG_FATAL, "Can't open public key file %s", path);
		return;
	}
	RSA* rsa_ = NULL;
	if (PEM_read_RSAPublicKey(fp.data(), &rsa_, NULL, NULL) == NULL)
		WRITE_TRACE(DBG_FATAL, "Invalid public key from file %s", path);
	m_Rsa = QSharedPointer<RSA>(rsa_, &RSA_free);
}

Prl::Expected<QString, Error::Simple>
CRsaHelper::RsaEncryptor::operator()(const QString& data)
{
	if (m_Rsa == NULL)
		return Error::Simple(PRL_ERR_RSA_ENCRYPTION_FAILED);
	QByteArray buf(RSA_size(m_Rsa.data()), 0);
	QByteArray raw = data.toUtf8();
	if (RSA_public_encrypt(raw.size(),
						   reinterpret_cast<unsigned char*>(raw.data()),
						   reinterpret_cast<unsigned char*>(buf.data()),
						   m_Rsa.data(), RSA_PKCS1_OAEP_PADDING) == -1)
	{
		return Error::Simple(PRL_ERR_RSA_ENCRYPTION_FAILED);
	}
	return QString(buf.toBase64());
}

// Rsa Decryptor

CRsaHelper::RsaDecryptor::RsaDecryptor(const QString& key)
{
	auto key_data = key.toUtf8();
	QSharedPointer<BIO> buf(BIO_new_mem_buf(reinterpret_cast<void*>(key_data.data()), key_data.size()), BIO_free);
	RSA* rsa_ = NULL;
	if (PEM_read_bio_RSAPrivateKey(buf.data(), &rsa_, NULL, NULL) == NULL)
		WRITE_TRACE(DBG_FATAL, "Invalid private key");
	m_Rsa = QSharedPointer<RSA>(rsa_, RSA_free);
}

CRsaHelper::RsaDecryptor::RsaDecryptor(const QFileInfo& key_file)
{
	auto path = qPrintable(key_file.absoluteFilePath());
	QSharedPointer<FILE> fp(fopen(path, "rb"), &fclose);
	if (fp.isNull())
	{
		WRITE_TRACE(DBG_FATAL, "Can't open private key file %s", path);
		return;
	}
	RSA* rsa_ = NULL;
	if (PEM_read_RSAPrivateKey(fp.data(), &rsa_, NULL, NULL) == NULL)
		WRITE_TRACE(DBG_FATAL, "Invalid private key from file %s", path);
	m_Rsa = QSharedPointer<RSA>(rsa_, &RSA_free);
}

Prl::Expected<QString, Error::Simple>
CRsaHelper::RsaDecryptor::operator()(const QString& data)
{
	if (m_Rsa == NULL)
		return Error::Simple(PRL_ERR_RSA_DECRYPTION_FAILED);
	QByteArray buf(RSA_size(m_Rsa.data()), 0);
	auto raw = QByteArray::fromBase64(data.toUtf8());
	if (RSA_private_decrypt(raw.size(),
							reinterpret_cast<unsigned char*>(raw.data()),
							reinterpret_cast<unsigned char*>(buf.data()),
							m_Rsa.data(), RSA_PKCS1_OAEP_PADDING) == -1)
	{
		return Error::Simple(PRL_ERR_RSA_ENCRYPTION_FAILED);
	}

	return QString(buf);
}

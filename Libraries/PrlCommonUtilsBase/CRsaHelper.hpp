///////////////////////////////////////////////////////////////////////////////
///
/// @file CRsaHelper.hpp
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

#pragma once

#include <QString>
#include <QFile>
#include <openssl/rsa.h>
#include "SysError.h"
#include "CKeygenHelper.h"
#include "CFileHelper.h"
#include "ErrorSimple.h"

class CRsaHelper
{
public:
	CRsaHelper(const QDir& home = CFileHelper::homePath());

	/**
	 * Read OpenSSH public key from home_dir.
	 *
	 * @return Public key or error
	 */
	Prl::Expected<QString, Error::Simple> getOpenSshPublicKey();

	/**
	 * Encrypt data with RSA using public key from home dir.
	 *
	 * @param data Data to Encrypt
	 * @return Encrypted data or Error
	 */
	Prl::Expected<QString, Error::Simple> Encrypt(const QString& data);

	/**
	 * Encrypt data with RSA using specific public key.
	 *
	 * @param data Data to encrypt
	 * @param public_key Public key to use for encryption (QString or QFileInfo)
	 * @return Encrypted data or Error
	 */
	template<typename T>
	Prl::Expected<QString, Error::Simple>
	Encrypt(const QString& data, const T& public_key);

	/**
	 * Decrypt data with RSA using private key from home dir.
	 *
	 * @param data Data to decrypt
	 * @return Decrypted data or Error
	 */
	Prl::Expected<QString, Error::Simple> Decrypt(const QString& data);

	/**
	 * Decrypt data with RSA using specific public key.
	 *
	 * @param data Data to decrypt
	 * @param private_key Private key to use for decryption (QString or QFileInfo)
	 * @return Encrypted data or Error
	 */
	template<typename T>
	Prl::Expected<QString, Error::Simple>
	Decrypt(const QString& data, const T& private_key);

	/**
	 * Check whether public key is authorized (present in authorized_keys file)
	 *
	 * @param public_key Public key
	 * @return true if authorized else false
	 */
	bool isAuthorized(const QString& public_key);

private:
	class RsaEncryptor
	{
	public:
		RsaEncryptor(const QString& key);
		RsaEncryptor(const QFileInfo& key_file);
		Prl::Expected<QString, Error::Simple> operator()(const QString& data);
	private:
		QSharedPointer<RSA> m_Rsa;
		PRL_RESULT m_InitFailure;
	};

	class RsaDecryptor
	{
	public:
		RsaDecryptor(const QString& key);
		RsaDecryptor(const QFileInfo& key_file);
		Prl::Expected<QString, Error::Simple> operator()(const QString& data);
	private:
		QSharedPointer<RSA> m_Rsa;
		PRL_RESULT m_InitFailure;
	};

	/**
	 * Encrypt data with RSA using specified public PEM key file.
	 *
	 * @param data Data to encrypt
	 * @param pem_public_key Public key in PEM format to use for encryption
	 * @return Encrypted data or Error
	 */
	static Prl::Expected<QString, Error::Simple>
	EncryptWithPemKey(const QString& data, const QString& pem_public_key);

	/**
	 * Write key (QString) to file.
	 * QFileInfo overload does nothing (key assumed already written)
	 *
	 * @param key Public Key
	 * @return Path to newly created file or error
	 */
	static Prl::Expected<QFileInfo, Error::Simple>
	WriteKey(const QString& key);
	// TODO: replace fake method with `if constexpr` C++17 extension
	static Prl::Expected<QFileInfo, Error::Simple>
	WriteKey(const QFileInfo& key);

	QFileInfo m_RsaPrivateKeyFile;
	QFileInfo m_RsaPublicKeyFile;
	QFileInfo m_AuthorizedKeysFile;
};

// Implementation of template methods

template<typename T>
Prl::Expected<QString, Error::Simple>
CRsaHelper::Encrypt(const QString& data, const T& public_key)
{
	QFileInfo public_key_file;

	// If public key is stored in file, use it
	// Note: condition can be constexpr
	if (std::is_same<T, QFileInfo>::value)
		public_key_file = QFileInfo(public_key);

	// If key is stored in QString, write it to tmp file
	// Note: condition can be constexpr
	if (std::is_same<T, QString>::value)
	{
		auto write_res = WriteKey(public_key);
		if (write_res.isFailed())
			return write_res.error();
		public_key_file = QFileInfo(write_res.value());
	}

	auto pem_key = CKeygenHelper::GetPemKey(public_key_file);
	// Delete temporary file if it was created
	if (std::is_same<T, QString>::value)
		QFile(public_key_file.absoluteFilePath()).remove();

	if (pem_key.isFailed())
		return pem_key.error();

	return EncryptWithPemKey(data, pem_key.value());
}

template<typename T>
Prl::Expected<QString, Error::Simple>
CRsaHelper::Decrypt(const QString& data, const T& private_key)
{
	return RsaDecryptor(private_key)(data);
}


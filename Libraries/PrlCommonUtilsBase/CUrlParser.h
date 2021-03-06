/*
 * CUrlParser.h: Helper class for parsing URLs strings like
 * [protocol://][user:password@]machine[:port].
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


#ifndef __VIRTUOZZO_URL_PARSER_H__
#define __VIRTUOZZO_URL_PARSER_H__

#include <QString>

/**
 * Simple parser of URLs strings like [protocol://][user:password@]machine[:port]
 */
class CUrlParser
{
public:
	/**
	 * Class constructor
	 * @param URL string that will be parsed
	 */
	CUrlParser(const QString &sUrl);
	/**
	 * Returns protocol name extracted from URL string
	 */
	QString proto() const;
	/**
	 * Returns user name extracted from URL string
	 */
	QString userName() const;
	/**
	 * Returns password extracted from URL string
	 */
	QString password() const;
	/**
	 * Returns hostname extracted from URL string
	 */
	QString host() const;
	/**
	 * Returns port extracted from URL string
	 */
	quint32 port() const;

private:
	/** Parsing URL string */
	QString m_sUrl;
};

#endif // __VIRTUOZZO_URL_PARSER_H__


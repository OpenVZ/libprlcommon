/*
 * StringUtils.h
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


#ifndef _LIB_STR_UTIL_H_
#define _LIB_STR_UTIL_H_

#if _MSC_VER > 1000
#pragma once
#endif

#include <QString>
#include <QStringList>

namespace Virtuozzo {
/**
 * Generates unique filename for specified directory entries list. New filename generates as {prefix}{index}{suffix} combination
 * @param directory entries list
 * @param filename prefix (optional; if not specified then using default prefix "tmpfile")
 * @param filename suffix (optional)
 * @param delimiter between prefix and generated index (optional)
 * @return generated filename
 */
QString GenerateFilename(
			const QStringList &_dir_entries,
			const QString &sFilenamePrefix = "",
			const QString &sFilenameSuffix = "",
			const QString &sIndexDelimiter = ""
			);

/**
 * Formats sizeBytes as a string representation which is suitable for showing in GUI.
 *
 * @p includeUnits specifies if it should append units name to resulting string
 * (for example "55.2 MB" vs simply "55.2")
 */
QString getPrettySizeString( qulonglong sizeBytes, bool includeUnits );

/**
 * Formats timeSeconds as a string representation which is suitable for showing in GUI.
 *
 * @p includeUnits specifies if it should append units name to resulting string
 * (for example "55 minutes" vs simply "55")
 */
QString getPrettyTimeString( uint timeSeconds, bool includeUnits );

/* MAX size of mask mask */
const unsigned int MAX_NCPU = 4096;

/**
 * parse CPU mask in the string representation (0,3-7) to the bit one.
 *
 */
int parseCpuMask(const QString &sMask, unsigned int nMaxCpu = 0,
			char *bitMask = NULL, unsigned int nLen = 0);

/**
 * parse node mask in the string representation (0,3-7) to the bit one.
 *
 */
int parseNodeMask(const QString &sMask, char *bitMask = NULL, unsigned int nLen = 0);

/**
* @return true if vmName represents a valid name for vm and false otherwise
*/
bool isValidVmName( const QString& vmName );

/**
 * @return number of bits in the bitmask
 */
unsigned int getBitsCount(char *p, unsigned int size);

/**
 * Returns currency sign ($, €, ect), based on currency ID (USD, EUR, etc)
 */
QString currencySign( const QString& currencyId );

/* Return string representation of VE mount information.
 * @p volumeId specifies volume name
 * @p imagePath path to image file/dir
 * @p mountPath path to directory where image is mounted
 * @p filesystem filesystem type
 * @p totalSpace Image total space in bytes
 * @p freeSpace Image free space in bytes
 */
QString formatMountInfo(
		const QString &volumeId, const QString &imagePath,
		const QString &mountPath, const QString &filesystem,
		quint64 totalSpace, quint64 freeSpace);

/** Convert number to base26 ('a' - 'z') representation */
QString toBase26(uint value_);

/** Convert base26 ('a' - 'z') representation to number */
uint fromBase26(const QString& value_);

/** Generate a serial number suitable for HDD */
QString generateDiskSerialNumber();

/*
 * Check invalid symbols in HDD's serial number
 */
bool IsSerialNumberValid(const QString& qsSerial);

}

#endif

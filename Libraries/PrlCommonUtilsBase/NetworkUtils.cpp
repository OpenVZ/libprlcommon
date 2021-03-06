/*
 * NetworkUtils.cpp
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


#include <QHostAddress>
#include <QStringList>
#include "NetworkUtils.h"

bool NetworkUtils::ValidateAndConvertIpMask(QString &ip_mask)
{
	QString ip, mask;
	bool ret = ParseIpMask(ip_mask, ip, mask);
	if (ret)
		ip_mask = ip + "/" + mask;
	return ret;
}

bool NetworkUtils::ParseIpMask(const QString &ip_mask, QString &ip)
{
	QString mask;
	return ParseIpMask(ip_mask, ip, mask);
}

bool NetworkUtils::ParseIpMask(const QString &ip_mask, QString &ip, QString &mask)
{
	QHostAddress ip_addr;

	if (ip_mask.contains('/')) {
		// parse ip
		QStringList splited_ip_mask = ip_mask.split('/');
		if (splited_ip_mask.size() != 2)
			return false;

		if (!ip_addr.setAddress(splited_ip_mask[0]))
			return false;

		if (ip_addr.protocol() == QAbstractSocket::IPv4Protocol) {
			// parse mask
			QPair<QHostAddress, int> pair = QHostAddress::parseSubnet(ip_mask);
			if (pair.second == -1)
				return false;
			quint32 u_mask = 0;
			for (int i = 0; i < pair.second; i++)
				u_mask = u_mask | (1 << (31-i));

			QHostAddress mask_addr;
			// convert ipv4 mask to dot notation
			mask_addr.setAddress(u_mask);
			mask = mask_addr.toString();
		} else {
			QPair<QHostAddress, int> pair = QHostAddress::parseSubnet(ip_mask);
			if (pair.second == -1)
			{
				QHostAddress ip6_mask;
				if(!ip6_mask.setAddress(splited_ip_mask[1]))
					return false;
				mask = ip6_mask.toString();
			}
			else
				mask = QString("%1").arg(pair.second);
		}
	} else {
		if (!ip_addr.setAddress(ip_mask))
			return false;

		if (ip_addr.protocol() == QAbstractSocket::IPv4Protocol)
			mask = "255.255.255.0";
		else
			mask = "64";
	}

	ip = ip_addr.toString();

	return true;
}

std::pair<QStringList, QStringList> NetworkUtils::ParseIps(const QList<QString>& ips_)
{
	QStringList ipv4, ipv6;
	foreach (const QString& a, ips_)
	{
		QString b;
		if(!NetworkUtils::ParseIpMask(a, b))
			continue;

		if (QHostAddress(b).protocol() == QAbstractSocket::IPv6Protocol)
			ipv6 << a;
		else if (QHostAddress(b).protocol() == QAbstractSocket::IPv4Protocol)
			ipv4 << a;
	}
	return std::make_pair(ipv4 ,ipv6);
}

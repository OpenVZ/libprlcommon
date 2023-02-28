/*
 * VirtuozzoDefines.h: Virtuozzo Namespace primary constant
 * definition. This file contains definitions, which are used in
 * various project parts. NOTE. Do not include other headers into
 * this file.
 *
 * Copyright (c) 1999-2017, Parallels International GmbH
 * Copyright (c) 2017-2023 Virtuozzo International GmbH. All rights reserved.
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


#ifndef __VIRTUOZZO_DEFINES__
#define __VIRTUOZZO_DEFINES__

// Do not include any other headers except current.ver


/**
 * Virtuozzo directories & files
 */
#define VIRTUOZZO_LICENSES_XML_FILE "licenses.xml"

#define DISPATCHER_CONFIGURATION_SERVER_XML_FILE	"dispatcher.xml"

#define DISPATCHER_SERVICE_COMMON_NAME				"prl_disp_service"

#define VMDIR_DEFAULT_CATALOGUE_SERVER_FILE			"vmdirectorylist.xml"

#define NETWORK_CONFIGURATION_SERVER_XML_FILE		"network.xml"

constexpr const char QEMU_IMG_BIN[] = "/usr/bin/qemu-img";

#endif // __VIRTUOZZO_DEFINES__

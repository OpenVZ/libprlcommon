/*
 * CGuestOsesHelper.cpp: Class helper for supported guest OSes
 * operations
 *
 * Copyright (c) 1999-2017, Parallels International GmbH
 *
 * This file is part of Parallels SDK. Parallels SDK is free
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
 * Our contact details: Parallels International GmbH, Vordergasse 59, 8200
 * Schaffhausen, Switzerland.
 */


#include "Build/Current.ver"
#include "CGuestOsesHelper.h"
#include "ParallelsDirs.h"

COpaqueTypeListImpl *COpaqueTypeListImpl::CreateInstance(PRL_SIZE nTypeSize)
{
	switch (nTypeSize)
	{
		case sizeof(PRL_UINT8): return (new TOpaqueTypeList<PRL_UINT8>);
		case sizeof(PRL_UINT16): return (new TOpaqueTypeList<PRL_UINT16>);
		case sizeof(PRL_UINT32): return (new TOpaqueTypeList<PRL_UINT32>);
		case sizeof(PRL_UINT64): return (new TOpaqueTypeList<PRL_UINT64>);
		default: throw (PRL_ERR_INVALID_ARG);
	}
}

PRL_HOST_OS_TYPE CGuestOsesHelper::GetHostOsType(PRL_HOST_OS_TYPE nPassedOsType)
{
	if (PHO_UNKNOWN == nPassedOsType)
	{
#ifdef _WIN_
		return (PHO_WIN);
#elif defined(_LIN_)
		return (PHO_LIN);
#else
		return (PHO_UNKNOWN);
#endif
	}
	else
		return (nPassedOsType);
}

TOpaqueTypeList<PRL_UINT8> CGuestOsesHelper::GetSupportedOsesTypes(PRL_HOST_OS_TYPE nHostOsType)
{
	PRL_HOST_OS_TYPE nProcessedHostOsType = GetHostOsType(nHostOsType);
	TOpaqueTypeList<PRL_UINT8> _result;
	if (PHO_UNKNOWN != nProcessedHostOsType)
	{
#define ADD_OS_TYPE( osType ) _result.GetContainer().append( osType );
		ADD_OS_TYPE( PVS_GUEST_TYPE_WINDOWS )
		ADD_OS_TYPE( PVS_GUEST_TYPE_LINUX )
		ADD_OS_TYPE( PVS_GUEST_TYPE_FREEBSD )

#ifdef EXTERNALLY_AVAILABLE_BUILD
	// MacOS Guest Support EULA Enforcement
	// (not allowed to run on non-apple platform)
#ifdef PSBM_MAC
		if ( nProcessedHostOsType == PHO_MAC ||
			PAM_SERVER == ParallelsDirs::getAppExecuteMode())
#else
		if ( nProcessedHostOsType == PHO_MAC )
#endif
#endif
			ADD_OS_TYPE( PVS_GUEST_TYPE_MACOS )

#ifndef EXTERNALLY_AVAILABLE_BUILD
		ADD_OS_TYPE( PVS_GUEST_TYPE_CHROMEOS )
		ADD_OS_TYPE( PVS_GUEST_TYPE_ANDROID )
		ADD_OS_TYPE( PVS_GUEST_TYPE_OS2 )
		ADD_OS_TYPE( PVS_GUEST_TYPE_MSDOS )
		ADD_OS_TYPE( PVS_GUEST_TYPE_NETWARE )
		ADD_OS_TYPE( PVS_GUEST_TYPE_SOLARIS )
		ADD_OS_TYPE( PVS_GUEST_TYPE_OTHER )
#else
		if (PAM_SERVER != ParallelsDirs::getAppExecuteMode())
		{
			ADD_OS_TYPE( PVS_GUEST_TYPE_CHROMEOS )
			ADD_OS_TYPE( PVS_GUEST_TYPE_ANDROID )
			ADD_OS_TYPE( PVS_GUEST_TYPE_OS2 )
			ADD_OS_TYPE( PVS_GUEST_TYPE_MSDOS )
			ADD_OS_TYPE( PVS_GUEST_TYPE_SOLARIS )
			ADD_OS_TYPE( PVS_GUEST_TYPE_OTHER )
		}
#endif

#undef ADD_OS_TYPE
	}

	return (_result);
}

TOpaqueTypeList<PRL_UINT16> CGuestOsesHelper::GetSupportedOsesVersions(PRL_HOST_OS_TYPE nHostOsType, PRL_UINT8 nOsType)
{
	PRL_HOST_OS_TYPE nProcessedHostOsType = GetHostOsType(nHostOsType);
	TOpaqueTypeList<PRL_UINT16> _result;
	if (PHO_UNKNOWN != nProcessedHostOsType)
	{
#ifdef EXTERNALLY_AVAILABLE_BUILD
		PRL_APPLICATION_MODE nAppMode = ParallelsDirs::getAppExecuteMode();
#endif

#define ADD_OS_VER( ver ) _result.GetContainer().append( ver );

		switch ( nOsType )
		{
			case PVS_GUEST_TYPE_MACOS:
#ifdef EXTERNALLY_AVAILABLE_BUILD
#ifdef PSBM_MAC
			if ( PHO_MAC == nProcessedHostOsType ||
				PAM_SERVER == nAppMode )
#else
			if ( PHO_MAC == nProcessedHostOsType )
#endif
#endif
			{
				ADD_OS_VER( PVS_GUEST_VER_MACOS_UNIVERSAL )
			}
			break;

			case PVS_GUEST_TYPE_WINDOWS:
				ADD_OS_VER( PVS_GUEST_VER_WIN_2K )
				ADD_OS_VER( PVS_GUEST_VER_WIN_XP )
				ADD_OS_VER( PVS_GUEST_VER_WIN_2003 )
				ADD_OS_VER( PVS_GUEST_VER_WIN_VISTA )
				ADD_OS_VER( PVS_GUEST_VER_WIN_2008 )
				ADD_OS_VER( PVS_GUEST_VER_WIN_WINDOWS7 )
				ADD_OS_VER( PVS_GUEST_VER_WIN_WINDOWS8 )
				ADD_OS_VER( PVS_GUEST_VER_WIN_2012 )
				ADD_OS_VER( PVS_GUEST_VER_WIN_WINDOWS8_1 )
				ADD_OS_VER( PVS_GUEST_VER_WIN_WINDOWS10 )
				ADD_OS_VER( PVS_GUEST_VER_WIN_2016 )
				ADD_OS_VER( PVS_GUEST_VER_WIN_OTHER )
			break;

			case PVS_GUEST_TYPE_CHROMEOS:
				ADD_OS_VER( PVS_GUEST_VER_CHROMEOS_OTHER )
			break;

			case PVS_GUEST_TYPE_ANDROID:
				ADD_OS_VER( PVS_GUEST_VER_ANDROID_OTHER )
			break;

			case PVS_GUEST_TYPE_LINUX:
				ADD_OS_VER( PVS_GUEST_VER_LIN_UBUNTU )
				ADD_OS_VER( PVS_GUEST_VER_LIN_FEDORA )
				ADD_OS_VER( PVS_GUEST_VER_LIN_REDHAT )
				ADD_OS_VER( PVS_GUEST_VER_LIN_REDHAT_7 )
				ADD_OS_VER( PVS_GUEST_VER_LIN_SUSE )
				ADD_OS_VER( PVS_GUEST_VER_LIN_SLES11 )
				ADD_OS_VER( PVS_GUEST_VER_LIN_SLES12 )
				ADD_OS_VER( PVS_GUEST_VER_LIN_DEBIAN )
				ADD_OS_VER( PVS_GUEST_VER_LIN_CENTOS )
				ADD_OS_VER( PVS_GUEST_VER_LIN_CENTOS_7 )
				ADD_OS_VER( PVS_GUEST_VER_LIN_VZLINUX )
				ADD_OS_VER( PVS_GUEST_VER_LIN_VZLINUX_7 )
				ADD_OS_VER( PVS_GUEST_VER_LIN_CLOUDLINUX )
				ADD_OS_VER( PVS_GUEST_VER_LIN_CLOUDLINUX_7 )
				ADD_OS_VER( PVS_GUEST_VER_LIN_PSBM )
				ADD_OS_VER( PVS_GUEST_VER_LIN_OPENSUSE )
				ADD_OS_VER( PVS_GUEST_VER_LIN_RH_LEGACY )
				ADD_OS_VER( PVS_GUEST_VER_LIN_MANDRAKE )
				ADD_OS_VER( PVS_GUEST_VER_LIN_XANDROS )
				ADD_OS_VER( PVS_GUEST_VER_LIN_MAGEIA )
				ADD_OS_VER( PVS_GUEST_VER_LIN_MINT )
				ADD_OS_VER( PVS_GUEST_VER_LIN_KRNL_24 )
				ADD_OS_VER( PVS_GUEST_VER_LIN_KRNL_26 )
				ADD_OS_VER( PVS_GUEST_VER_LIN_OTHER )
			break;

			case PVS_GUEST_TYPE_FREEBSD:
				ADD_OS_VER( PVS_GUEST_VER_BSD_OTHER )
			break;

			case PVS_GUEST_TYPE_MSDOS:
				ADD_OS_VER( PVS_GUEST_VER_DOS_OTHER )
			break;

			case PVS_GUEST_TYPE_NETWARE:
				ADD_OS_VER( PVS_GUEST_VER_NET_OTHER )
			break;

			case PVS_GUEST_TYPE_SOLARIS:
				ADD_OS_VER( PVS_GUEST_VER_SOL_OTHER )
			break;

			default:;//unknown guest OS type
		}//switch
#undef ADD_OS_VER
	}//if

	return (_result);
}

PRL_UINT16 CGuestOsesHelper::GetDefaultOsVersion(PRL_UINT8 nOsType)
{
	PRL_APPLICATION_MODE nAppMode = ParallelsDirs::getAppExecuteMode();
	bool bIsDesktopOrWorkstation = ( nAppMode != PAM_SERVER );
	PRL_UINT16 nDefaultOsVersion = 0;
	switch ( nOsType )
	{
		case PVS_GUEST_TYPE_MACOS:
			nDefaultOsVersion = PVS_GUEST_VER_MACOS_SNOW_LEOPARD;
		break;
		case PVS_GUEST_TYPE_WINDOWS:
			nDefaultOsVersion = ( bIsDesktopOrWorkstation ? PVS_GUEST_VER_WIN_WINDOWS7 : PVS_GUEST_VER_WIN_2012 );
		break;
		case PVS_GUEST_TYPE_CHROMEOS:
			nDefaultOsVersion = PVS_GUEST_VER_CHROMEOS_1x;
		break;
		case PVS_GUEST_TYPE_ANDROID:
			nDefaultOsVersion = PVS_GUEST_VER_ANDROID_4_0;
		break;
		case PVS_GUEST_TYPE_LINUX:
			nDefaultOsVersion = ( bIsDesktopOrWorkstation ? PVS_GUEST_VER_LIN_UBUNTU : PVS_GUEST_VER_LIN_REDHAT );
		break;
		case PVS_GUEST_TYPE_FREEBSD:
			nDefaultOsVersion = PVS_GUEST_VER_BSD_7X;
		break;
		case PVS_GUEST_TYPE_OS2:
			nDefaultOsVersion = PVS_GUEST_VER_OS2_WARP45;
		break;
		case PVS_GUEST_TYPE_NETWARE:
			nDefaultOsVersion = PVS_GUEST_VER_NET_5X;
		break;
		case PVS_GUEST_TYPE_SOLARIS:
			nDefaultOsVersion = PVS_GUEST_VER_SOL_10;
		break;
		case PVS_GUEST_TYPE_MSDOS:
			nDefaultOsVersion = PVS_GUEST_VER_DOS_MS622;
		break;
		case PVS_GUEST_TYPE_OTHER:
			nDefaultOsVersion = PVS_GUEST_VER_OTH_OPENSTEP;
		break;
		default:;
	}
	return (nDefaultOsVersion);
}

PRL_GUEST_OS_SUPPORT_TYPE CGuestOsesHelper::GetSupportType(PRL_UINT16 nOsVersion)
{
	switch( nOsVersion )
	{
	case PVS_GUEST_VER_WIN_2008:
	case PVS_GUEST_VER_WIN_2012:
	case PVS_GUEST_VER_LIN_CENTOS:
	case PVS_GUEST_VER_LIN_CENTOS_7:
	case PVS_GUEST_VER_LIN_VZLINUX:
	case PVS_GUEST_VER_LIN_VZLINUX_7:
	case PVS_GUEST_VER_LIN_CLOUDLINUX:
	case PVS_GUEST_VER_LIN_CLOUDLINUX_7:
	case PVS_GUEST_VER_LIN_DEBIAN:
	case PVS_GUEST_VER_LIN_UBUNTU:
	case PVS_GUEST_VER_LIN_OPENSUSE:
	case PVS_GUEST_VER_LIN_FEDORA:
		return PGS_OS_SUPPORTED;
	default:
		return PGS_OS_EXPERIMENTAL;
	}
}

bool CGuestOsesHelper::CheckGuestOsType(PRL_UINT8 nOsType)
{
	PRL_UINT16 nOsVersion = PRL_UINT16(nOsType)<<8;
	return (
		IS_MACOS(nOsVersion)	||
		IS_WINDOWS(nOsVersion)	||
		IS_LINUX(nOsVersion)	||
		IS_FREEBSD(nOsVersion)	||
		IS_OS2(nOsVersion)		||
		IS_MSDOS(nOsVersion)	||
		IS_NETWARE(nOsVersion)	||
		IS_SOLARIS(nOsVersion)	||
		IS_OTHER(nOsVersion)
	);
}

bool CGuestOsesHelper::CheckGuestOsVersion(PRL_UINT8 nOsType, PRL_UINT16 nOsVersion)
{
	if (!CheckGuestOsType(nOsType))
		return false;

	TOpaqueTypeList<PRL_UINT16> lstVersions = GetSupportedOsesVersions(PHO_UNKNOWN, nOsType);
	return lstVersions.GetContainer().contains(nOsVersion);
}

bool CGuestOsesHelper::SupportsEfiBios(PRL_UINT32 uOsVer)
{
	if ( IS_VALID_LINUX_VERSION( uOsVer ) )
		return true;

	if ( uOsVer >= PVS_GUEST_VER_WIN_2008 && uOsVer <= PVS_GUEST_VER_WIN_LAST )
		return true;

	return false;
}

TOpaqueTypeList<PRL_UINT8> COsesMatrix::GetSupportedOsesTypes()
{
	TOpaqueTypeList<PRL_UINT8> _result;
	foreach(const TGuestTypeDescription &_guest_type_descr, m_OsesMatrix)
	{
		_result.GetContainer().append(_guest_type_descr.m_nGuestType);
	}
	return (_result);
}

TOpaqueTypeList<PRL_UINT16> COsesMatrix::GetSupportedOsesVersions(PRL_UINT8 nOsType)
{
	TOpaqueTypeList<PRL_UINT16> _result;
	foreach(const TGuestTypeDescription &_guest_type_descr, m_OsesMatrix)
	{
		if (_guest_type_descr.m_nGuestType == nOsType)
		{
			_result = _guest_type_descr.m_lstOsesVersions;
			break;
		}
	}
	return (_result);
}

PRL_UINT16 COsesMatrix::GetDefaultOsVersion(PRL_UINT8 nOsType)
{
	PRL_UINT16 nResult = 0;
	foreach(const TGuestTypeDescription &_guest_type_descr, m_OsesMatrix)
	{
		if (_guest_type_descr.m_nGuestType == nOsType)
		{
			nResult = _guest_type_descr.m_nDefaultOsVersion;
			break;
		}
	}
	return (nResult);
}

void COsesMatrix::AddOsType(PRL_HOST_OS_TYPE nHostOsType, PRL_UINT8 nGuestOsType)
{
	m_OsesMatrix.append(TGuestTypeDescription(nHostOsType, nGuestOsType));
}

bool COsesMatrix::operator==(const COsesMatrix &_obj) const
{
	return (m_OsesMatrix == _obj.m_OsesMatrix);
}

void COsesMatrix::Serialize(QDataStream &_stream)
{
	int nSize = m_OsesMatrix.size();
	CPrlOpaqueTypeDataSerializer<int>(nSize).Serialize(_stream);
	for (int i = 0; i < nSize; ++i)
		m_OsesMatrix[i].Serialize(_stream);
}

void COsesMatrix::Deserialize(QDataStream &_stream)
{
	m_OsesMatrix.clear();
	int nSize = 0;
	CPrlOpaqueTypeDataSerializer<int>(nSize).Deserialize(_stream);
	for (int i = 0; i < nSize; ++i)
	{
		TGuestTypeDescription _guest_type_descr;
		_guest_type_descr.Deserialize(_stream);
		m_OsesMatrix.append(_guest_type_descr);
	}
}


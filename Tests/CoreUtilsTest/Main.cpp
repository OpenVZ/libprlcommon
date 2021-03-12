/////////////////////////////////////////////////////////////////////////////
///
/// Copyright (c) 2006-2017, Parallels International GmbH
/// Copyright (c) 2017-2021 Virtuozzo International GmbH, All rights reserved.
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
///		TscTimeTest.cpp
///
/// @author
///		sergeyt
///
/// @brief
///		main().
///
/// @brief
///		None.
///
/////////////////////////////////////////////////////////////////////////////

#include "TscTimeTest.h"
#include "CAuthHelperTest.h"
#include "CFileHelperTest.h"
#include "CAclHelperTest.h"

#define EXECUTE_TESTS_SUITE(TESTS_SUITE_CLASS_NAME)\
{\
	TESTS_SUITE_CLASS_NAME _tests_suite;\
	nRet += QTest::qExec(&_tests_suite, argc, argv);\
}

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	int nRet = 0;
	EXECUTE_TESTS_SUITE(TscTimeTest);
	EXECUTE_TESTS_SUITE(CFileHelperTest)
	EXECUTE_TESTS_SUITE(CAuthHelperTest)
	EXECUTE_TESTS_SUITE(CAclHelperTest)

	return nRet;
}

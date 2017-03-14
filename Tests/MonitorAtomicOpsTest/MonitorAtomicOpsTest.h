/////////////////////////////////////////////////////////////////////////////
///
/// @file MonitorAtomicOpsTest.h
///
/// @brief Tests for different atomic operations implemented in monitor
///
/// @author denisla
///
/// Copyright (c) 2013-2017, Parallels International GmbH
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
/// Our contact details: Parallels International GmbH, Vordergasse 59, 8200
/// Schaffhausen, Switzerland.
///
/////////////////////////////////////////////////////////////////////////////

#ifndef __MONITOR_ATOMIC_OPS_H__
#define __MONITOR_ATOMIC_OPS_H__

#include <QtTest/QtTest>


/**
* Class that contains unit-tests for testing monitor atomic operations.
*
* @author denisla
*/
class CMonitorAtomicOpsTest : public QObject
{
	Q_OBJECT

public:
	 CMonitorAtomicOpsTest() {}
	~CMonitorAtomicOpsTest() {}

private slots:
	void initTestCase();
	void cleanupTestCase() {}

	void testIncI();
	void testIncUI();
	void testIncI64();
	void testIncUI64();

	void testDecI();
	void testDecUI();
	void testDecI64();
	void testDecUI64();

	void testIncAndTestI();
	void testIncAndTestUI();
	void testIncAndTestI64();
	void testIncAndTestUI64();

	void testDecAndTestI();
	void testDecAndTestUI();
	void testDecAndTestI64();
	void testDecAndTestUI64();

	void testAddI();
	void testAddUI();
	void testAddI64();
	void testAddUI64();

	void testOrI();
	void testOrUI();
	void testOrI64();
	void testOrUI64();

	void testXorI();
	void testXorUI();
	void testXorI64();
	void testXorUI64();

	void testAndI();
	void testAndUI();
	void testAndI64();
	void testAndUI64();

	void testSwapI();
	void testSwapUI();
	void testSwapI64();
	void testSwapUI64();

	void testCompareSwapI();
	void testCompareSwapUI();
	void testCompareSwapI64();
	void testCompareSwapUI64();

	void testReadI();
	void testReadUI();
	void testReadI64();
	void testReadUI64();

	void testWriteI();
	void testWriteUI();
	void testWriteI64();
	void testWriteUI64();

public:
	static const unsigned int m_nIterations = 32;
	// Maximum is 32
	static const unsigned int m_nThreads = 8;
};

#endif // __MONITOR_ATOMIC_OPS_H__

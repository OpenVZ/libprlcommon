/////////////////////////////////////////////////////////////////////////////
///
/// Copyright (c) 2006-2017, Parallels International GmbH
/// Copyright (c) 2017-2019 Virtuozzo International GmbH, All rights reserved.
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
///		CommonTestsUtils.h
///
/// @author
///		sergeyt, aleksandera
///
/// @brief
///		Common utils using at all dispatcher API tests suites classes.
///
/// @brief
///		None.
///
/////////////////////////////////////////////////////////////////////////////
#ifndef CommonTestsUtils_H
#define CommonTestsUtils_H

#include "Interfaces/ParallelsNamespace.h"
#include "Interfaces/ParallelsNamespaceTests.h"
#include <prlsdk/PrlEnums.h>

#include <QList>
#include <QString>
#include <QtTest/QtTest>
#include "Libraries/Logging/Logging.h"
#include "Libraries/PrlCommonUtilsBase/PrlStringifyConsts.h"

#define PRL_JOB_WAIT_TIMEOUT 18000
#define STR_BUF_LENGTH 32768

#define GEN_VM_NAME_BY_TEST_FUNCTION() \
	QString("%1_%2").arg(QTest::currentTestFunction()).arg( qrand() )

#define QVERIFY_EXPRESSION( _expression ) \
{	(_expression); QVERIFY( ! QTest::currentTestFailed() ); }

#define CHECK_RET_CODE_EXP(expression)\
	{\
		PRL_RESULT nExprRetCode = (expression);\
		if (PRL_FAILED(nExprRetCode))\
			WRITE_TRACE(DBG_FATAL, "Expression: '%s' nRetCode=%.8X '%s'", #expression, nExprRetCode, PRL_RESULT_TO_STRING(nExprRetCode));\
		QCOMPARE(nExprRetCode, PRL_ERR_SUCCESS);\
	}

#define CHECK_CONCRETE_EXPRESSION_RET_CODE(expression, retcode)\
	{\
		PRL_RESULT nRetCode = (expression);\
		if (nRetCode != retcode)\
			WRITE_TRACE(DBG_FATAL, "Expression: '%s' expected %.8X '%s' but %.8X '%s' was received", #expression,\
					retcode, PRL_RESULT_TO_STRING(retcode), nRetCode, PRL_RESULT_TO_STRING(nRetCode));\
		QVERIFY(nRetCode == retcode);\
	}

#define EXECUTE_TESTS_SUITE(TESTS_SUITE_CLASS_NAME)\
	{\
		QStringList argList;\
		if (checkArguments(argc, argv, #TESTS_SUITE_CLASS_NAME ,argList))\
		{\
			TESTS_SUITE_CLASS_NAME _tests_suite;\
			nRet += QTest::qExec(&_tests_suite, argList);\
		}\
	}

#define OUTPUT_HANDLES_NUM_FOR_TYPE(handle_type)\
	{\
		PRL_UINT32 nHandlesNum = 0;\
		PrlDbg_GetHandlesNum(&nHandlesNum, handle_type);\
		if (nHandlesNum != 0)\
			WRITE_TRACE(DBG_FATAL, "WARNING Not all handles were freed. Handles instances of type '%s': %d",\
							PRL_HANDLE_TYPE_TO_STRING(handle_type), nHandlesNum);\
	}

#define CHECK_RET_CODE(_ret_code)\
	if (PRL_FAILED(_ret_code))\
	{\
		WRITE_TRACE(DBG_FATAL, "Return code: %.8X '%s'", _ret_code, PRL_RESULT_TO_STRING(_ret_code));\
		QFAIL("Return code not successful!");\
	}

#define CALL_CMD(cmd, expecting_response_code)\
	{\
		m_pHandler->Clear();\
		QString sReqId = cmd;\
		const quint32 nStepInterval = 100;\
		m_pPveControl->WaitForRequestComplete(sReqId, PRL_JOB_WAIT_TIMEOUT);\
		quint32 nWaitTimeout = PRL_JOB_WAIT_TIMEOUT;\
		while (m_pHandler->GetResult().getOpCode() != expecting_response_code && nWaitTimeout)\
		{\
			QCoreApplication::processEvents(QEventLoop::AllEvents);\
			if (nWaitTimeout < nStepInterval)\
				nWaitTimeout = 0;\
			else\
				nWaitTimeout -= nStepInterval;\
		}\
	}

#define TEST_BEHAVIOR_ON_NULL_BUF_SIZE(handle, sdk_call)\
	{\
		PRL_UINT32 nBufSize = 0;\
		CHECK_RET_CODE_EXP(sdk_call(handle, 0, &nBufSize));\
	}

#define PRL_EXTRACT_STRING_VALUE(result_string, handle, sdk_method)\
	{\
		QByteArray _string_value_extracting_buf;\
		PRL_UINT32 nStringValueExtractingBufSize = 0;\
		CHECK_RET_CODE_EXP(sdk_method(handle, 0, &nStringValueExtractingBufSize))\
		QVERIFY(nStringValueExtractingBufSize != 0);\
		_string_value_extracting_buf.resize(nStringValueExtractingBufSize);\
		CHECK_RET_CODE_EXP(sdk_method(handle, _string_value_extracting_buf.data(), &nStringValueExtractingBufSize))\
		result_string = UTF8_2QSTR(_string_value_extracting_buf);\
	}

#define PRL_EXTRACT_STRING_VALUE_BY_INDEX(result_string, handle, index, sdk_method)\
	{\
		QByteArray _string_value_extracting_buf;\
		PRL_UINT32 nStringValueExtractingBufSize = 0;\
		CHECK_RET_CODE_EXP(sdk_method(handle, index, 0, &nStringValueExtractingBufSize))\
		QVERIFY(nStringValueExtractingBufSize != 0);\
		_string_value_extracting_buf.resize(nStringValueExtractingBufSize);\
		CHECK_RET_CODE_EXP(sdk_method(handle, index, _string_value_extracting_buf.data(), &nStringValueExtractingBufSize))\
		result_string = UTF8_2QSTR(_string_value_extracting_buf);\
	}

#define CHECK_EVENT_PARAMETER(pEvent, param_name, param_type, param_value)\
	{\
		CVmEventParameter *pParam = pEvent->getEventParameter(param_name);\
		QVERIFY(pParam != NULL);\
		QVERIFY(pParam->getParamType() == param_type);\
		QCOMPARE(param_value, pParam->getParamValue());\
	}

#define VM_CONFIG_INIT \
	CVmConfiguration _vm_conf; \
	CHECK_RET_CODE_EXP(PrlSrv_CreateVm(m_ServerHandle, m_VmHandle.GetHandlePtr()))

#define VM_CONFIG_TO_XML_OBJECT \
{ \
	PRL_VOID_PTR pXml = 0; \
	CHECK_RET_CODE_EXP(PrlHandle_ToString(m_VmHandle, &pXml)); \
	_vm_conf.fromString( UTF8_2QSTR((PRL_CONST_STR )pXml) ); \
	PrlBuffer_Free(pXml); \
}

#define VM_CONFIG_FROM_XML_OBJECT \
	CHECK_RET_CODE_EXP(PrlHandle_FromString( m_VmHandle, QSTR2UTF8(_vm_conf.toString()) ));

#define CHECK_PARAMS_COUNT(hResult, expected_count)\
	{\
		PRL_UINT32 nCheckParamsCount = 0;\
		CHECK_RET_CODE_EXP(PrlResult_GetParamsCount((hResult), &nCheckParamsCount))\
		QCOMPARE(quint32(nCheckParamsCount), quint32(expected_count));\
	}

#define SKIP_IF_EXTERNAL_BUILD \
	if ( TestConfig::isExternalBuild() ) \
		QSKIP("Test skipping due functionality not supported at external build", SkipAll);

extern const int CONNECTION_TIMEOUT;

namespace QTest {

template<>
char *toString ( const PVE::IDispatcherCommands &value);

}//namespace QTest

class TestConfig
{
public:
   static char* getUserLogin();
   static char* getUserLogin2();
   static char* getUserPassword();
   static char* getRemoteHostName();
   static char* getLocalHostName();

   static QString getPathToDispatcherConfig();

	static bool isExternalBuild();
	static void readTestParameters();
	static PRL_APPLICATION_MODE getApplicationMode();
	static PRL_UINT32	getSdkInitFlags();
	static bool isCtMode() { return g_CtMode; }

public:
	class InitRandom
	{
	public:
		InitRandom();
	};
public:
	static bool g_CtMode;

private:
	static PRL_APPLICATION_MODE g_executeMode;
};

bool CheckElementPresentsInList(const QList<QString> &_list, const QString &_elem);
void InitializeTooLongString();
void CleanupTooLongString();
char *GetTooLongString();

bool checkArguments(int argc, char **argv, const QString testName, QStringList &testArgsList);

#define VERIFY_WITH_BOOL_RETURN(statement) \
	do {\
		if (!(statement)){ \
			WRITE_TRACE( DBG_FATAL, "'%s' FAILED", #statement );\
			return false; \
		}\
	} while (0)

#define CHECK_RET_CODE_EXP_WITH_BOOL_RETURN(expression) \
{\
	PRL_RESULT nExprRetCode = (expression);\
	if (PRL_FAILED(nExprRetCode))\
		WRITE_TRACE(DBG_FATAL, "Expression: '%s' nRetCode=%.8X '%s'", #expression, nExprRetCode, PRL_RESULT_TO_STRING(nExprRetCode));\
	VERIFY_WITH_BOOL_RETURN(PRL_SUCCEEDED(nExprRetCode));\
}

#define CLEAR_QDATETIME_MSECS( _dateTime ) \
{	\
	QTime t = _dateTime.time(); \
	_dateTime.setTime( QTime( t.hour(), t.minute(), t.second() ) ); \
}

#endif

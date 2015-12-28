TEMPLATE = lib
CONFIG += staticlib

include(Std.pri)

INCLUDEPATH += ../../..

	HEADERS += \
		   SmartPtr.h  \
		   PrlAssert.h \

	HEADERS += \
		   PrlTime.h   \
		   LockedPtr.h  \
		   BitOps.h \

	SOURCES += \
		PrlTime.cpp    \

	linux-*:SOURCES += \
			PrlTime_lin.cpp \

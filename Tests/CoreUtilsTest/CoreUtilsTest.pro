CONFIG += qtestlib testcase
QT = core

include(CoreUtilsTest.deps)

HEADERS += \
	TscTimeTest.h \
	CAuthHelperTest.h \
	CFileHelperTest.h \
	CAclHelperTest.h

SOURCES += \
	Main.cpp \
	TscTimeTest.cpp \
    CAuthHelperTest.cpp \
    CFileHelperTest.cpp \
    CAclHelperTest.cpp

macx {
    LIBS += \
		-framework DirectoryService
}

LIBS += -lTestsUtils

# It is important to have "File Info" embedded in the
# windows binaries - which means we need windows resource file
win32:RC_FILE = $$SRC_LEVEL/Tests/UnitTests.rc

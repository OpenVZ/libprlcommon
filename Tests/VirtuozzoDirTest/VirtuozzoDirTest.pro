CONFIG += qtestlib testcase
QT = core

INCLUDEPATH += /usr/share /usr/include/prlsdk

include(VirtuozzoDirTest.deps)

HEADERS += \
	VirtuozzoDirTest.h\
	CUrlParserTest.h

SOURCES += \
	VirtuozzoDirTest.cpp\
	Main.cpp   \
	CUrlParserTest.cpp

macx {
	LIBS += -framework Carbon
}

LIBS += -lTestsUtils

# It is important to have "File Info" embedded in the
# windows binaries - which means we need windows resource file
win32:RC_FILE = $$SRC_LEVEL/Tests/UnitTests.rc

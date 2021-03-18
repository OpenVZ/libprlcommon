CONFIG += qtestlib testcase
QT = network core

INCLUDEPATH += /usr/share /usr/include/prlsdk

include(QtLibraryTest.deps)

HEADERS += QtCoreTest.h

SOURCES +=  Main.cpp \
            QtCoreTest.cpp

LIBS += -lTestsUtils

# It is important to have "File Info" embedded in the
# windows binaries - which means we need windows resource file
win32:RC_FILE = $$SRC_LEVEL/Tests/UnitTests.rc

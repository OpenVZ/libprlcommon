CONFIG += qtestlib thread testcase
QT = core

TARGET = test_atomic_ops

LEVEL = ../..
include($$LEVEL/Sources/Parallels.pri)

HEADERS += AtomicOpsTest.h
SOURCES += AtomicOpsTest.cpp

INCLUDEPATH += /usr/share $$LEVEL $$LEVEL/Interfaces /usr/include/prlsdk

# It is important to have "File Info" embedded in the
# windows binaries - which means we need windows resource file
win32:RC_FILE = $$LEVEL/Sources/Tests/UnitTests.rc

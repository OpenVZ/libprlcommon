CONFIG += qtestlib testcase
QT = xml core

include(CoreUtilsTest.deps)

copydata.commands = $(MKDIR) $$PWD/../../z-Build/Debug/.vz/keys && \
                    $(MKDIR) $$PWD/../../z-Build/Release/.vz/keys && \
                    $(COPY) $$PWD/RSAExampleKeys/* $$PWD/../../z-Build/Debug/.vz/keys && \
                    $(COPY) $$PWD/RSAExampleKeys/* $$PWD/../../z-Build/Release/.vz/keys
first.depends = $(first) copydata
export(first.depends)
export(copydata.commands)
QMAKE_EXTRA_TARGETS += first copydata

HEADERS += \
	TscTimeTest.h \
	CAuthHelperTest.h \
	CFileHelperTest.h \
	CAclHelperTest.h \
	CRsaHelperTest.h

SOURCES += \
	Main.cpp \
	TscTimeTest.cpp \
    CAuthHelperTest.cpp \
    CFileHelperTest.cpp \
    CAclHelperTest.cpp \
    CRsaHelperTest.cpp

macx {
    LIBS += \
		-framework DirectoryService
}

LIBS += -L$$SRC_LEVEL/z-Build/Release -lssl -lcrypto -lTestsUtils

# It is important to have "File Info" embedded in the
# windows binaries - which means we need windows resource file
win32:RC_FILE = $$SRC_LEVEL/Tests/UnitTests.rc

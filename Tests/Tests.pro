TEMPLATE = subdirs

LEVEL = ..

CONFIG = testcase

#archive.target = $$LEVEL/z-Build/unittests.zip
#archive.target = $$LEVEL/z-Build/Debug/out.txt
#archive.command = $$PWD/PackTests.sh
#QMAKE_EXTRA_TARGETS += archive
#
#POST_TARGETDEPS += archive

include($$LEVEL/Build/Options.pri)

include($$PWD/CoreUtilsTest/CoreUtilsTest.deps)
addSubdirsDir(AtomicOpsTest, $$PWD/AtomicOpsTest)
addSubdirsDir(MonitorAtomicOpsTest, $$PWD/MonitorAtomicOpsTest)
addSubdirsDir(IOServiceTest, $$PWD/IOServiceTest)

include($$PWD/ParallelsDirTest/ParallelsDirTest.deps)
include($$PWD/StdTest/StdTest.pro)
include($$PWD/QtLibraryTest/QtLibraryTest.deps)

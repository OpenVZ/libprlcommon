
TEMPLATE = subdirs

CONFIG += testcase

LEVEL = ../..
include($$LEVEL/Build/Options.pri)

include($$PWD/UuidTest/UuidTest.deps)
include($$PWD/BitOpsTest/BitOpsTest.deps)

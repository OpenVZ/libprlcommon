
TEMPLATE = subdirs

CONFIG += testcase

LEVEL = ../../..
include($$LEVEL/Build/Options.pri)

include(communicationtest/communicationtest.deps)
include(communicationtest_gui/communicationtest_gui.pro)
#include(encoderstest/encoderstest.deps)
include(protocoltest/protocoltest.deps)
include(routingtabletest/routingtabletest.deps)
#include(tcpcontrolblockstattest/tcpcontrolblockstattest.deps)
include(ssltest/ssltest.deps)

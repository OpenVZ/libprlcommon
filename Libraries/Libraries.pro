LIBTARGET = prlcommon
TARGET = prlcommon
PROJ_PATH = $$PWD

include(../Build/qmake/build_target.pri)
include(Libraries.deps)

target.path = $${PREFIX}/lib64
INSTALLS += target

LIBTARGET = Std
PROJ_FILE = $$PWD/Std.pro
QTCONFIG = core
include(../../Build/qmake/staticlib.pri)

win32: LIBS	+= -lshell32 -lAdvapi32 -lole32
linux-*: LIBS += -lrt

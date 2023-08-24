QT -= gui

TEMPLATE = lib
CONFIG += staticlib

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ../../src/tup_body.c \
    ../../src/tup_bufReader.c \
    ../../src/tup_bufWriter.c \
    ../../src/tup_crc32.c \
    ../../src/tup_endianness.c \
    ../../src/tup_frame_receiver.c \
    ../../src/tup_frame_sender.c \
    ../../src/tup_header.c \
    ../../src/tup_instance.c \
    ../../src/tup_platform.c \
    ../../src/tup_v1_body.c \
    ../../src/tup_v1_transfer.c

HEADERS += \
    ../../inc/tup_instance.h \
    ../../inc/tup_port.h \
    ../../inc/tup_v1_types.h \
    ../../src/tup_body.h \
    ../../src/tup_bufReader.h \
    ../../src/tup_bufWriter.h \
    ../../src/tup_crc32.h \
    ../../src/tup_endianness.h \
    ../../src/tup_frame_receiver.h \
    ../../src/tup_frame_sender.h \
    ../../src/tup_header.h \
    ../../src/tup_platform.h \
    ../../src/tup_types.h \
    ../../src/tup_v1_body.h \
    ../../src/tup_v1_transfer.h

INCLUDEPATH += \
    ../../inc  \
    ../../src  \

DESTDIR = ../../lib

win32 {
    !contains(QMAKE_TARGET.arch, x86_64) {
        Debug:TARGET = tup_win_x86_debug
        Release:TARGET = tup_win_x86_release

    } else {
        Debug:TARGET = tup_win_x64_debug
        Release:TARGET = tup_win_x64_release
    }
}




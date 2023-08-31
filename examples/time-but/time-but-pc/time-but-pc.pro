QT       += core gui serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

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
    ../../../tup/src/tup_body.c \
    ../../../tup/src/tup_bufReader.c \
    ../../../tup/src/tup_bufWriter.c \
    ../../../tup/src/tup_crc32.c \
    ../../../tup/src/tup_endianness.c \
    ../../../tup/src/tup_frame_receiver.c \
    ../../../tup/src/tup_frame_sender.c \
    ../../../tup/src/tup_header.c \
    ../../../tup/src/tup_instance.c \
    ../../../tup/src/tup_platform.c \
    ../../../tup/src/tup_v1_body.c \
    ../../../tup/src/tup_v1_transfer.c \
    ../common/app_protocol.c \
    main.cpp \
    mainwindow.cpp \
    master_form.cpp \
    master_handler.cpp \
    port_widget.cpp \
    slave_form.cpp \
    slave_handler.cpp \
    tup_wrapper.cpp

HEADERS += \
    ../../../tup/src/tup_body.h \
    ../../../tup/src/tup_bufReader.h \
    ../../../tup/src/tup_bufWriter.h \
    ../../../tup/src/tup_crc32.h \
    ../../../tup/src/tup_endianness.h \
    ../../../tup/src/tup_frame_receiver.h \
    ../../../tup/src/tup_frame_sender.h \
    ../../../tup/src/tup_header.h \
    ../../../tup/src/tup_platform.h \
    ../../../tup/src/tup_types.h \
    ../../../tup/src/tup_v1_body.h \
    ../../../tup/src/tup_v1_transfer.h \
    ../common/app_protocol.h \
    mainwindow.h \
    master_form.h \
    master_handler.h \
    port_widget.h \
    slave_form.h \
    slave_handler.h \
    tup_wrapper.h

FORMS += \
    mainwindow.ui \
    master_form.ui \
    port_widget.ui \
    slave_form.ui

INCLUDEPATH += \
    ../../../tup/inc \
    ../common

#win32 {
#    !contains(QMAKE_TARGET.arch, x86_64) {
#        Debug:LIBS += -L../../../tup/lib -ltup_win_x86_debug
#        Release:LIBS += -L../../../tup/lib -ltup_win_x86_release
#
#    } else {
#        Debug:LIBS += -L../../../tup/lib -ltup_win_x64_debug
#        Release:LIBS += -L../../../tup/lib -ltup_win_x64_debug
#    }
#}

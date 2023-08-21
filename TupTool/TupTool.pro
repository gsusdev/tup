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
    ../tup/src/tup_body.c \
    ../tup/src/tup_bufReader.c \
    ../tup/src/tup_bufWriter.c \
    ../tup/src/tup_crc32.c \
    ../tup/src/tup_endianness.c \
    ../tup/src/tup_frame_receiver.c \
    ../tup/src/tup_frame_sender.c \
    ../tup/src/tup_header.c \
    ../tup/src/tup_instance.c \
    ../tup/src/tup_platform.c \
    ../tup/src/tup_v1_body.c \
    ../tup/src/tup_v1_transfer.c \
    ack_widget.cpp \
    coder_widget.cpp \
    data_widget.cpp \
    fin_widget.cpp \
    instance_form.cpp \
    instance_widget.cpp \
    main.cpp \
    mainwindow.cpp \
    syn_widget.cpp \
    transfer_widget.cpp \
    tup_instance_w.cpp \
    tup_receiver.cpp \
    tup_sender.cpp \
    tup_transfer.cpp \
    utils.cpp

HEADERS += \
    ../tup/src/tup_body.h \
    ../tup/src/tup_bufReader.h \
    ../tup/src/tup_bufWriter.h \
    ../tup/src/tup_crc32.h \
    ../tup/src/tup_endianness.h \
    ../tup/src/tup_frame_receiver.h \
    ../tup/src/tup_frame_sender.h \
    ../tup/src/tup_header.h \     \
    ../tup/src/tup_platform.h \
    ../tup/src/tup_types.h \
    ../tup/src/tup_v1_body.h \
    ../tup/src/tup_v1_transfer.h \
    ../tup/inc/tup_v1_types.h \
    ../tup/inc/tup_port.h \
    ../tup/inc/tup_instance.h \
    ack_widget.h \
    coder_widget.h \
    common.h \
    data_widget.h \
    fin_widget.h \
    instance_form.h \
    instance_widget.h \
    mainwindow.h \
    syn_widget.h \
    transfer_widget.h \
    tup_instance_w.h \
    tup_receiver.h \
    tup_sender.h \
    tup_transfer.h \
    utils.h

FORMS += \
    ack_widget.ui \
    coder_widget.ui \
    data_widget.ui \
    fin_widget.ui \
    instance_form.ui \
    instance_widget.ui \
    mainwindow.ui \
    syn_widget.ui \
    transfer_widget.ui

INCLUDEPATH += ../tup/src
INCLUDEPATH += ../tup/inc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

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
    ../tup/tup_body.c \
    ../tup/tup_bufReader.c \
    ../tup/tup_bufWriter.c \
    ../tup/tup_crc32.c \
    ../tup/tup_endianness.c \
    ../tup/tup_frame_receiver.c \
    ../tup/tup_frame_sender.c \
    ../tup/tup_header.c \
    ../tup/tup_platform.c \
    ../tup/tup_v1_body.c \
    ../tup/tup_v1_transfer.c \
    ack_widget.cpp \
    coder_widget.cpp \
    data_widget.cpp \
    fin_widget.cpp \
    instance_form.cpp \
    main.cpp \
    mainwindow.cpp \
    syn_widget.cpp \
    transfer_widget.cpp \
    tup_receiver.cpp \
    tup_sender.cpp \
    tup_transfer.cpp \
    utils.cpp

HEADERS += \
    ../tup/tup_body.h \
    ../tup/tup_bufReader.h \
    ../tup/tup_bufWriter.h \
    ../tup/tup_crc32.h \
    ../tup/tup_endianness.h \
    ../tup/tup_frame_receiver.h \
    ../tup/tup_frame_sender.h \
    ../tup/tup_header.h \
    ../tup/tup_platform.h \
    ../tup/tup_port.h \
    ../tup/tup_types.h \
    ../tup/tup_v1_body.h \
    ../tup/tup_v1_transfer.h \
    ../tup/tup_v1_types.h \
    ack_widget.h \
    coder_widget.h \
    common.h \
    data_widget.h \
    fin_widget.h \
    instance_form.h \
    mainwindow.h \
    syn_widget.h \
    transfer_widget.h \
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
    mainwindow.ui \
    syn_widget.ui \
    transfer_widget.ui

INCLUDEPATH += ../tup

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

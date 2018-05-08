QT = widgets serialport network

TARGET = QTGCS
TEMPLATE = app
ICON = QTGCS.icns
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000

SOURCES += \
    main.cpp\
    mainwindow.cpp \
    map.cpp \
    multiwii.cpp \
    quadstates.cpp \
    serialcommunication_usb.cpp \
    serialcommunication_xbee_api.cpp \
    serialcommunication_xbee_at.cpp \
    serialcommunication.cpp \
    dataexchange.cpp \
    inputdialog.cpp \
    qtxb.cpp \
    qtxb/digimeshpacket.cpp \
    qtxb/atcommandqueueparam.cpp \
    qtxb/atcommand.cpp \
    qtxb/txrequest.cpp \
    qtxb/txrequestexplicit.cpp \
    qtxb/atcommandremote.cpp \
    qtxb/atcommandresponse.cpp \
    qtxb/modemstatus.cpp \
    qtxb/transmitstatus.cpp \
    qtxb/rxindicator.cpp \
    qtxb/remotecommandrequest.cpp \
    qtxb/rxindicatorexplicit.cpp \
    qtxb/nodeidentificationindicator.cpp \
    qtxb/remotecommandresponse.cpp \
    localserver.cpp \
    remotecontrol_xbee_at.cpp \
    helpdialog.cpp \
    logdialog.cpp \
    logmanager.cpp \
    logmessage.cpp \
    msp_v2.cpp \
    msp_v1.cpp \
    telemetryserialworker.cpp \
    utility.cpp

HEADERS += \
    mainwindow.h \
    map.h \
    multiwii.h \
    quadstates.h \
    serialcommunication_usb.h \
    serialcommunication_xbee_api.h \
    serialcommunication_xbee_at.h \
    serialcommunication.h \
    dataexchange.h \
    inputdialog.h \
    qtxb.h \
    qtxb/digimeshpacket.h \
    qtxb/atcommandqueueparam.h \
    qtxb/atcommand.h \
    qtxb/txrequest.h \
    qtxb/txrequestexplicit.h \
    qtxb/atcommandremote.h \
    qtxb/atcommandresponse.h \
    qtxb/modemstatus.h \
    qtxb/transmitstatus.h \
    qtxb/rxindicator.h \
    qtxb/remotecommandrequest.h \
    qtxb/rxindicatorexplicit.h \
    qtxb/nodeidentificationindicator.h \
    qtxb/remotecommandresponse.h \
    localserver.h \
    remotecontrol_xbee_at.h \
    helpdialog.h \
    logdialog.h \
    logmanager.h \
    logmessage.h \
    msp_v2.h \
    msp_v1.h \
    telemetryserialworker.h \
    utility.h

FORMS += \
    mainwindow.ui \
    logdialog.ui

QMAKE_MAC_SDK = macosx10.13
QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.13.4

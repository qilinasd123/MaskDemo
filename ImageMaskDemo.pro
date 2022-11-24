QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Widget.cpp \
    canvas/MGraphicsItem.cpp \
    canvas/MPointItem.cpp \
    canvas/MaskGraphicsScene.cpp \
    canvas/RoiGraphicsScene.cpp \
    canvas/RoiViewWidget.cpp \
    main.cpp \
    tools/SetImageMaskWidget.cpp

HEADERS += \
    Widget.h \
    canvas/MGraphicsItem.h \
    canvas/MPointItem.h \
    canvas/MaskGraphicsScene.h \
    canvas/RoiGraphicsScene.h \
    canvas/RoiViewWidget.h \
    tools/SetImageMaskWidget.h

FORMS += \
    Widget.ui \
    tools/SetImageMaskWidget.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES +=

RESOURCES += \
    image.qrc

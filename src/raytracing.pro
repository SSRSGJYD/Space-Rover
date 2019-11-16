#-------------------------------------------------
#
# Project created by QtCreator 2019-10-29T16:46:28
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = raytracing
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    openglscene.cpp \
    paramdialog.cpp \
    cameradialog.cpp \
    global.cpp \
    raytracingscene.cpp \
    scene.cpp \
    aabb.cpp \
    camera.cpp \
    locallighting.cpp \
    geometry.cpp \
    raytracingdialog.cpp

HEADERS += \
        mainwindow.h \
    openglscene.h \
    camera.h \
    paramdialog.h \
    cameradialog.h \
    global.h \
    stb_image.h \
    mesh.h \
    aabb.h \
    raytracingscene.h \
    objmodel.h \
    model.h \
    cubemodel.h \
    scene.h \
    raytracing/basic.h \
    light.h \
    locallighting.h \
    geometry.h \
    raytracingdialog.h

FORMS += \
        mainwindow.ui \
    paramdialog.ui \
    cameradialog.ui \
    raytracingdialog.ui

INCLUDEPATH += C:\opengl\include \
            C:\assimp-3.3.1\include \

LIBS += C:\assimp-3.3.1\lib\libassimp.dll.a \
        C:\assimp-3.3.1\lib\libzlibstatic.a

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    shader.qrc \

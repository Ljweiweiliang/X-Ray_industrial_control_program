#-------------------------------------------------
#
# Project created by QtCreator 2026-05-30T08:41:56
#
#-------------------------------------------------

QT       += core gui serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = X-Ray_industrial_control_program
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# C++14 标准（VTK 8.2 要求）
msvc: QMAKE_CXXFLAGS += /std:c++14
CONFIG += c++14

# 告知 MSVC 正确报告 __cplusplus
msvc: QMAKE_CXXFLAGS += /Zc:__cplusplus

# 告诉 MSVC 源文件使用 UTF-8 编码（解决中文字符编译错误）
msvc: QMAKE_CXXFLAGS += /utf-8

# open62541 OPC UA library
INCLUDEPATH += $$PWD/open62541/include
LIBS += $$PWD/open62541/bin/open62541.lib

# VTK 8.2 三维可视化库
INCLUDEPATH += $$PWD/install_qt/include/vtk-8.2
LIBS += -L$$PWD/install_qt/lib
LIBS += -lvtkGUISupportQt-8.2
LIBS += -lvtkRenderingOpenGL2-8.2
LIBS += -lvtkRenderingVolume-8.2
LIBS += -lvtkRenderingVolumeOpenGL2-8.2
LIBS += -lvtkInteractionStyle-8.2
LIBS += -lvtkInteractionWidgets-8.2
LIBS += -lvtkRenderingFreeType-8.2
LIBS += -lvtkRenderingCore-8.2
LIBS += -lvtkCommonCore-8.2
LIBS += -lvtkCommonDataModel-8.2
LIBS += -lvtkCommonExecutionModel-8.2
LIBS += -lvtkIOImage-8.2
LIBS += -lvtkIOLegacy-8.2
LIBS += -lvtkIOCore-8.2
LIBS += -lvtkImagingCore-8.2
LIBS += -lvtkImagingGeneral-8.2
LIBS += -lvtkImagingColor-8.2
LIBS += -lvtkFiltersSources-8.2
LIBS += -lvtkFiltersCore-8.2

# 构建后将 DLL 复制到 exe 所在目录
win32 {
    QMAKE_POST_LINK += copy /y $$shell_path($$PWD/open62541/bin/open62541.dll) $$shell_path($$OUT_PWD/release) $$escape_expand(\\n)
    QMAKE_POST_LINK += copy /y $$shell_path($$PWD/install_qt/bin/*.dll) $$shell_path($$OUT_PWD/release) $$escape_expand(\\n)
}
LIBS += -lws2_32

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        mainwindow.cpp \
        edit_operations.cpp \
        adjust_operations.cpp \
        static_operations.cpp \
        windowleveldialog.cpp \
        brightnesscontrastdialog.cpp \
        acquisitionworker.cpp \
        xraycontroltab.cpp \
        visualizationtab.cpp \
        logmanager.cpp \
        opcclientmanager.cpp \
    ct_module_test2.cpp

HEADERS += \
        mainwindow.h \
        edit_operations.h \
        adjust_operations.h \
        static_operations.h \
        windowleveldialog.h \
        brightnesscontrastdialog.h \
        acquisitionworker.h \
        xraycontroltab.h \
        visualizationtab.h \
        logmanager.h \
        opcclientmanager.h \
    ct_module_test2.h

FORMS += \
        mainwindow.ui

RESOURCES += \
        resources.qrc

# 嵌入 Windows 应用程序图标（任务栏 + exe 图标）
RC_ICONS = resources/icons/app.ico

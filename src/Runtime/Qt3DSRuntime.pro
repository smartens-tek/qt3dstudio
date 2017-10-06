TEMPLATE = lib
TARGET = qt3dsruntime

DEFINES += QT3DS_RUNTIME_EXPORTS

CONFIG += installed
include(commoninclude.pri)
QT += qml

boot2qt: {
    RESOURCES += res.qrc
    DEFINES += EMBEDDED_LINUX # TODO: Is there a compile-time flag for boot2qt?
}

integrity {
RESOURCES += res.qrc
}

SOURCES += \
    Source/Viewer/UICViewerApp.cpp \
    Source/Viewer/UICAudioPlayerImpl.cpp

HEADERS += \
    Source/Viewer/qt3dsruntimeglobal.h \
    Source/Viewer/UICViewerApp.h \
    Source/Viewer/UICAudioPlayerImpl.h \
    Source/Viewer/UICViewerTimer.h

linux {
    BEGIN_ARCHIVE = -Wl,--whole-archive
    END_ARCHIVE = -Wl,--no-whole-archive
}

STATICRUNTIME = \
    $$BEGIN_ARCHIVE \
    -lEASTL$$qtPlatformTargetSuffix() \
    -lLua$$qtPlatformTargetSuffix() \
    -lqt3dsruntimestatic$$qtPlatformTargetSuffix() \
    $$END_ARCHIVE

# On non-windows systems link the whole static archives and do not put them
# in the prl file to prevent them being linked again by targets that depend
# upon this shared library
!win32:!CONFIG(static){
    QMAKE_LFLAGS += $$STATICRUNTIME
    LIBS += -lqt3dsqmlstreamer$$qtPlatformTargetSuffix()
} else {
    LIBS += \
        $$STATICRUNTIME \
        -lqt3dsqmlstreamer$$qtPlatformTargetSuffix()
}

win32 {
    LIBS += \
        -lws2_32

    RESOURCES += platformres.qrc
}

linux {
    LIBS += \
        -ldl \
        -lEGL
}

PREDEPS_LIBS = qt3dsruntimestatic

include($$PWD/../utils.pri)
PRE_TARGETDEPS += $$fixLibPredeps($$LIBDIR, PREDEPS_LIBS)

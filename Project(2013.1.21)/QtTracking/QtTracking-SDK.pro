SOURCES = main.cpp \
    TrackingWindow.cpp \
    Tracking.cpp \
    MainWindow.cpp
HEADERS = \ 
    TrackingWindow.h \
    Tracking.h \
    MainWindow.h

SDK_ROOT=../..

INCLUDEPATH += $$SDK_ROOT/include

TARGET_BUILD_PATH=$$SDK_ROOT/bin

DESTDIR=$$TARGET_BUILD_PATH

message(putting executable at $$DESTDIR)

QMAKE_LFLAGS += -Wl,-rpath,./

QMAKE_LIBDIR=-L$$TARGET_BUILD_PATH

LIBS += $$QMAKE_LIBDIR \
        -lShadow

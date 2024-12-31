# Specify output directories
DESTDIR = $$OUT_PWD/build
OBJECTS_DIR = $$OUT_PWD/build/obj
MOC_DIR = $$OUT_PWD/build/moc
RCC_DIR = $$OUT_PWD/build/rcc
UI_DIR = $$OUT_PWD/build/ui

TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG += qt core

SOURCES += \
    main.cpp

HEADERS += \
    ../qbignum.hpp

INCLUDEPATH += \
    ../

QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -O3 -funroll-loops -ftree-vectorize -finline-functions
QMAKE_LFLAGS_RELEASE += -Wl,--gc-sections

QMAKE_POST_LINK += $$OUT_PWD/build/example && echo "Build and example completed successfully."



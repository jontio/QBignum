# Check if Qt Creator has opened the project
exists(./*.pro.user) {
    DEFINES += BUILD_IN_QT_CREATOR
}

# Specify output directories
DESTDIR = $$OUT_PWD/build
OBJECTS_DIR = $$OUT_PWD/build/obj
MOC_DIR = $$OUT_PWD/build/moc
RCC_DIR = $$OUT_PWD/build/rcc
UI_DIR = $$OUT_PWD/build/ui

CONFIG += c++17

# Define the project
TEMPLATE = app
TARGET = tst_qbignum512
CONFIG += console
CONFIG += testlib

# Include the paths to your source and header files
SOURCES += tst_qbignum512.cpp

HEADERS += ../qbignum.hpp

# GMP includes and linking
LIBS += -lgmp  # Set the path to GMP library and link it

# Additional Qt libraries (if needed)
QT += core testlib

QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -O3 -funroll-loops -ftree-vectorize -finline-functions
QMAKE_LFLAGS_RELEASE += -Wl,--gc-sections

!contains(DEFINES, BUILD_IN_QT_CREATOR): {
    # Run the test program after build completion and print success message
    QMAKE_POST_LINK += $$OUT_PWD/build/tst_qbignum512 && echo "Build and test completed successfully."
}

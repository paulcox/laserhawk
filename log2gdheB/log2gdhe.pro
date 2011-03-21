CONFIG += qt \
    debug
TEMPLATE = app
TARGET = 
DEPENDPATH += .
INCLUDEPATH += .
HEADERS += imu.h \
    mainwindow.h \
    ../common/pnm_io2.h \
    ../common/chokuyoplus.h \
    ../common/localefunc.h \
    ../common/cmtilog.h \
    ../common/cinterfacewithgdhe.h
FORMS += mainwindow.ui
SOURCES += imu.cpp \
    main.cpp \
    mainwindow.cpp \
    ../common/pnm_io2.cpp \
    ../common/chokuyoplus.cpp \
    ../common/localefunc.cpp \
    ../common/cmtilog.cpp \
    ../common/cinterfacewithgdhe.cpp
LIBS += ${ROBOTPKG_BASE}/lib/libMTI.a \
    ${ROBOTPKG_BASE}/lib/libMTI.so \
    ${ROBOTPKG_BASE}/lib/libGDHE.so \
    -L${ROBOTPKG_BASE}/lib/gearbox \
    -lhokuyo_aist
INCLUDEPATH += ${ROBOTPKG_BASE}/include \
    ${ROBOTPKG_BASE}/include/MTI-clients \
    ${ROBOTPKG_BASE}/include/gearbox
INCLUDEPATH += ../common

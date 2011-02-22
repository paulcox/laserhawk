
TEMPLATE = app
TARGET = 
DEPENDPATH += .
INCLUDEPATH += . \
            ${ROBOTPKG_BASE}/sources/hardware/MTI/work.paul-Inspiron-1210/MTI-0.4/MTI/

HEADERS += imu.h \
           mainwindow.h \
           pnm_io2.h \

FORMS += mainwindow.ui \

SOURCES += imu.cpp \
           main.cpp \
           mainwindow.cpp \
           pnm_io2.cpp
		   
LIBS += ${ROBOTPKG_BASE}/lib/libMTI.a \
        ${ROBOTPKG_BASE}/lib/libMTI.so \
        ${ROBOTPKG_BASE}/lib/libGDHE.so \
        -L${ROBOTPKG_BASE}/lib/gearbox -lhokuyo_aist


INCLUDEPATH +=  ${ROBOTPKG_BASE}/include \
                ${ROBOTPKG_BASE}/include/MTI-clients \
                ${ROBOTPKG_BASE}/include/gearbox/



TEMPLATE = app
TARGET = 
DEPENDPATH += . test
INCLUDEPATH += . \
            /home/paul/openrobots/sources/hardware/MTI/work.paul-Inspiron-1210/MTI-0.4/MTI/

# Input
HEADERS += imu.h \
           mainwindow.h \
           pnm_io2.h \

FORMS += mainwindow.ui \
#         test/mainwindow.ui

SOURCES += imu.cpp \
           main.cpp \
           mainwindow.cpp \
           pnm_io2.cpp
LIBS += /home/paul/openrobots/lib/libMTI.a \
        /home/paul/openrobots/lib/libMTI.so \
        /home/paul/openrobots/lib/libGDHE.so \
        -L/home/paul/openrobots/lib/gearbox -lhokuyo_aist


INCLUDEPATH +=  /home/paul/openrobots/include \
                /home/paul/openrobots/include/MTI-clients \
                /home/paul/openrobots/include/gearbox/

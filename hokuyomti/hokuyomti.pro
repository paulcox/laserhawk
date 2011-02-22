#librairie hokuyo dans
#/home/paul/openrobots/sources/hardware/gbx-hokuyo-aist/work.paul-Inspiron-1210/gearbox-9.11/src/hokuyo_aist/libhokuyo_aist.so.1.0.0
#programme dexemple dans
#~/openrobots/sources/hardware/gbx-hokuyo-aist/work.paul-Inspiron-1210/gearbox-9.11/src/hokuyo_aist/test/hokuyo_aist_example

QT -= gui

TARGET = hokuyomti
CONFIG += console
CONFIG -= app_bundle
TEMPLATE = app
SOURCES += main.cpp \
			chokuyoplus.cpp \
			chokuyoprocess.cpp \
			pnm_io2.cpp \

INCLUDEPATH += ${ROBOTPKG_BASE}/include \
		${ROBOTPKG_BASE}/include/MTI-clients \
		${ROBOTPKG_BASE}/include/gearbox/

INCLUDEPATH += /usr/local/include/opencv/

LIBS +=	-L/usr/local/lib/ \
	-L${ROBOTPKG_BASE}/lib/gearbox/ \
	-lhokuyo_aist \
	-lopencv_core -lopencv_imgproc -lopencv_calib3d -lopencv_video -lopencv_features2d -lopencv_ml -lopencv_highgui -lopencv_objdetect -lopencv_contrib -lopencv_legacy
#-ljpeg
#LIBS += -ljpeg

HEADERS += chokuyoplus.h \
    chokuyoprocess.h \
    pnm_io2.h \
    std.h \
#OTHER_FILES += 

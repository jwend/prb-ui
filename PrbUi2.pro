#-------------------------------------------------
#
# Project created by QtCreator 2013-09-04T13:48:37
#
#-------------------------------------------------

# QMake AND Qt settings
# IMPORTANT: application will only work with Qt Kit 4.8.5 because of QGis dependencies
QT += qt3support sql network svg gui core xml
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
LANGUAGE= C++
TARGET = PrbUi2
TEMPLATE = app
QMAKE_CXXFLAGS += -std=c++0x
CONFIG += qt gui exceptions stl warn_on debug thread
#optional, comment if you don't want application to start from a console window
CONFIG += console

SOURCES += main.cpp\
        prb.cpp \
    prasterblaster-pio.cc \
    qgispreviewdialog.cpp \
    previewwindow.cpp
HEADERS  += prb.h \
    prasterblaster-pio.h \
    qgispreviewdialog.h \
    previewwindow.h
FORMS    += prb.ui \
    previewwindowbase.ui
RESOURCES += \
    resources.qrc
OTHER_FILES += \
    mActionZoomOut.png \
    mActionZoomIn.png \
    mActionPan.png \
    mActionAddLayer.png


LIBS += -L./prasterblaster/lib -lrasterblaster -lsptw
INCLUDEPATH += ./prasterblaster/include


# QGis >= 2.0 installation requried
QGISDIR=/usr
QGISLIBDIR=$${QGISDIR}/lib
QGISPLUGINDIR=$${QGISLIBDIR}/qgis/plugins
DEFINES += QGISPLUGINDIR=$${QGISPLUGINDIR}
LIBS += -L$${QGISLIBDIR} -lqgis_core -lqgis_gui
INCLUDEPATH += $${QGISDIR}/include/qgis
DEFINES += CORE_EXPORT=""
DEFINES += GUI_EXPORT=""

# gdal >= 1.9.2 requried
# Change the following variables to your GDAL and Prog installation directories
# NOTE: A script called buildgdal.sh is provdied in this directory
# which can be used to download and build these libraries
# run ./buildgdal.sh from this directory and the GDALDIR and PROJDIR variables
# do not need to be changed
GDALDIR=./gdal
PROJDIR=./gdal
LIBS += -L$${GDALDIR}/lib/ -lgdal
INCLUDEPATH += $${GDALDIR}/include

# proj >=  4.8.0 required
LIBS += -L$${PROJDIR}/lib/ -lproj
INCLUDEPATH += $${PROJDIR}/include

# open mpi 1.5 requried
LIBS += -L/usr/lib/openmpi/lib/ -lmpi_cxx -lmpi
INCLUDEPATH += /usr/lib/openmpi/include


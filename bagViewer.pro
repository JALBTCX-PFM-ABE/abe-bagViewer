contains(QT_CONFIG, opengl): QT += opengl
QT += 
RC_FILE = bagViewer.rc
RESOURCES = icons.qrc
INCLUDEPATH += /c/PFM_ABEv7.0.0_Win64/include
LIBS += -L/c/PFM_ABEv7.0.0_Win64/lib -lproj -lnvutility -lbag -lbeecrypt -lhdf5 -lgdal -lxml2 -lpoppler -lglu32 -lopengl32 -lwsock32 -liconv
DEFINES += WIN32 NVWIN3X
CONFIG += console
CONFIG += static

#
# The following line is included so that the contents of acknowledgments.hpp will be available for translation
#

HEADERS += /c/PFM_ABEv7.0.0_Win64/include/acknowledgments.hpp

QMAKE_LFLAGS += 
######################################################################
# Automatically generated by qmake (2.01a) Wed Jan 22 13:53:57 2020
######################################################################

TEMPLATE = app
TARGET = bagViewer
DEPENDPATH += .
INCLUDEPATH += .

# Input
HEADERS += bagViewer.hpp \
           bagViewerDef.hpp \
           bagViewerHelp.hpp \
           hotkeyHelp.hpp \
           prefs.hpp \
           prefsHelp.hpp \
           version.hpp
SOURCES += bagViewer.cpp \
           env_in_out.cpp \
           geotiff.cpp \
           hotkeyHelp.cpp \
           main.cpp \
           overlayFlag.cpp \
           prefs.cpp \
           set_defaults.cpp
RESOURCES += icons.qrc
TRANSLATIONS += bagViewer_xx.ts

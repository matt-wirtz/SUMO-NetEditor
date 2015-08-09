HEADERS = \
       mainwindow.h \
       networkview.h \
       item.h \
       model.h \
       pathelement.h \
       pointelement.h \
       propsview.h \
       controls.h \
       editview.h \
       attredit.h \
       jcteditor.h \
       tleditor.h
SOURCES = \
       main.cpp \
       mainwindow.cpp \
       networkview.cpp \
       item.cpp \
       model.cpp \
       pathelement.cpp \
       pointelement.cpp \
       propsview.cpp \
       controls.cpp \
       editview.cpp \
       attredit.cpp \
       jcteditor.cpp \
       tleditor.cpp
CONFIG  += qt debug
QT      += xml widgets

# install
# INSTALLS += target

RESOURCES += vres.qrc

RC_FILE = icon.rc

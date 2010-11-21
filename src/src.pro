TEMPLATE = app

CONFIG += \
  debug \
  warn_on \
  fast_printing

CONFIG -= \
  qt

SOURCES += \
  atem.cpp \
  metastock.cpp \
  ms_file.cpp \
  util.cpp

HEADERS += \
  metastock.h \
  ms_file.h \
  util.h

LIBS += \
  -lpopt

TARGET = atem

QMAKE_CXXFLAGS += -O3



fast_printing {
  DEFINES += FAST_PRINTING
}

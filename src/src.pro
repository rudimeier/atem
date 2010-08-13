TEMPLATE = app

QT -= \
  gui

CONFIG += \
  console \
  debug \
  warn_on

CONFIG -= \
  app_bundle

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
  -lpopt \

TARGET = atem

DEFINES += USE_FPRINTF

QMAKE_CXXFLAGS += -O3
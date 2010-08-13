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
  util.cpp

HEADERS += \
  metastock.h \
  util.h

LIBS += \
  -lpopt \

TARGET = atem

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
  metastock.cpp

HEADERS += \
  metastock.h

LIBS += \
  -lpopt \

TARGET = atem

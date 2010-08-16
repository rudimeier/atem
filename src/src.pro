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
  util.cpp \
  printf_fp.c

HEADERS += \
  metastock.h \
  ms_file.h \
  util.h

# these objects are for printf_fp.c only!
LIBS += \
  glibc/objects/addmul_1.o \
  glibc/objects/add_n.o \
  glibc/objects/cmp.o \
  glibc/objects/dbl2mpn.o \
  glibc/objects/divrem.o \
  glibc/objects/fpioconst.o \
  glibc/objects/lshift.o \
  glibc/objects/mul_1.o \
  glibc/objects/mul_n.o \
  glibc/objects/mul.o \
  glibc/objects/rshift.o \
  glibc/objects/submul_1.o \
  glibc/objects/sub_n.o \
  -lpopt \

TARGET = atem

# DEFINES += USE_FPRINTF

QMAKE_CXXFLAGS += -O3

# these CFLAGS are for printf_fp.c only!
QMAKE_CFLAGS += -O2 \
        -std=gnu99 -fgnu89-inline -Wall -Winline -Wwrite-strings -fmerge-all-constants -Wstrict-prototypes \
        -I./glibc/include                               \
        -I./glibc/sysdeps/generic                       \
        -I./glibc                                       \
        -MD -MP -MF printf_fp.o.dt -MT printf_fp.o
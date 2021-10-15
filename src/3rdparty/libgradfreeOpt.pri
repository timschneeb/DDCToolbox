INCLUDEPATH += $$PWD/libgradfreeOpt/GradientFree/gradfreeOpt \
               $$PWD/libgradfreeOpt/libgenmath

HEADERS += $$PWD/libgradfreeOpt/GradientFree/gradfreeOpt/gradfreeOpt.h \
    $$PWD/libgradfreeOpt/libgenmath/interpolation2.h \
    $$PWD/libgradfreeOpt/libgenmath/peakfinder.h \
    $$PWD/libgradfreeOpt/libgenmath/rand_c.h

SOURCES += $$PWD/libgradfreeOpt/GradientFree/gradfreeOpt/opt_alg.c \
    $$PWD/libgradfreeOpt/libgenmath/interpolation2.c \
    $$PWD/libgradfreeOpt/libgenmath/misc.c \
    $$PWD/libgradfreeOpt/libgenmath/naive1DDiff.c \
    $$PWD/libgradfreeOpt/libgenmath/peakfinder.c \
    $$PWD/libgradfreeOpt/libgenmath/rand_c.c \
    $$PWD/libgradfreeOpt/libgenmath/sortingIdx.c

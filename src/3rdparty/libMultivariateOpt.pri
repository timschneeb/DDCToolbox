INCLUDEPATH += $$PWD/libMultivariateOpt/GradientFree/gradfreeOpt \
               $$PWD/libMultivariateOpt/libgenmath \
               $$PWD/libMultivariateOpt/ReverseModeAD/kann

HEADERS += $$PWD/libMultivariateOpt/GradientFree/gradfreeOpt/gradfreeOpt.h \
    $$PWD/libMultivariateOpt/ReverseModeAD/kann/kann.h \
    $$PWD/libMultivariateOpt/ReverseModeAD/kann/kautodiff.h \
    $$PWD/libMultivariateOpt/libgenmath/interpolation2.h \
    $$PWD/libMultivariateOpt/libgenmath/peakfinder.h \
    $$PWD/libMultivariateOpt/libgenmath/rand_c.h

SOURCES += $$PWD/libMultivariateOpt/GradientFree/gradfreeOpt/opt_alg.c \
    $$PWD/libMultivariateOpt/ReverseModeAD/kann/kann.c \
    $$PWD/libMultivariateOpt/ReverseModeAD/kann/kautodiff.c \
    $$PWD/libMultivariateOpt/libgenmath/interpolation2.c \
    $$PWD/libMultivariateOpt/libgenmath/misc.c \
    $$PWD/libMultivariateOpt/libgenmath/naive1DDiff.c \
    $$PWD/libMultivariateOpt/libgenmath/peakfinder.c \
    $$PWD/libMultivariateOpt/libgenmath/rand_c.c \
    $$PWD/libMultivariateOpt/libgenmath/sortingIdx.c

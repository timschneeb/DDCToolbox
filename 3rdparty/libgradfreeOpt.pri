INCLUDEPATH += $$PWD/libgradfreeOpt/gradfreeOpt \
               $$PWD/libgradfreeOpt/Examples

HEADERS += $$PWD/libgradfreeOpt/gradfreeOpt/gradfreeOpt.h \
           $$PWD/libgradfreeOpt/Examples/PeakingFit/linear_interpolation.h \
           $$PWD/libgradfreeOpt/Examples/PeakingFit/peakfinder.h \
           $$PWD/libgradfreeOpt/gradfreeOpt/rand_c.h


SOURCES += $$PWD/libgradfreeOpt/gradfreeOpt/opt_alg.c \
           $$PWD/libgradfreeOpt/Examples/PeakingFit/peakfinder.c \
           $$PWD/libgradfreeOpt/gradfreeOpt/rand_c.c

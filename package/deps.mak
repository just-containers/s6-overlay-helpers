#
# This file has been generated by tools/gen-deps.sh
#

src/s6-overlay-stat/s6-overlay-stat.o src/s6-overlay-stat/s6-overlay-stat.lo: src/s6-overlay-stat/s6-overlay-stat.c
src/s6-overlay-suexec/s6-overlay-suexec.o src/s6-overlay-suexec/s6-overlay-suexec.lo: src/s6-overlay-suexec/s6-overlay-suexec.c

s6-overlay-stat: EXTRA_LIBS := ${MAYBEPTHREAD_LIB}
s6-overlay-stat: src/s6-overlay-stat/s6-overlay-stat.o ${LIBNSSS} -lskarnet
s6-overlay-suexec: EXTRA_LIBS := ${SPAWN_LIB} ${MAYBEPTHREAD_LIB}
s6-overlay-suexec: src/s6-overlay-suexec/s6-overlay-suexec.o -lexecline ${LIBNSSS} -lskarnet
INTERNAL_LIBS :=

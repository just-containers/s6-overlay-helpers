#
# This file has been generated by tools/gen-deps.sh
#

src/justc-envdir/justc-envdir.o src/justc-envdir/justc-envdir.lo: src/justc-envdir/justc-envdir.c
src/s6-overlay-suexec/s6-overlay-suexec.o src/s6-overlay-suexec/s6-overlay-suexec.lo: src/s6-overlay-suexec/s6-overlay-suexec.c

justc-envdir: EXTRA_LIBS := -lskarnet
justc-envdir: src/justc-envdir/justc-envdir.o
s6-overlay-suexec: EXTRA_LIBS := -lexecline -lskarnet ${SPAWN_LIB}
s6-overlay-suexec: src/s6-overlay-suexec/s6-overlay-suexec.o
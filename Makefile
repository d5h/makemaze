# Copyright (C) Daniel Hipschman, Aug 2004
# $Id: Makefile,v 1.5 2004/08/09 05:06:00 sirdan Exp sirdan $
PROJ = makemaze
VERS = 4.0.beta

DIST = main.c makemaze.c xmemory.c move.c play_maze.c \
	common.h move.h maze.h config.h \
	trail.data wall.data you.data finish.data \
	left_arrow.data right_arrow.data up_arrow.data down_arrow.data \
	Makefile
OBJS = main.o makemaze.o xmemory.o move.o play_maze.o
LIBS = -lSDL -lpthread

CFLAGS = -g -W -Wall -DUSE_SDL -DVERSION=\"${VERS}\"

#-----------------------------------------------------------------------------#
ZIP = gzip

DISTDIR = ${PROJ}-${VERS}

${PROJ}: ${OBJS} Makefile
	${CC} -o $@ ${CFLAGS} ${LDFLAGS} ${OBJS} ${LIBS}

clean:
	rm -f ${OBJS}

dist:
	mkdir ${DISTDIR} 
	@for i in ${DIST} ; do \
		if [ ! -e $$i ] ; then \
			${CO} $$i ; \
			chmod 644 $$i ; \
			mv $$i ${DISTDIR} ; \
		else \
			cp $$i ${DISTDIR} ; \
		fi ; \
	done 
	tar -cvf ${DISTDIR}.tar ${DISTDIR} 
	${ZIP} ${DISTDIR}.tar
	rm -fr ${DISTDIR}

${OBJS} : Makefile

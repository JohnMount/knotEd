# make file for the X11 version of knotEd

SHELL= /bin/sh
LINTSRC= knMain.c knMisc.c knXdriver.c
LINTDEP= main.h misc.h struct.h xdriver.h


#Linux flags
CC=cc
CFLAGS = -g -O -DPIC -I/usr/X11R6/include
LIBS = -lm -L/usr/X11R6/lib -lX11


knotEd: knMain.o knMisc.o knXdriver.o ktob.o 
	$(CC) $(CFLAGS) -o knotEd knMain.o knMisc.o ktob.o knXdriver.o $(LIBS)

clean:
	rm knotEd *.o

ktob.o: ktob.c 
	$(CC) -c $(CFLAGS) ktob.c

knMain.o: knMain.c $(LINTDEP)
	$(CC) -c $(CFLAGS) knMain.c

knMisc.o: knMisc.c $(LINTDEP)
	$(CC) -c $(CFLAGS) knMisc.c

knXdriver.o: knXdriver.c $(LINTDEP)
	$(CC) -c $(CFLAGS) knXdriver.c

lintout: $(LINTSRC) $(LINTDEP) Makefile
	rm -f lintout
	lint $(LFLAGS) $(LINTSRC) > lintout

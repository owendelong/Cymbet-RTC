CFLAGS = -g -I.
LDFLAGS = -g -L /usr/local/lib -lmpsse

all:	readclock setclock

readclock:	readclock.o pcf2123.o
setclock:	setclock.o pcf2123.o


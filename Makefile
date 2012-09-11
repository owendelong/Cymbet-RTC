CFLAGS = -g -I.
LDFLAGS = -g -L /usr/local/lib -lmpsse
INSTALLDIR = /usr/local/bin

all:		readclock setclock setSystemTime 65-cymbet.rules

install:	all
	cp readclock ${INSTALLDIR}
	cp setclock ${INSTALLDIR}
	cp setSystemTime ${INSTALLDIR}
	cp 65-cymbet.rules /etc/udev/rules.d
	udevadm control --reload
	chown root.root ${INSTALLDIR}/setSystemTime
	chmod 4755 ${INSTALLDIR}/setSystemTime

clean:
	rm *.o
	rm readclock
	rm setclock
	rm setSystemTime

readclock:	readclock.o pcf2123.o
setclock:	setclock.o pcf2123.o
setSystemTime:	setSystemTime.o pcf2123.o



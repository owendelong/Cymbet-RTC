Cymbet-RTC
==========

Supporting Software for Cymbet-Eval-06 USB-based Real Time Clock

All software here depends on a system with working FTDI drivers and a working installation of libmpsse
from google code: http://code.google.com/p/libmpsse/

NOTE: The software isn't ready yet, so this README is currently a place holder. Once the software is available,
a Makefile and source files will be included in the repository. I hope to have preliminary code up shortly.
If you download the preliminary code, feedback, bug reports, and most of all suggested patches are highly encouraged.

Installation
============

1. Dependencies
  A. Working FTDI drivers
    Plug in your RTC board and issue the lsusb command. If you do not see a line like:

Bus ??? Device ???: ID 0403:6010 Future Technology Devices International, Ltd FT2232C Dual USB-UART/FIFO IC

    Then you probably don't have working drivers. (At the very least, you should see some enumeration of
    a device with ID 0403:6010 or this isn't going to work.)
    
    If you have more than one such line, you may need to alter the Opening sequence in the software to
    ensure connection to the correct device.
  
  B. Google Code libmpsse
    Download from http://code.google.com/p/libmpsse/ if you don't already have it.
    After unpacking, the relevant software will be in libmpsse-1.2/src
    Change into that directory and follow the directions in ../docs/INSTALL
      (usually configure, make, make install)
  
2. Building the software

  Once the dependencies are satisfied, you should be able to just type make install and have everything
  compile and install automatically.

  If you want to make without installing, just type make. This should produce three binaries in the local
  directory -- readclock, setclock, and setSystemTime.
  
3.  Using the software

  readclock will grab all information from the clock and display it.
  setclock takes an optional command-line argument and will set the time in the RTC.
    If a command line argument is specified, then setclock will immediately set the RTC to the value specified
    if it can be parsed.
    
    If no argument is specified, setclock will do the following:
      Stop the RTC
      Set the RTC time to system time + 2 seconds.
      Wait (ungracefully pending in a tight loop) for the system clock to reach the specified time
      Start the RTC
      
  setSystemTime will set the system clock from the data read from the RTC.
  
  Note: All software assumes UTC and makes no allowances for timezones. (The system clock is UTC on
    almost all systems and local time is just what is displayed, so this should not be a problem).

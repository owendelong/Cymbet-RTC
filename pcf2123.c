/* Copyright (C) 2012 Owen DeLong

   This software is licensed under the GNU LGPL. You may
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This software is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this software; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <pcf2123.h>

#include <stdarg.h>
void debug(int lvl, char *fmt, ...);

#define DEBUG 99

/* Connect to the clock and return a valid handle. Returns NULL on failure. */
struct mpsse_context *OpenClock(int VID, int PID, uint32_t FREQ)
{
  int vid;
  int pid;
  uint32_t freq;
  struct mpsse_context *rv;

  debug(5, "OpenClock: VID=%d, PID=%d, FREQ=%d\n", VID, PID, FREQ);
  vid = (VID ? VID : 0x0403);		/* Default VID/PID */
  pid = (PID ? PID : 0x6010);
  freq = (FREQ ? FREQ : 5000);	/* Default to 100 Khz */
  debug(5, "OpenClock: vid=%d, pid=%d, freq=%d\n", vid, pid, freq);

  rv=Open(vid, pid, SPI0, freq, MSB, IFACE_A, NULL, NULL);
  debug(6, "OpenClock: Open returned %s\n", (rv ? "Success" : "Failure"));
  SetCSIdle(rv, 0);
  return(rv);
}

/* Reset the Clock */
int ResetClock(struct mpsse_context *clock_handle)
{
  char c[2];
  int rv;

  debug(5, "ResetClock\n");
  c[0] = CLOCK_RESET;
  rv=WriteClock(clock_handle, 0, c, 1);
  sleep(1);
  return(rv);
}

/* Read from Clock starting at register start for size bytes */
char *ReadClock(struct mpsse_context *clock_handle, unsigned char start, int size)
{
  char c[2];
  char *rv;

  debug(5, "ReadClock: Start=%d Size=%d\n", start, size);
  c[0] = CLOCK_READ | (start & 0x0f);
  Start(clock_handle);
  Write(clock_handle, c, 1);
  rv = Read(clock_handle, size);
  Stop(clock_handle);
  return(rv);
}

/* Write buffer to Clock starting at register start for size bytes */
int WriteClock(struct mpsse_context *clock_handle, unsigned char start, char *buffer, int size)
{
  char c[2];
  int rv;

  debug(5, "WriteClock: Start=%d Size=%d\n", start, size);
  c[0] = CLOCK_WRITE | (start & 0x0f);
  Start(clock_handle);
  Write(clock_handle, c, 1);
  rv = Write(clock_handle, buffer, size);
  Stop(clock_handle);
  return(rv);
}

/* Attempt to start the Clock and verify the Oscillator is running
 *
 * Return values: 0 -- Oscillator successfully started or already running
 *                -1 -- Failed to start oscillator
 */
int StartOscillator(struct mpsse_context *clock_handle)
{
  char *buf;
  char c[2];
  int rv;

  debug(5, "StartOscillator\n");
  c[0]=CLOCK_READ;		/* Read current Control1 values */
  Start(clock_handle);
  Write(clock_handle, c, 1);
  buf = Read(clock_handle, 1);
  Stop(clock_handle);
  debug(9, "\tRetrieved: %02x from Control1\n", buf[0]);
  c[0]=CLOCK_WRITE | 0x00;	/* Clear STOP flag in Control1 */
  buf[0] &= 0xdf;
  debug(9, "\tSetting: %02x to Control1\n", buf[0]);
  Start(clock_handle);
  Write(clock_handle, c, 1);
  Write(clock_handle, buf, 1);
  Stop(clock_handle);
  free(buf);			/* free buffer from previous operations */
  c[0]=CLOCK_READ | 0x02;	/* Read current Seconds register */
  Start(clock_handle);
  Write(clock_handle, c, 1);
  buf = Read(clock_handle, 1);
  Stop(clock_handle);
  debug(9, "\tRetrieved: %02x from OS/Seconds\n", buf[0]);
  c[0]=CLOCK_WRITE | 0x02;	/* Attempt to clear OS flag in Seconds register */
  buf[0] &= 0x7f;
  debug(9, "\tSetting: %02x to OS/Seconds\n", buf[0]);
  Start(clock_handle);
  Write(clock_handle, c, 1);
  Write(clock_handle, buf, 1);
  Stop(clock_handle);
  free(buf);			/* free buffer from previous operations */
  c[0]=CLOCK_READ | 0x02;	/* Read Seconds register to verify OS Cleared */
  Start(clock_handle);
  Write(clock_handle, c, 1);
  buf = Read(clock_handle, 1);
  debug(7, "\tResulting OS/Seconds: %d\n", buf[0]);
  rv=0;				/* Prepare return value */
  if (buf[0] & 0x80) rv=-1;
  free(buf);			/* Free buffer from previous operations */
  debug(5, "StartOscillator: Return %d\n", rv);
  return(rv);
}
  
/* Stop the clock and verify it stopped
 *
 * Return values: 0 -- Oscillator successfully stopped or not running
 *                -1 -- Failed to stop oscillator
 */
int StopOscillator(struct mpsse_context *clock_handle)
{
  char *buf;
  char c[2];
  int rv;

  debug(5, "StopOscillator:\n");
  c[0]=CLOCK_READ;		/* Read in the Control1 Register */
  Start(clock_handle);
  Write(clock_handle, c, 1);
  buf = Read(clock_handle, 1);
  debug(9, "\tRetrieved: %02x from Control1\n", buf[0]);
  Stop(clock_handle);
  c[0]=CLOCK_WRITE;		/* Prepare to Write new value to Control1 Register */
  buf[0] |= 0x20;		/* Bring STOP flag high in Control1 Register */
  debug(9, "\tSetting: %02x to Control1\n", buf[0]);
  Start(clock_handle);
  Write(clock_handle, c, 1);
  Write(clock_handle, buf, 1);	/* Write out new value */
  Stop(clock_handle);
  free(buf);
  c[0]=CLOCK_READ | 0x00;	/* Read in the Control1 Register */
  Start(clock_handle);
  Write(clock_handle, c, 1);
  buf = Read(clock_handle, 1);
  debug(9, "\tResulting: %02x in Control1\n", buf[0]);
  rv=0;				/* Prepare return value */
  if (buf[0] & 0x20) rv=-1;
  free(buf);			/* Free buffer from previous operations */
  debug(5, "StopOscillator: Return %d\n", rv);
  return(rv);
}

void CloseClock(struct mpsse_context *clock_handle)
{
  debug(5, "CloseClock\n");
  return(Close(clock_handle));
}

void debug(int lvl, char *fmt, ...)
{
  va_list ap;
  if (DEBUG > lvl)
  {
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
  }
}

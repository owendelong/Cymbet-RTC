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

#ifndef	_PCF2123_H
#include <mpsse.h>
#include <sys/types.h>

#define MAX_CLOCK_SPEED		1
#define CLOCK_RESET		0x54
#define CLOCK_READ		0x90
#define CLOCK_WRITE		0x10
#define READ_SIZE		16

struct mpsse_context *OpenClock(int VID, int PID, uint32_t freq);
int ResetClock(struct mpsse_context *clock_handle);
char *ReadClock(struct mpsse_context *clock_handle, unsigned char start, int size);
int WriteClock(struct mpsse_context *clock_handle, unsigned char start, char *buffer, int size);

int StartOscillator(struct mpsse_context *clock_handle);
int StopOscillator(struct mpsse_context *clock_handle);

void CloseClock(struct mpsse_context *clock_handle);
#endif

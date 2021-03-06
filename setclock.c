#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <pcf2123.h>


#define READ_SIZE	16


/* Clock Polarity - Idle = Low			0
   Clock Phase - Rising Edge			0
   Chip Select -- Active = High
*/

unsigned char tobcd(int value);

int main(int ARGC, char **ARGV, char **ENVP)
{
	char *data;
        /*                       1          2   */
	/*             01 34 67 90 23 56 78 012 */
        /*            "00/00/00 00:00:00 AM DOW"*/
        char asc_time[] = "00/00/00 00:00:00 AM DOW\n";
	char *wday = "SUNMONTUEWEDTHUFRISAT";
	int retval = EXIT_FAILURE;
	struct mpsse_context *clock = NULL;
	struct tm *gt;
	time_t t;
	struct timespec tv;

	if(clock = OpenClock(0,0,0))
	{
		printf("%s initialized at %dHz (SPI mode 0)\n", GetDescription(clock), GetClock(clock));
		ResetClock(clock);

		printf("Check Oscillator Running: ");
		if(StartOscillator(clock))
		{
			printf("NO!\n");
			fprintf(stderr, "Warning: Oscillator not running.\n");
		}
		else
		{
			printf("Yes.\n");
		}

		/* Set Clock */
		if (ARGC != 1)
		{
			/* Parse time from args */
			printf("Incorrect %d Arguments.\n", ARGC);
			printf("Usage: %s [time]\n\n\tTime argument not yet supported.\n", ARGV[0]);
			exit(0);
		}
		else
		{
			/* Get UTC from system clock */
			t=time(NULL)+2;
			gt=gmtime(&t);
			printf("Obtained Time from System: %02d/%02d/%04d %02d:%02d:%02d UTC\n",
					gt->tm_mon + 1, gt->tm_mday, gt->tm_year+1900, gt->tm_hour, gt->tm_min, gt->tm_sec);
			
		}
		data = ReadClock(clock, 0, READ_SIZE);
		if(data)
		{
			printf("Raw data read out from clock: C1=%02x C2=%02x SEC=%02x MIN=%02x HR=%02x DAY=%02x DOW=%02x MON=%02x YR=%02x AMIN=%02x AHR=%02x ADY=%02x ADOW=%02x OFFSET=%02x TCO=%02x CDN=%02x\n",
				 data[0],  data[1],  data[2],  data[3],
				 data[4],  data[5],  data[6],  data[7],
				 data[8],  data[9], data[10], data[11],
				data[12], data[13], data[14], data[15]
			);
		}
		free(data);
		/* Update clock registers */
		data = malloc(32);
		/*	Control 1 */
		data[0] = 0x24;		/* STOP | 24HR */
		/*	Control 2 */
		data[1] = 0x00;
		/*	Seconds */
		data[2] = tobcd(gt->tm_sec);
		/*	Minutes */
		data[3] = tobcd(gt->tm_min);
		/*	Hours */
		data[4] = tobcd(gt->tm_hour);
		/*	Days */
		data[5] = tobcd(gt->tm_mday);
		/*	Weekday */
		data[6] = tobcd(gt->tm_wday);
		/*	Month */
		data[7] = tobcd(gt->tm_mon + 1);
		/*	Year */
		data[8] = tobcd(gt->tm_year % 100);
		/*	Alarm Settings */
		data[9] = 0;		/* Minutes */
		data[10] = 0;		/* Hour */
		data[11] = 0;		/* Day */
		data[12] = 0;		/* Weekday */
		/*	Offset Register */
		data[13] = 0;
		/*	Timer */
		data[14] = 0x60;	/* 1 Hz when ~STOP */
		data[15] = 0;
		printf("Raw data prepared for clock: C1=%02x C2=%02x SEC=%02x MIN=%02x HR=%02x DAY=%02x DOW=%02x MON=%02x YR=%02x AMIN=%02x AHR=%02x ADY=%02x ADOW=%02x OFFSET=%02x TCO=%02x CDN=%02x\n",
			 data[0],  data[1],  data[2],  data[3],
			 data[4],  data[5],  data[6],  data[7],
			 data[8],  data[9], data[10], data[11],
			data[12], data[13], data[14], data[15]
		);
		printf("Data loaded into buffer, writing to clock...\n");
		StopOscillator(clock);
		WriteClock(clock, 0, data, READ_SIZE);
		free(data);
		data = ReadClock(clock, 0, READ_SIZE);
		if(data)
		{
			printf("Raw data read back from clock: C1=%02x C2=%02x SEC=%02x MIN=%02x HR=%02x DAY=%02x DOW=%02x MON=%02x YR=%02x AMIN=%02x AHR=%02x ADY=%02x ADOW=%02x OFFSET=%02x TCO=%02x CDN=%02x\n",
				 data[0],  data[1],  data[2],  data[3],
				 data[4],  data[5],  data[6],  data[7],
				 data[8],  data[9], data[10], data[11],
				data[12], data[13], data[14], data[15]
			);
		};
		printf("Done!\n");
		tv.tv_sec = 0;
		tv.tv_nsec = 490 * 1000 * 1000; /* 490 milliseconds expressed as nanoseconds */
		if(ARGC == 1)
		{
			/* Wait for time to strt the clock */
			while (time(NULL) < t);
			/* Must wait an additional 1/2 second before starting clock */
			nanosleep(&tv, NULL);
		}
		StartOscillator(clock);
		data = ReadClock(clock, 0, READ_SIZE);
		if(data)
		{
			printf("Raw data returned from clock: C1=%02x C2=%02x SEC=%02x MIN=%02x HR=%02x DAY=%02x DOW=%02x MON=%02x YR=%02x AMIN=%02x AHR=%02x ADY=%02x ADOW=%02x OFFSET=%02x TCO=%02x CDN=%02x\n",
				 data[0],  data[1],  data[2],  data[3],
				 data[4],  data[5],  data[6],  data[7],
				 data[8],  data[9], data[10], data[11],
				data[12], data[13], data[14], data[15]
			);
			printf("Clock values returned:\n");
			/* Control 1 */
			printf("\tControl_1: %s - %s %s - %s %s -\n",
				(data[0] & 0x80 ? "EXT_TEST" : "ext_test"),
				(data[0] & 0x20 ? "STOP" : "stop"),
				(data[0] & 0x10 ? "SR" : "sr"),
				(data[0] & 0x04 ? "24-Hr" : "AM/PM"),
				(data[0] & 0x02 ? "CIE" : "cie")
			);
			/* Control 2 */
			printf("\tControl_2: %s %s %s %s %s %s %s %s\n",
				(data[1] & 0x80 ? "MI" : "mi"),
				(data[1] & 0x40 ? "SI" : "si"),
				(data[1] & 0x20 ? "MSF" : "msf"),
				(data[1] & 0x10 ? "TI_TP" : "ti_tp"),
				(data[1] & 0x08 ? "AF" : "af"),
				(data[1] & 0x04 ? "TF" : "tf"),
				(data[1] & 0x02 ? "AIE" : "aie"),
				(data[1] & 0x01 ? "TIE" : "tie")
			);
			/* Seconds & Oscillator Integrity */
			if (data[2] & 0x80)
			{
				printf("Oscillator (was) Stopped\n");
			}
			else
			{
				printf("Oscellator Valid\n");
			}
        /*                       1          2   */
	/*             01 34 67 90 23 56 89 123 */
        /*            "00/00/00 00:00:00 AM DOW"*/
			asc_time[15] = ((data[2] & 0x70) >> 4) + 0x30;
			asc_time[16] = (data[2] & 0x0f) + 0x30;
			/* Minutes */
			asc_time[12] = ((data[3] & 0x70) >> 4) + 0x30;
			asc_time[13] = (data[3] & 0x0f) + 0x30;
			/* Hours & AM/PM */
			if(data[0] & 0x04)
			{
				asc_time[18]='2';
				asc_time[19]='4';
				asc_time[9]  = ((data[4] & 0x30) >> 4) + 0x30;
			}
			else
			{
				asc_time[18]= (data[4] & 0x20) ? 'P' : 'A';
				asc_time[19]= 'M';
				asc_time[9] = ((data[4] & 0x10) >> 4) + 0x30;
			}
			asc_time[10] = (data[4] & 0x0f) + 0x30;
			/* Days */
			asc_time[3]  = ((data[5] & 0x30) >> 4) + 0x30;
			asc_time[4]  = (data[5] & 0x0f) + 0x30;
			/* DOW */
			asc_time[21] = wday[data[6]*3];
			asc_time[22] = wday[data[6]*3+1];
			asc_time[23] = wday[data[6]*3+2];
			/* Months */
			asc_time[0]  = ((data[7] & 0x10) >> 4) + 0x30;
			asc_time[1]  = (data[7] & 0x0f) + 0x30;
			/* Years */
			asc_time[6]  = ((data[8] & 0xf0) >> 4) + 0x30;
			asc_time[7]  = (data[8] & 0x0f) + 0x30;
			/* Print it all out */
			printf("\tTime: %s", asc_time);
		}
		else
		{
			printf("Failed to obtain data from MPESSE SPI interface: %s\n", ErrorString(clock));
		}
	}
	else
	{
		printf("Failed to initialize MPSSE SPI to Clock: %s\n", ErrorString(clock));
	}

	Close(clock);

	return retval;
}

unsigned char tobcd(int value)
{
	if (value < 0 || value > 99)
	{
		fprintf(stderr, "Error: tobcd called with invalid parameter: %d substituting 0.\n", value);
		return(0);
	}

	return((value/10) * 16 + (value % 10));
}


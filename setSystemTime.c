#include <stdio.h>
#include <stdlib.h>
#include <mpsse.h>
#include <string.h>
#include <pcf2123.h>
#include <time.h>

#define READ_SIZE	16


/* Clock Polarity - Idle = Low			0
   Clock Phase - Rising Edge			0
   Chip Select -- Active = High
*/

int bcdtodec(unsigned long i);

int main(int ARGC, char **ARGV, char **ENVP)
{
	FILE *fp = NULL;
	char *data;
        /*                       1          2   */
	/*             01 34 67 90 23 56 78 012 */
        /*            "00/00/00 00:00:00 AM DOW"*/
        char time[] = "00/00/00 00:00:00 AM DOW\n";
	char *wday = "SUNMONTUEWEDTHUFRISAT";
	int retval = EXIT_FAILURE;
	struct mpsse_context *clock = NULL;
	time_t t;
	struct tm tstruct;
	struct timeval tv;
        char *tz;

	if(clock = OpenClock(0, 0, 0))
	{
		data=ReadClock(clock, 0, 16);
		if (data)
		{
			if (data[2] & 0x80)
			{
				fprintf(stderr, "%s: Assertion Failed: Oscillator has been stopped. Clock not reliable.\n", ARGV[0]);
				exit(3);
			}
			tstruct.tm_sec = bcdtodec(data[2] & 0x7f);
			tstruct.tm_min = bcdtodec(data[3] & 0x7f);
			tstruct.tm_hour = (data[0] & 0x04) ? (bcdtodec(data[4] & 0x3f)) : (12*(data[4] & 0x20) >> 5) + bcdtodec(data[4] & 0x1f);
			tstruct.tm_mday = bcdtodec(data[5] & 0x3f);
			tstruct.tm_wday = bcdtodec(data[6] & 0x07);
			tstruct.tm_mon = bcdtodec(data[7] & 0x1f) - 1;	/* Clock counts 1-12, tstruct expects 0-11 */
			tstruct.tm_year = bcdtodec(data[8]) + (data[8] < 0x70 ? 100 : 0);
			tstruct.tm_isdst=0;
			tz = getenv("TZ");
			setenv("TZ", "", 1);
			tzset();
			t = mktime(&tstruct);
			if(tz) setenv("TZ", tz, 1);
			else unsetenv("TZ");
			tzset();
			if(gettimeofday(&tv, NULL))
			{
				fprintf(stderr, "%s: Error: Cound not get current system time: ", ARGV[0]);
				perror(NULL);
				exit(4);
			}
			else
			{
				tv.tv_sec = t;
				tv.tv_usec = 0;
				if(settimeofday(&tv, NULL))
				{
					fprintf(stderr, "%s: Error: Failed to set system time: ", ARGV[0]);
					perror(NULL);
					printf("Attempted to set: tv.tv_sec = %d tv.tv_usec = %d\n");
					exit(5);
				}
				else
				{
					printf("%s: Successfully set system time to %s", ARGV[0], asctime(gmtime(&t)));
				}
			}
		}
		else
		{
			fprintf(stderr, "%s: Error: Could not read USB Clock: ", ARGV[0]);
			perror(NULL);
			exit(2);
		}
	}
	else
	{
		fprintf(stderr, "%s: Error: Could not open USB Clock: ", ARGV[0]);
		perror(NULL);
		exit(1);
	}
}

int bcdtodec(unsigned long i)
{
	int j=1;
	int k=0;
	while(i)
	{
		k+=(i & 0xf) * j;
		i >>= 4;
		j *= 10;
	}
	return(k);
}

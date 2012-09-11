#include <stdio.h>
#include <stdlib.h>
#include <mpsse.h>
#include <string.h>
#include <pcf2123.h>

#define READ_SIZE	16


/* Clock Polarity - Idle = Low			0
   Clock Phase - Rising Edge			0
   Chip Select -- Active = High
*/

int main(void)
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

	if(clock = OpenClock(0, 0, 0))
	{
		printf("%s initialized at %dHz (SPI mode 0)\n", GetDescription(clock), GetClock(clock));
		ResetClock(clock);
		printf("Reset clock\n");
		while(1)
		{
		    data=ReadClock(clock, 0, 16);
		
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
			printf("\tControl_2: %s %s %s %s %s %s %s %sn",
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
			if (data[2] & 0x80) printf("Oscillator (was) Stopped\n");
        /*                       1          2   */
	/*             01 34 67 90 23 56 89 123 */
        /*            "00/00/00 00:00:00 AM DOW"*/
			time[15] = ((data[2] & 0x70) >> 4) + 0x30;
			time[16] = (data[2] & 0x0f) + 0x30;
			/* Minutes */
			time[12] = ((data[3] & 0x70) >> 4) + 0x30;
			time[13] = (data[3] & 0x0f) + 0x30;
			/* Hours & AM/PM */
			if(data[0] & 0x04)
			{
				time[18]='2';
				time[19]='4';
				time[9]  = ((data[4] & 0x30) >> 4) + 0x30;
			}
			else
			{
				time[18]= (data[4] & 0x20) ? 'P' : 'A';
				time[19]= 'M';
				time[9] = ((data[4] & 0x10) >> 4) + 0x30;
			}
			time[10] = (data[4] & 0x0f) + 0x30;
			/* Days */
			time[3]  = ((data[5] & 0x30) >> 4) + 0x30;
			time[4]  = (data[5] & 0x0f) + 0x30;
			/* DOW */
			time[21] = wday[data[6]*3];
			time[22] = wday[data[6]*3+1];
			time[23] = wday[data[6]*3+2];
			/* Months */
			time[0]  = ((data[7] & 0x10) >> 4) + 0x30;
			time[1]  = (data[7] & 0x0f) + 0x30;
			/* Years */
			time[6]  = ((data[8] & 0xf0) >> 4) + 0x30;
			time[7]  = (data[8] & 0x0f) + 0x30;
			/* Print it all out */
			printf("\tTime: %s", time);
			free(data);
		    }
		        else
		    {
			printf("Failed to obtain data from MPESSE SPI interface: %s\n", ErrorString(clock));
		    }
		    sleep(1);
		}

		Close(clock);
    }

    return(retval);
}

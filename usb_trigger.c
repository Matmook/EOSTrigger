/**
 * usb_trigger.c
 * @author	Matthieu Barreteau
 * @email   matthieu@barreteau.org
 * @date	09/07/2018
 */	
 
/* build with cc usb_trigger.c -o usb_trigger */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include "stdio.h"
#include "termios.h"
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>

#define PRODUCT_NAME "Canon Bulb driver by Matmook"
#define PRODUCT_VERSION "August 2018"
#define DEFAULT_DELAY 500

/**
 *  vars
 **/

char bulb_port[254];
int bulb_delay;
int lock_mirror = 0;

/**
 * Catch system signals
 * @param		SIGx (terminal code)
 */	
 void sigHandler(int sig) {
    switch (sig) {
    case SIGSEGV:
        printf("I think you're trying to write on the moon ...\n");
        break;
    case SIGTERM:
        printf("Closing %s by system request ...\n",PRODUCT_NAME);
        break;
	default:
        printf("wasn't expecting that!\n");
    }
    exit(0);
}


/**
 * Print_usage
 */
static void print_usage(const char *prog)
{
    printf("Usage: %s [--version][--help][-d delay (seconds)][-p usb port][-l]\n", prog);
  
	puts(
		 "\t--version\n"
			"\t\tdisplay version number\n"
		 "\t-d delay \n"
			"\t\tbulb action timeout (default 350ms)\n"
		 "\t-l use mirror lock\n"
		 "\t-p port\n"
			"\t\tusb port (default /dev/ttyUSB0)\n"
		 "\n"
	     );
	exit(1);
}


/**
 *Tweakkkkk
 */
static void parse_opts(int argc, char *argv[])
{
	int c;
	int option_index = 0;

	struct option long_options[] = {
		{ "port",  required_argument, 0, 'p' },
		{ "delay",  required_argument, 0, 'd' },
		{ "lock",  required_argument, 0, 'l' },
		{ "version",  no_argument, 0, 'v' },
		{ "help",  no_argument, 0, 'h' },
		{ NULL, 0, 0, 0 },
	};

	while (1) {
		c = getopt_long(argc, argv, "p:d:v:h:l", long_options, &option_index);

		if (c == -1)
			break;

		switch (c) {
		case 'v':
			printf("Version : %s\n",PRODUCT_VERSION);
			exit(1);
			break;

		case 'p':
			memset(bulb_port, '\0', sizeof(bulb_port));
			strcpy(bulb_port, optarg);
			break;

		case 'd':
			bulb_delay = atoi(optarg) + 1;
			break;
			
		case 'l':
			lock_mirror = 1;
			break;
			
		case 'h':
		default:
			print_usage(argv[0]);
			exit(0);
			break;
		}
	}
}


/**
 * Wait for some number of milliseconds
 */
void delay (unsigned int howLong)
{
  struct timespec sleeper, dummy ;

  sleeper.tv_sec  = (time_t)(howLong / 1000) ;
  sleeper.tv_nsec = (long)(howLong % 1000) * 1000000 ;

  nanosleep (&sleeper, &dummy) ;
}


/**
 * main		You should always start somewhere
 * @author		Matthieu Barreteau
 * @date		04/26/2014
 */
int main(int argc, char **argv) {
	int cpt = 0;
  	int fd;
	int DTR_flag = TIOCM_DTR;

	strcpy(bulb_port,"/dev/ttyUSB0");

	bulb_delay = 0 ;  
  
	/* get options */
	parse_opts(argc, argv);

	/* listen to system signals */
	signal(SIGSEGV, sigHandler);
	signal(SIGTERM, sigHandler);

	/* open adapter */
	fd = open( bulb_port, O_RDWR | O_NOCTTY );

	if (lock_mirror)
	{
		ioctl( fd, TIOCMBIS, &DTR_flag );
		sleep(3);
		ioctl( fd, TIOCMBIC, &DTR_flag );
		delay(500);
	}
	
	ioctl( fd, TIOCMBIS, &DTR_flag );

	if ( fd != -1 )
	{
		if ( bulb_delay > 0 ) 
		{
			sleep( bulb_delay );
		} else {
			delay( DEFAULT_DELAY );
		}
		ioctl( fd, TIOCMBIC, &DTR_flag );
	} else {
	  printf("Unable to open %s\n",bulb_port);
	  return 1;
	}
  
	close( fd );
	return 0;
}
/* this line is empty.... but must exist ! */

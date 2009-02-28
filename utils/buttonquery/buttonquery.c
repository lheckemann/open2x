#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <unistd.h>
#include <stropts.h>
#include <errno.h>

extern int errno;
unsigned long *memregs32;
unsigned short *memregs16;

int memfd;

#define GPIOAPINLVL 0x1180
#define GPIOBPINLVL 0x1182
#define GPIOCPINLVL 0x1184
#define GPIODPINLVL 0x1186
#define GPIOEPINLVL 0x1188
#define GPIOFPINLVL 0x118A
#define GPIOGPINLVL 0x118C
#define GPIOHPINLVL 0x118E
#define GPIOIPINLVL 0x1190
#define GPIOJPINLVL 0x1192
#define GPIOKPINLVL 0x1194
#define GPIOLPINLVL 0x1196
#define GPIOMPINLVL 0x1198
#define GPIONPINLVL 0x119A
#define GPIOOPINLVL 0x119C

#define GPIOAOUT 0x1060
#define GPIOBOUT 0x1062
#define GPIOCOUT 0x1064
#define GPIODOUT 0x1066
#define GPIOEOUT 0x1068
#define GPIOFOUT 0x106A
#define GPIOGOUT 0x106C
#define GPIOHOUT 0x106E
#define GPIOIOUT 0x1070
#define GPIOJOUT 0x1072
#define GPIOKOUT 0x1074
#define GPIOLOUT 0x1076
#define GPIOMOUT 0x1078
#define GPIONOUT 0x107A
#define GPIOOOUT 0x107C

#define GP2X_BUTTON_UP              (0)
#define GP2X_BUTTON_DOWN            (4)
#define GP2X_BUTTON_LEFT            (2)
#define GP2X_BUTTON_RIGHT           (6)
#define GP2X_BUTTON_UPLEFT          (1)
#define GP2X_BUTTON_UPRIGHT         (7)
#define GP2X_BUTTON_DOWNLEFT        (3)
#define GP2X_BUTTON_DOWNRIGHT       (5)
#define GP2X_BUTTON_CLICK           (18)
#define GP2X_BUTTON_A               (12)
#define GP2X_BUTTON_B               (13)
#define GP2X_BUTTON_X               (15)
#define GP2X_BUTTON_Y               (14)
#define GP2X_BUTTON_L               (11)
#define GP2X_BUTTON_R               (10)
#define GP2X_BUTTON_START           (8)
#define GP2X_BUTTON_SELECT          (9)
#define GP2X_BUTTON_VOLUP           (16)
#define GP2X_BUTTON_VOLDOWN         (17)


void *trymmap (void *start, size_t length, int prot, int flags, int fd, off_t offset)
{
	char *p;
	int aa;

	//printf ("mmap(%X, %X, %X, %X, %X, %X) ... ", (unsigned int)start, length, prot, flags, fd, (unsigned int)offset);
	p = mmap (start, length, prot, flags, fd, offset);
	if (p == (char *)0xFFFFFFFF)
	{
		aa = errno;
		printf ("failed. errno = %d\n", aa);
	}
	else
	{
		//printf ("OK! (%X)\n", (unsigned int)p);
	}

	return p;
}

unsigned char initphys (void)
{
	memfd = open("/dev/mem", O_RDWR);
	if (memfd == -1)
	{
		printf ("Open failed\n");
		return 0;
	}

	//printf ("/dev/mem opened successfully - fd = %d\n", memfd);

	memregs32 = trymmap(0, 0x10000, PROT_READ|PROT_WRITE, MAP_SHARED, memfd, 0xc0000000);
	if (memregs32 == (unsigned long *)0xFFFFFFFF) return 0;

	memregs16 = (unsigned short *)memregs32;

	return 1;
}

void closephys (void)
{
	close (memfd);
}

// return status of a specific GP2X button, 1 if pressed, 0 if not
int getstatus (int number)
{
	unsigned long mask = 0;
	switch (number) {
		case GP2X_BUTTON_UP:
			mask = 1;
			if (~(memregs16[GPIOMPINLVL >> 1]) & mask) 
				return 1;
			break;
		case GP2X_BUTTON_DOWN:
			mask = 1 << 4;
			if (~(memregs16[GPIOMPINLVL >> 1]) & mask)
				return 1;
			break;
		case GP2X_BUTTON_LEFT:
			mask = 1 << 2;
			if (~(memregs16[GPIOMPINLVL >> 1]) & mask)
				return 1;
			break;
		case GP2X_BUTTON_RIGHT:
			mask = 1 << 6;
			if (~(memregs16[GPIOMPINLVL >> 1]) & mask) 
				return 1;
			break;
		case GP2X_BUTTON_UPLEFT:
			mask = 1 << 1;
			if (~(memregs16[GPIOMPINLVL >> 1]) & mask)
				return 1;
			break;
		case GP2X_BUTTON_UPRIGHT:
			mask = 1 << 7;
			if (~(memregs16[GPIOMPINLVL >> 1]) & mask)
				return 1;
			break;
		case GP2X_BUTTON_DOWNLEFT:
			mask = 1 << 3;
			if (~(memregs16[GPIOMPINLVL >> 1]) & mask)
				return 1;
			break;
		case GP2X_BUTTON_DOWNRIGHT:
			mask = 1 << 5;
			if (~(memregs16[GPIOMPINLVL >> 1]) & mask)
				return 1;
			break;
		case GP2X_BUTTON_VOLDOWN:
			mask = 1 << 6;
			if (~(memregs16[GPIODPINLVL >> 1]) & mask)
				return 1;
			break;
		case GP2X_BUTTON_VOLUP:
			mask = 1 << 7;
			if (~(memregs16[GPIODPINLVL >> 1]) & mask)
				return 1;
			break;
		case GP2X_BUTTON_CLICK:
			mask = 1 << 11;
			if (~(memregs16[GPIODPINLVL >> 1]) & mask)
				return 1;
			break;
		case GP2X_BUTTON_L:
			mask = 1 << 10;
			if (~(memregs16[GPIOCPINLVL >> 1]) & mask)
				return 1;
			break;
		case GP2X_BUTTON_R:
			mask = 1 << 11;
			if (~(memregs16[GPIOCPINLVL >> 1]) & mask)
				return 1;
			break;
		case GP2X_BUTTON_A:
			mask = 1 << 12;
			if (~(memregs16[GPIOCPINLVL >> 1]) & mask)
				return 1;
			break;
		case GP2X_BUTTON_B:
			mask = 1 << 13;
			if (~(memregs16[GPIOCPINLVL >> 1]) & mask)
				return 1;
			break;
		case GP2X_BUTTON_X:
			mask = 1 << 14;
			if (~(memregs16[GPIOCPINLVL >> 1]) & mask)
				return 1;
			break;
		case GP2X_BUTTON_Y:
			mask = 1 << 15;
			if (~(memregs16[GPIOCPINLVL >> 1]) & mask)
				return 1;
			break;
		case GP2X_BUTTON_SELECT:
			mask = 1 << 9;
			if (~(memregs16[GPIOCPINLVL >> 1]) & mask)
				return 1;
			break;
		case GP2X_BUTTON_START:
			mask = 1 << 8;
			if (~(memregs16[GPIOCPINLVL >> 1]) & mask)
				return 1;
			break;
		default:
			// shouldn't happen
			printf("error in getstatus()\n");
			return 0;
			break;
	}

	return 0;
}


int main(int argc, char *argv[])
{
	int i;
	
	
	if (!initphys()) {
		printf("Error mapping GPIO registers!\n");
		printf("Exiting..\n");
		return 100;
	}

	if (argc > 1) {
		// We have been passed command-line parameters
		switch (argv[1][1]) {
			case 'h':
				// display help:
				printf("GP2X button input utility written by Senor Quack v1.0\n\n");
				printf("OPTIONS:\n");
				printf("-h\t\t\t:\tDisplay help and version info\n\n");
				printf("-sBUTTON_NUMBER\t\t:\tGet current status of a specific button.\n");
				printf("\t\t\t\t(Returns immediately, return status is 0 or 1, 1 if button is pressed.)\n");
				printf("\t\t\t\tBUTTON_NUMBER is one of the following:\n\n");
				printf("\tGP2X_BUTTON_UP\t\t\t0\t\tGP2X_BUTTON_A\t\t12\n");
				printf("\tGP2X_BUTTON_DOWN\t\t4\t\tGP2X_BUTTON_B\t\t13\n");
				printf("\tGP2X_BUTTON_LEFT\t\t2\t\tGP2X_BUTTON_X\t\t15\n");
				printf("\tGP2X_BUTTON_RIGHT\t\t6\t\tGP2X_BUTTON_Y\t\t14\n");
				printf("\tGP2X_BUTTON_UPLEFT\t\t1\t\tGP2X_BUTTON_L\t\t11\n");
				printf("\tGP2X_BUTTON_UPRIGHT\t\t7\t\tGP2X_BUTTON_R\t\t10\n");
				printf("\tGP2X_BUTTON_DOWNLEFT\t\t3\t\tGP2X_BUTTON_START\t8\n");
				printf("\tGP2X_BUTTON_DOWNRIGHT\t\t5\t\tGP2X_BUTTON_SELECT\t9\n");
				printf("\tGP2X_BUTTON_CLICK\t\t18\t\tGP2X_BUTTON_VOLUP\t16\n");
				printf("\t\t\t\t\t\t\tGP2X_BUTTON_VOLDOWN\t17\n\n");
				printf("* If no option is passed, this program will wait until a button is pressed.\n\n");
				printf("* The return value upon exiting is one of the values listed above, corresponding\n");
				printf("\tto the button pressed before exiting.\n\n");
				printf("* Utility returns 100 on error.\n");
				break;
			case 's':
				// caller wants status of specific button, return immediately regardless of whether any buttons are pressed.
				if (argv[1][2] == '\0') {
					//caller didn't supply required argument to -s
					printf("Error:  -s requires a parameter, see help by passing -h\n");
				} else {
					int button_wanted = atoi(&argv[1][2]);
					if ((button_wanted < 0) || (button_wanted > 18)) {
						//caller supplied button number that is out of range
						printf("Error: button value out of range, see help by passing -h\n");
					} else {
						//OK, get down to returning status of specified button
						
						if (getstatus(button_wanted)) {
							closephys();
							return 1;
						} else {
							closephys();
							return 0;
						}
					}
				}
				break;
			default:
				// invalid parameter
				printf("Error: invalid parameter passed, see help by passing -h\n");
				closephys();
				return 100;
		} // switch
	} // if (argc > 1) 
	else {
		// we weren't passed a command line parameter, so we should block until a button is pressed
		int can_exit = 0;
		int button_pressed;
		while (!can_exit) {
			for (i = 0; i <= 18; i++) {
				if (getstatus(i)) {
					button_pressed = i;
					can_exit = 1;
					break;
				}
			}
			sleep(.005); 	//sleep 5ms
		}
		
		closephys();
		return button_pressed;
	}		
			
	closephys();

	return 0;
}

